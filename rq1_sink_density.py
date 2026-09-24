"""
rq1_sink_density.py
-------------------
SANER revision: does sink density explain why the backward slice and the
variable-mention region behave alike?

Per deletion pair: sink density = share of statements that match the sink
predicate; overlap = Jaccard(slice, variable-mention region); gap = slice
coverage - variable-mention coverage; and each region's RSR. Reports Spearman
correlations and a breakdown by sink-density quartile, on the main 294 pairs
and on all 760 of the full test split. Writes results/rq1_sink_density.json.
"""
import os
import sys
import json

import numpy as np
from scipy.stats import spearmanr

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

import ast_slicer  # noqa: E402
from consolidated_study import vuln_pairs, load_all_rows, RQ1_CAP  # noqa: E402
from rq1_baselines import changed_lines, DATA_PATH, slice_and_criteria, variable_mention_region  # noqa: E402

FULL_JSONL = os.path.join(HERE, "data", "bigvul_full", "test.jsonl")
OUT = os.path.join(HERE, "results", "rq1_sink_density.json")


def sink_density(code):
    cb = code.encode("utf-8", "replace")
    body, _ = ast_slicer._find_function_body(ast_slicer._PARSER.parse(cb).root_node)
    if body is None:
        return None
    stmts, _ = ast_slicer._gather_statements(body, cb)
    if not stmts:
        return None
    return sum(ast_slicer._is_criterion(s.node, cb) for s in stmts) / len(stmts)


def run(pairs):
    rows = []
    for pr in pairs:
        fbl = pr["func_before"].splitlines()
        n = len(fbl)
        dels, _ = changed_lines(fbl, pr["func_after"].splitlines())
        if not n or not dels:
            continue
        dens = sink_density(pr["func_before"])
        if dens is None:
            continue
        S, crit, _ = slice_and_criteria(pr["func_before"])
        vm, _ = variable_mention_region(fbl, crit)
        rows.append({"density": dens,
                     "jaccard": len(S & vm) / len(S | vm) if (S | vm) else 1.0,
                     "gap": len(S & dels) / len(dels) - (len(vm & dels) / len(dels) if vm else 0.0),
                     "abs_gap": abs(len(S & dels) / len(dels) - (len(vm & dels) / len(dels) if vm else 0.0)),
                     "rsr": len(S) / n, "vm_rsr": len(vm) / n})
    a = lambda k: np.array([r[k] for r in rows])  # noqa: E731
    d = a("density")
    out = {"n": len(rows), "mean_density": float(d.mean())}
    for k in ("jaccard", "abs_gap", "gap", "rsr"):
        rho, p = spearmanr(d, a(k))
        out[f"spearman_density_{k}"] = {"rho": float(rho), "p": float(p)}
    qs = np.quantile(d, [0.25, 0.5, 0.75])
    out["quartile_bounds"] = [float(x) for x in qs]
    bins = np.digitize(d, qs)
    out["by_quartile"] = []
    for b in range(4):
        m = bins == b
        out["by_quartile"].append({"quartile": f"Q{b + 1}", "n": int(m.sum()),
                                   "density": float(d[m].mean()), "jaccard": float(a("jaccard")[m].mean()),
                                   "abs_gap": float(a("abs_gap")[m].mean()), "gap": float(a("gap")[m].mean()),
                                   "rsr": float(a("rsr")[m].mean()), "vm_rsr": float(a("vm_rsr")[m].mean())})
    return out


def main():
    out = {"main_sample": run(vuln_pairs(load_all_rows(DATA_PATH), RQ1_CAP)),
           "full_population": run(vuln_pairs(load_all_rows(FULL_JSONL), 10 ** 9))}
    json.dump(out, open(OUT, "w"), indent=2)
    print(json.dumps(out, indent=2))


if __name__ == "__main__":
    main()
