#!/usr/bin/env python3
"""
analyze_sink_fallback.py
--------------------------
Answers a Reviewer question: across the 294 real CVE pairs used for RQ1/RQ2,
how often does sink-criterion identification fall back to "last non-blank
line" (no vulnerability-pattern match found), and does coverage/RSR differ
between matched-criterion and fallback-criterion cases?

Reuses ast_slicer's own internal statement-gathering and criterion-matching
logic (does not duplicate it) and consolidated_study's vuln_pairs() loader,
so this is exactly the same criterion-selection path RQ1 actually runs, just
instrumented to report whether a real match or the fallback was used.

Writes: results/sink_fallback_analysis.json
"""

import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from ast_slicer import (  # noqa: E402
    _PARSER, _TS_OK, _find_function_body, _gather_statements, _is_criterion,
    semantic_slice_indices,
)
from consolidated_study import vuln_pairs, load_all_rows, RQ1_CAP, changed_lines, DATA_PATH  # noqa: E402


def load_rows():
    return load_all_rows(DATA_PATH)

RESULTS_PATH = os.path.join("results", "sink_fallback_analysis.json")


def criterion_used_fallback(code: str):
    """Returns True if no vulnerability-pattern criterion matched (the last
    non-blank statement was used as a conservative fallback), False if a real
    sink pattern matched, None if slicing itself fell back to whole-function
    (parse failure / no body / no statements -- a different, whole-function
    fallback, tracked separately)."""
    if not _TS_OK or _PARSER is None:
        return None
    try:
        code_bytes = code.encode("utf-8", "replace")
        tree = _PARSER.parse(code_bytes)
        root = tree.root_node
        body, func_node = _find_function_body(root)
        if body is None:
            return None
        stmts, _predicate_index = _gather_statements(body, code_bytes)
        if not stmts:
            return None
        criteria = [s for s in stmts if _is_criterion(s.node, code_bytes)]
        return len(criteria) == 0  # True -> fallback (last stmt used)
    except Exception:
        return None


def main():
    rows = load_rows()
    pairs = vuln_pairs(rows, RQ1_CAP)
    print(f"analyzing {len(pairs)} CVE pairs (RQ1_CAP={RQ1_CAP})")

    matched_cov, matched_rsr = [], []
    fallback_cov, fallback_rsr = [], []
    whole_fn_fallback = 0
    n_used = 0

    for pr in pairs:
        fb = pr["func_before"]
        fa = pr["func_after"]
        fb_lines = fb.splitlines()
        fa_lines = fa.splitlines()
        n_lines = len(fb_lines)
        if n_lines == 0:
            continue
        try:
            del_lines, _add = changed_lines(fb_lines, fa_lines)
        except Exception:
            continue
        if not del_lines:
            continue

        is_fallback = criterion_used_fallback(fb)
        if is_fallback is None:
            whole_fn_fallback += 1
            continue

        slice_idx = semantic_slice_indices(fb)
        if not slice_idx:
            continue
        cov = len(slice_idx & set(del_lines)) / len(del_lines)
        rsr = len(slice_idx) / n_lines
        n_used += 1
        if is_fallback:
            fallback_cov.append(cov)
            fallback_rsr.append(rsr)
        else:
            matched_cov.append(cov)
            matched_rsr.append(rsr)

    def mean(xs):
        return sum(xs) / len(xs) if xs else None

    out = {
        "n_pairs_considered": len(pairs),
        "n_used": n_used,
        "n_whole_function_parse_fallback": whole_fn_fallback,
        "n_sink_criterion_matched": len(matched_cov),
        "n_sink_criterion_fallback": len(fallback_cov),
        "sink_criterion_fallback_rate_pct": (
            100.0 * len(fallback_cov) / n_used if n_used else None
        ),
        "matched": {
            "n": len(matched_cov),
            "mean_coverage": mean(matched_cov),
            "mean_rsr": mean(matched_rsr),
        },
        "fallback": {
            "n": len(fallback_cov),
            "mean_coverage": mean(fallback_cov),
            "mean_rsr": mean(fallback_rsr),
        },
    }
    os.makedirs("results", exist_ok=True)
    with open(RESULTS_PATH, "w") as f:
        json.dump(out, f, indent=2)
    print(json.dumps(out, indent=2))
    print("written to", RESULTS_PATH)


if __name__ == "__main__":
    main()
