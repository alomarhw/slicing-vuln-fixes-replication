"""
rq1_inspection_effort.py
------------------------
SANER revision: practical value of the slice as an INSPECTION ORDER.

An auditor handed a vulnerable function reads lines in some order. We compare
  top-down          : read the function from the first line (no tool),
  slice-first       : read the backward slice top-down, then the rest,
  sink-proximity    : read the slice ordered by distance to the nearest sink
                      line (ties top-down), then the rest -- the ordering the
                      slice_audit.py tool emits,
  var-mention-first : read the variable-mention region top-down, then the rest,
  random            : expected values for a uniformly random reading order.
Two further orders separate the region from the ordering (does the slice help, or does ordering
by sink proximity alone?):
  var-mention-sink-proximity : the variable-mention region ordered by distance to the nearest
                               sink, then the rest,
  whole-sink-proximity       : every line of the function ordered by distance to the nearest
                               sink, with no slice at all.
A per-function analysis relates the gain of sink-proximity reading over top-down (IFA) to sink
density, the share of the function's lines that hold a sink.
using the effort-aware metrics common in line-level vulnerability localization:
  IFA      lines read before the first deleted fix line (initial false alarms),
  Top-k    share of functions whose first fix line is among the first k lines read,
  Effort   share of the function read before ALL deleted fix lines are seen.
Runs on the main 294 deletion pairs and on all 760 of the full test split.
Writes results/rq1_inspection_effort.json.
"""
import os
import sys
import json

import numpy as np
from scipy.stats import spearmanr, wilcoxon

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

from consolidated_study import vuln_pairs, load_all_rows, RQ1_CAP  # noqa: E402
from rq1_baselines import (changed_lines, DATA_PATH, slice_and_criteria,  # noqa: E402
                           variable_mention_region)

FULL_JSONL = os.path.join(HERE, "data", "bigvul_full", "test.jsonl")
RESULTS_PATH = os.path.join(HERE, "results", "rq1_inspection_effort.json")
KS = (1, 3, 5, 10)


def order_first(region, n):
    return sorted(region) + [i for i in range(n) if i not in region]


def sink_proximity_order(S, sinks, n):
    def dist(li):
        return min((abs(li - s) for s in sinks), default=n)
    return sorted(S, key=lambda li: (dist(li), li)) + [i for i in range(n) if i not in S]


def metrics(order, dels, n):
    pos = {li: k for k, li in enumerate(order)}
    first = min(pos[d] for d in dels)
    last = max(pos[d] for d in dels)
    return {"ifa": first, "effort": (last + 1) / n, **{f"top{k}": float(first < k) for k in KS}}


def random_expectation(dels, n):
    # Uniform random order: first fix line position ~ min of |D| draws without replacement.
    d = len(dels)
    ifa = (n - d) / (d + 1)                                   # E[#non-fix lines before first fix]
    effort = (d * (n + 1) / (d + 1)) / n                      # E[position of last fix line] / n
    top = {}
    for k in KS:
        # P(no fix line among first k) = C(n-d, k) / C(n, k)
        p_none = 1.0
        for j in range(min(k, n)):
            p_none *= max(0, (n - d - j)) / (n - j)
        top[f"top{k}"] = 1 - p_none
    return {"ifa": ifa, "effort": effort, **top}


def run(pairs):
    per = {"top_down": [], "slice_first": [], "sink_proximity": [], "var_mention_first": [], "random": [],
           "var_mention_sink_proximity": [], "whole_sink_proximity": []}
    density = []
    for pr in pairs:
        fbl = pr["func_before"].splitlines()
        n = len(fbl)
        dels, _ = changed_lines(fbl, pr["func_after"].splitlines())
        if not n or not dels:
            continue
        S, crit_lines, sinks = slice_and_criteria(pr["func_before"])
        vm, _ = variable_mention_region(fbl, crit_lines)
        per["top_down"].append(metrics(list(range(n)), dels, n))
        per["slice_first"].append(metrics(order_first(S, n), dels, n))
        per["sink_proximity"].append(metrics(sink_proximity_order(S, sinks, n), dels, n))
        per["var_mention_first"].append(metrics(order_first(vm, n), dels, n))
        per["random"].append(random_expectation(dels, n))
        per["var_mention_sink_proximity"].append(metrics(sink_proximity_order(set(vm), sinks, n), dels, n))
        per["whole_sink_proximity"].append(metrics(sink_proximity_order(set(range(n)), sinks, n), dels, n))
        density.append(len(set(sinks)) / n)
    out = {"n": len(per["top_down"])}
    for k, rows in per.items():
        out[k] = {m: float(np.mean([r[m] for r in rows])) for m in rows[0]}
        out[k]["ifa_median"] = float(np.median([r["ifa"] for r in rows]))
    for tool in ("slice_first", "sink_proximity"):
        sf = [r["ifa"] for r in per[tool]]
        for base in ("top_down", "var_mention_first", "random"):
            b = [r["ifa"] for r in per[base]]
            out[f"p_ifa_{tool}_vs_{base}"] = float(wilcoxon(sf, b).pvalue)
    sp = [r["ifa"] for r in per["sink_proximity"]]
    for base in ("var_mention_sink_proximity", "whole_sink_proximity"):
        b = [r["ifa"] for r in per[base]]
        diff = [x - y for x, y in zip(sp, b)]
        out[f"p_ifa_sink_proximity_vs_{base}"] = (float(wilcoxon(sp, b).pvalue)
                                                  if any(diff) else 1.0)
    # Does the gain over top-down depend on sink density? (gain > 0: fewer lines read)
    gain = [t["ifa"] - s["ifa"] for t, s in zip(per["top_down"], per["sink_proximity"])]
    top3_gain = [s["top3"] - t["top3"] for t, s in zip(per["top_down"], per["sink_proximity"])]
    rho, p = spearmanr(density, gain)
    q = np.quantile(density, [0.25, 0.5, 0.75])
    bins = np.digitize(density, q)
    out["sink_density"] = {
        "mean": float(np.mean(density)), "quartile_edges": [float(x) for x in q],
        "spearman_rho_density_vs_ifa_gain": float(rho), "spearman_p": float(p),
        "by_quartile": [{"quartile": i + 1, "n": int((bins == i).sum()),
                         "mean_density": float(np.mean([d for d, b in zip(density, bins) if b == i])),
                         "mean_ifa_gain": float(np.mean([g for g, b in zip(gain, bins) if b == i])),
                         "top3_gain": float(np.mean([g for g, b in zip(top3_gain, bins) if b == i]))}
                        for i in range(4)],
    }
    return out


def main():
    out = {"main_sample": run(vuln_pairs(load_all_rows(DATA_PATH), RQ1_CAP)),
           "full_population": run(vuln_pairs(load_all_rows(FULL_JSONL), 10 ** 9))}
    with open(RESULTS_PATH, "w") as f:
        json.dump(out, f, indent=2)
    print(json.dumps(out, indent=2))


if __name__ == "__main__":
    main()
