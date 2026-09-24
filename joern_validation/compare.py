"""
joern_validation/compare.py
---------------------------
SANER revision: validate the tree-sitter backward slicer against a Joern
reference slice (backward_slice.sc: same sink categories, Joern's
flow-sensitive reaching definitions + control dependences).

Both slices are mapped onto the same statement units (ast_slicer statements):
a statement is in the Joern slice if Joern marks any line it spans. We report
statement-level agreement (precision/recall/Jaccard of ours w.r.t. Joern) and
re-run the RQ1 comparison (coverage, RSR, lift, variable-mention baseline) with
the Joern slice on the same functions. Writes ../results/joern_validation.json.
"""
import os
import sys
import json
import collections

import numpy as np
from scipy.stats import wilcoxon

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, ROOT)
os.chdir(ROOT)

import ast_slicer  # noqa: E402
from consolidated_study import vuln_pairs, load_all_rows, RQ1_CAP  # noqa: E402
from rq1_baselines import (changed_lines, DATA_PATH, slice_and_criteria,  # noqa: E402
                           variable_mention_region)

OUT = os.path.join(ROOT, "results", "joern_validation.json")


def statements(code):
    cb = code.encode("utf-8", "replace")
    body, _ = ast_slicer._find_function_body(ast_slicer._PARSER.parse(cb).root_node)
    if body is None:
        return []
    stmts, _ = ast_slicer._gather_statements(body, cb)
    return [(s.start_line, s.end_line) for s in stmts]


def to_units(lines0, units):
    return {i for i, (lo, hi) in enumerate(units) if any(lo <= ln <= hi for ln in lines0)}


def span(unit_ids, units, n):
    return {li for i in unit_ids for li in range(max(0, units[i][0]), units[i][1] + 1) if li < n}


def _p(x, y):
    return float(wilcoxon(x, y).pvalue) if np.any(np.asarray(x) - np.asarray(y)) else None


def main():
    pairs = vuln_pairs(load_all_rows(DATA_PATH), RQ1_CAP)
    by_file = collections.defaultdict(list)
    for line in open(os.path.join(HERE, "joern_slices.jsonl")):
        r = json.loads(line)
        by_file[os.path.basename(r["file"])].append(r)

    recs, skipped = [], collections.Counter()
    for fname, methods in sorted(by_file.items()):
        i = int(fname[1:4])
        fb, fa = pairs[i]["func_before"], pairs[i]["func_after"]
        fbl = fb.splitlines()
        n = len(fbl)
        dels, _ = changed_lines(fbl, fa.splitlines())
        m = max(methods, key=lambda r: len(r["lines"]))        # the function, not macro stubs
        J0 = {ln - 1 for ln in m["lines"] if 1 <= ln <= n}
        units = statements(fb)
        S, crit_lines, _ = slice_and_criteria(fb)
        if not units or not J0 or len(S) == n and not crit_lines:
            skipped["no statements / empty Joern slice / whole-function fallback"] += 1
            continue
        Su = to_units(S, units)
        Ju = to_units(J0, units)
        if not Ju:
            skipped["Joern lines outside parsed statements"] += 1
            continue
        J = span(Ju, units, n)
        vm, _ = variable_mention_region(fbl, crit_lines)
        inter = len(Su & Ju)
        recs.append({
            "file": fname, "n": n,
            "precision": inter / len(Su) if Su else 0.0,   # share of our statements Joern also keeps
            "recall": inter / len(Ju),                      # share of Joern's statements we keep
            "jaccard": inter / len(Su | Ju),
            "ours_cov": len(S & dels) / len(dels), "ours_rsr": len(S) / n,
            "joern_cov": len(J & dels) / len(dels), "joern_rsr": len(J) / n,
            "vm_cov": (len(vm & dels) / len(dels)) if vm else 0.0, "vm_rsr": len(vm) / n,
        })

    def m(k):
        return float(np.mean([r[k] for r in recs]))
    oc, jc, vc = ([r[k] for r in recs] for k in ("ours_cov", "joern_cov", "vm_cov"))
    out = {
        "n_functions_exported": 294, "n_joern_methods": len(by_file), "n_compared": len(recs),
        "skipped": dict(skipped),
        "agreement": {"precision_mean": m("precision"), "recall_mean": m("recall"),
                      "jaccard_mean": m("jaccard"), "jaccard_median": float(np.median([r["jaccard"] for r in recs])),
                      "pct_jaccard_ge_0_8": float(100 * np.mean([r["jaccard"] >= 0.8 for r in recs]))},
        "rq1_on_same_functions": {
            "ours": {"coverage": m("ours_cov"), "rsr": m("ours_rsr"), "lift": m("ours_cov") / m("ours_rsr")},
            "joern": {"coverage": m("joern_cov"), "rsr": m("joern_rsr"), "lift": m("joern_cov") / m("joern_rsr")},
            "variable_mention": {"coverage": m("vm_cov"), "rsr": m("vm_rsr")},
            "p_ours_vs_joern_cov": _p(oc, jc),
            "p_joern_vs_vm_cov": _p(jc, vc),
            "cliffs_joern_vs_vm": float(np.mean([np.sign(a - b) for a in jc for b in vc])),
        },
    }
    json.dump({"summary": out, "per_function": recs}, open(OUT, "w"), indent=2)
    print(json.dumps(out, indent=2))


if __name__ == "__main__":
    main()
