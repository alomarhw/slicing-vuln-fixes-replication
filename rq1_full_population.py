"""
rq1_full_population.py
----------------------
SANER revision: representativeness check for the RQ1 sample.

The main study takes the first 400 qualifying CVE pairs (dataset order) from
the first 20,000 rows of the BigVul test split. This script re-runs the
unchanged rq1_baselines.py analysis on EVERY qualifying pair in the full
33,050-row test split (data/bigvul_full/test.parquet, downloaded from the
HuggingFace parquet export of bstee615/bigvul), and reports the project and
CWE composition of both populations plus the slice's RSR distribution.

Writes results/rq1_full_population.json. The main-study files are untouched.
"""
import os
import sys
import json
import collections

import numpy as np
import pandas as pd

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

import rq1_baselines  # noqa: E402
from consolidated_study import vuln_pairs, load_all_rows, RQ1_CAP  # noqa: E402

FULL_PARQUET = os.path.join(HERE, "data", "bigvul_full", "test.parquet")
FULL_JSONL = os.path.join(HERE, "data", "bigvul_full", "test.jsonl")
RESULTS_PATH = os.path.join(HERE, "results", "rq1_full_population.json")


def _composition(rows, pairs_idx):
    projects = collections.Counter(rows[i].get("project") or "" for i in pairs_idx)
    cwes = collections.Counter(rows[i].get("CWE ID") or "unlabeled" for i in pairs_idx)
    n = len(pairs_idx)
    return {
        "n_pairs": n,
        "n_projects": len(projects),
        "n_cves": len({rows[i].get("CVE ID") for i in pairs_idx}),
        "top_projects": [(p, c, round(100 * c / n, 1)) for p, c in projects.most_common(8)],
        "top_cwes": [(w, c, round(100 * c / n, 1)) for w, c in cwes.most_common(10)],
    }


def _qualifying_indices(rows):
    out = []
    for i, r in enumerate(rows):
        if str(r.get("vul")) not in ("1", "True", "true"):
            continue
        fb, fa = r.get("func_before"), r.get("func_after")
        if isinstance(fb, str) and isinstance(fa, str) and fa.strip() and fa != fb:
            out.append(i)
    return out


def main():
    if not os.path.exists(FULL_JSONL):
        pd.read_parquet(FULL_PARQUET).to_json(FULL_JSONL, orient="records", lines=True)
    rows = load_all_rows(FULL_JSONL)
    q = _qualifying_indices(rows)
    first400 = q[:RQ1_CAP]
    assert len(vuln_pairs(rows, 10 ** 9)) == len(q)

    # Re-run the unchanged RQ1 analysis on all qualifying pairs.
    rq1_baselines.DATA_PATH = FULL_JSONL
    rq1_baselines.RQ1_CAP = 10 ** 9
    rq1_baselines.RESULTS_PATH = os.path.join(HERE, "results", "_rq1_full_tmp.json")
    rq1_baselines.main()
    full = json.load(open(rq1_baselines.RESULTS_PATH))
    os.remove(rq1_baselines.RESULTS_PATH)

    s = full["summary"]
    pp = full["per_pair"]
    dele = [p for p in pp if "slice_cov_del" in p]
    rsr = np.array([p["rsr"] for p in dele])
    cov = np.array([p["slice_cov_del"] for p in dele])
    rng = np.random.default_rng(1337)
    boot = [cov[rng.integers(0, len(cov), len(cov))].mean() for _ in range(2000)]
    out = {
        "source": "bstee615/bigvul test split, all 33,050 rows",
        "n_rows": len(rows),
        "composition_all_qualifying": _composition(rows, q),
        "composition_first_400": _composition(rows, first400),
        "rq1_all_qualifying": {
            "n_deletion_pairs": s["n_deletion_pairs"],
            "n_insertion_only_pairs": s["n_insertion_only_pairs"],
            "mean_coverage": float(cov.mean()),
            "coverage_ci95": [float(np.percentile(boot, 2.5)), float(np.percentile(boot, 97.5))],
            "median_coverage": float(np.median(cov)),
            "mean_rsr": float(rsr.mean()),
            "lift_vs_random_lines": s["lift_vs_random_lines"],
            "baselines": s["baselines"],
            "insertion_hit_slice": s["slice"]["mean_hitrate_insertion"],
        },
        "rsr_distribution_all_qualifying": {
            "quartiles": [float(x) for x in np.percentile(rsr, [25, 50, 75])],
            "pct_rsr_1": float(100 * np.mean(rsr >= 0.999)),
            "pct_rsr_ge_0_9": float(100 * np.mean(rsr >= 0.9)),
            "pct_rsr_le_0_25": float(100 * np.mean(rsr <= 0.25)),
        },
    }
    with open(RESULTS_PATH, "w") as f:
        json.dump(out, f, indent=2)
    print(json.dumps(out, indent=2))


if __name__ == "__main__":
    main()
