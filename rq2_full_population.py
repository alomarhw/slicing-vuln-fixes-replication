"""
rq2_full_population.py
----------------------
SANER revision: repeat RQ2 (fix-signature discrimination) on EVERY qualifying
CVE pair of the full 33,050-row BigVul test split, using the unchanged
augmented_study.run_rq2 (same features, same srcSlice coarse arm, same 70/30
held-out dual-signature protocol, same statistics). Only the pair cap and the
input rows differ from the main study.

Needs data/bigvul_full/test.jsonl (written by rq1_full_population.py).
Writes results/rq2_full_population.json.
"""
import os
import sys
import json

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

import augmented_study  # noqa: E402

FULL_JSONL = os.path.join(HERE, "data", "bigvul_full", "test.jsonl")
RESULTS_PATH = os.path.join(HERE, "results", "rq2_full_population.json")


def main():
    augmented_study.DATA_PATH = FULL_JSONL
    augmented_study.RQ2_CAP = 10 ** 9
    rows = augmented_study.load_rows()
    res = augmented_study.run_rq2(rows)
    out = {"source": "bstee615/bigvul test split, all rows", "n_rows": len(rows),
           "rq2_aug": res, "tool_failures": dict(augmented_study._fail)}
    with open(RESULTS_PATH, "w") as f:
        json.dump(out, f, indent=2, default=augmented_study._json_default)
    print("written", RESULTS_PATH)


if __name__ == "__main__":
    main()
