"""
rq3_grouped_cv.py
-----------------
SANER revision: RQ3 sensitivity to near-duplicate leakage across folds.

The main RQ3 run uses RepeatedStratifiedKFold (30 x 5), so functions from the
same CVE can fall in both training and test folds. This script re-runs the
unchanged augmented_study.run_rq3 (same 2,500 functions, same five
representations, same classifier, same Nadeau-Bengio + Holm statistics) with
the splitter replaced by 30 repeats of StratifiedGroupKFold(5), grouping
functions by CVE ID (default) or by project (argument "project") so that no
CVE/project contributes functions to both sides.

Writes results/rq3_grouped_cv.json (CVE) or results/rq3_grouped_cv_project.json.
"""
import os
import sys
import json
import random

import numpy as np
from sklearn.model_selection import StratifiedGroupKFold

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

import augmented_study as A  # noqa: E402

GROUP_BY = sys.argv[1] if len(sys.argv) > 1 else "cve"        # "cve" or "project"
RESULTS_PATH = os.path.join(HERE, "results",
                            "rq3_grouped_cv.json" if GROUP_BY == "cve" else f"rq3_grouped_cv_{GROUP_BY}.json")
_GROUPS = {}


def detection_set_with_groups(rows):
    """Exactly augmented_study.build_detection_set, but also returns each function's CVE ID.
    random.shuffle's permutation depends only on list length and RNG state, so shuffling the
    row lists reproduces the original order; asserted against the original below."""
    pos = [r for r in rows if A._is_vuln(r.get("vul")) and isinstance(r.get("func_before"), str)
           and r["func_before"].strip()]
    neg = [r for r in rows if (r.get("vul") in (0, "0", False)) and isinstance(r.get("func_before"), str)
           and r["func_before"].strip()]
    rng = random.Random(A.SEED)
    rng.shuffle(pos)
    rng.shuffle(neg)
    max_pos = max(1, A.DET_CAP // (1 + A.DET_NEG_RATIO))
    n_pos = min(len(pos), max_pos)
    n_neg = min(len(neg), n_pos * A.DET_NEG_RATIO)
    chosen = pos[:n_pos] + neg[:n_neg]
    labels = [1] * n_pos + [0] * n_neg
    idx = list(range(len(chosen)))
    rng.shuffle(idx)
    chosen = [chosen[i] for i in idx]
    labels = [labels[i] for i in idx]
    if GROUP_BY == "project":
        groups = [(r.get("project") or f"row{i}") for i, r in enumerate(chosen)]
    else:
        groups = [(r.get("CVE ID") or r.get("commit_id") or f"row{i}") for i, r in enumerate(chosen)]
    return [r["func_before"] for r in chosen], labels, groups


class GroupedRepeatedCV:
    def __init__(self, n_splits, n_repeats, random_state):
        self.n_splits, self.n_repeats, self.random_state = n_splits, n_repeats, random_state

    def split(self, X, y):
        groups = _GROUPS["groups"]
        for r in range(self.n_repeats):
            sgkf = StratifiedGroupKFold(n_splits=self.n_splits, shuffle=True, random_state=self.random_state + r)
            yield from sgkf.split(X, y, groups)


def main():
    rows = A.load_rows()
    codes, labels, groups = detection_set_with_groups(rows)
    ref_codes, ref_labels = A.build_detection_set(rows)
    assert codes == ref_codes and labels == ref_labels, "detection set differs from the main study"
    _GROUPS["groups"] = groups
    A.build_detection_set = lambda _rows: (codes, labels)
    A.RepeatedStratifiedKFold = GroupedRepeatedCV
    res = A.run_rq3(rows)
    g = np.array(groups)
    label = "CVE ID" if GROUP_BY == "cve" else GROUP_BY
    out = {"grouping": f"{label} (StratifiedGroupKFold, 30 repeats x 5 folds)",
           "n_groups": int(len(set(groups))),
           "largest_group": int(max(np.unique(g, return_counts=True)[1])),
           "rq3_grouped": res}
    with open(RESULTS_PATH, "w") as f:
        json.dump(out, f, indent=2, default=A._json_default)
    print("written", RESULTS_PATH)


if __name__ == "__main__":
    main()
