"""Trace the motivating example (CVE-2017-13006, tcpdump l2tp_q931_cc_print) through the slicer.

Prints, for func_before and func_after: each sink statement with the SySeVR category that makes it
a sink, the worklist order of Algorithm 1 (which statement is popped, and what it adds through data
or control dependence), the slice S, the deleted/added lines, Coverage and RSR, the variable-mention
region, the srcML feature sets phi over each version's slice with the fix's delta, and the srcML
markup of line 5 with the phi tokens it yields. The trace
replays the slicer's own helpers and asserts that it reproduces ast_slicer.semantic_slice_indices.
Writes results/example_trace.json.
"""

from __future__ import annotations

import difflib
import json
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import ast_slicer as A  # noqa: E402
from augmented_study import feat_set, varmention_idx  # noqa: E402

CVE, FUNC = "CVE-2017-13006", "l2tp_q931_cc_print"


def load_pair():
    with open(os.path.join(HERE, "data", "bigvul", "sample.jsonl")) as fh:
        for line in fh:
            r = json.loads(line)
            if r.get("CVE ID") == CVE and FUNC in (r.get("func_before") or ""):
                return r["func_before"], r["func_after"]
    raise SystemExit(f"{CVE} not found in data/bigvul/sample.jsonl")


def sink_reasons(node, code_bytes):
    reasons = []
    if A._contains_sensitive_or_any_call(node, code_bytes):
        reasons.append("call")
    if A._contains_type(node, {"subscript_expression"}, code_bytes):
        reasons.append("subscript")
    if A._contains_type(node, {"pointer_expression"}, code_bytes):
        reasons.append("pointer * or &")
    if A._has_arrow_field(node, code_bytes):
        reasons.append("-> field")
    if A._contains_pointer_arith(node, code_bytes):
        reasons.append("arithmetic over identifier")
    return reasons


def trace(code):
    """Replay semantic_slice_indices step by step (same helpers, same order)."""
    code_bytes = code.encode("utf-8", "replace")
    root = A._PARSER.parse(code_bytes).root_node
    body, func_node = A._find_function_body(root)
    stmts, predicate_index = A._gather_statements(body, code_bytes)
    params = A._function_parameters(func_node, code_bytes)
    line = lambda s: s.start_line + 1  # noqa: E731  (1-based source line)
    criteria = [s for s in stmts if A._is_criterion(s.node, code_bytes)] or [stmts[-1]]
    sinks = [{"line": line(s), "reasons": sink_reasons(s.node, code_bytes)} for s in criteria]
    defs_by_var = {}
    for s in stmts:
        for v in s.defs:
            defs_by_var.setdefault(v, []).append(s)
    by_id = {s.sid: s for s in stmts}
    in_s, work, steps = set(), [], []
    for c in criteria:
        if c.sid not in in_s:
            in_s.add(c.sid)
            work.append(c)
    while work:
        s = work.pop()
        step = {"pop": line(s), "uses": sorted(s.uses), "data": [], "control": []}
        for v in sorted(s.uses):
            definers = defs_by_var.get(v)
            if not definers:
                continue
            before = [d for d in definers if d.start_byte <= s.start_byte]
            for d in (before or definers):
                if d.sid not in in_s:
                    in_s.add(d.sid)
                    work.append(d)
                    step["data"].append({"line": line(d), "var": v})
        for pid in A._enclosing_predicates(s.node, predicate_index):
            if pid not in in_s:
                in_s.add(pid)
                if pid in by_id:
                    work.append(by_id[pid])
                    step["control"].append(line(by_id[pid]))
        steps.append(step)
    kept = set()
    for sid in in_s:
        s = by_id[sid]
        kept |= set(range(s.start_line, s.end_line + 1))
    kept &= set(range(len(code.splitlines())))
    assert kept == A.semantic_slice_indices(code), "trace diverges from ast_slicer"
    return {"params": sorted(params), "sinks": sinks, "steps": steps,
            "slice": sorted(i + 1 for i in kept), "n": len(code.splitlines())}


def srcml_of_line(code, line):
    """srcML markup of the statement starting on a 1-based line (the if on line 5 in the paper)."""
    with tempfile.NamedTemporaryFile("w", suffix=".c", delete=False) as fh:
        fh.write(code)
        path = fh.name
    try:
        xml = subprocess.run(["srcml", path], capture_output=True, text=True, check=True).stdout
    finally:
        os.unlink(path)
    target = code.splitlines()[line - 1].strip().split("(")[0].strip()  # "if"
    start = xml.index(f"<{target}_stmt>") if f"<{target}_stmt>" in xml else 0
    return xml[start:xml.index("</condition>", start) + len("</condition>")]


def main():
    before, after = load_pair()
    tb, ta = trace(before), trace(after)
    b_lines, a_lines = before.splitlines(), after.splitlines()
    deleted, added = [], []
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, b_lines, a_lines, autojunk=False).get_opcodes():
        if tag in ("replace", "delete"):
            deleted += range(i1 + 1, i2 + 1)
        if tag in ("replace", "insert"):
            added += range(j1 + 1, j2 + 1)
    s = set(tb["slice"])
    phi_b = feat_set(before, [i - 1 for i in tb["slice"]])
    phi_a = feat_set(after, [i - 1 for i in ta["slice"]])
    out = {
        "cve": CVE, "function": FUNC,
        "before": tb, "after": ta, "deleted_lines": deleted, "added_lines": added,
        "coverage": len(s & set(deleted)) / len(deleted), "rsr": f"{len(s)}/{tb['n']}",
        "variable_mention_region": sorted(i + 1 for i in varmention_idx(before)),
        "phi_before_slice": sorted(phi_b), "phi_after_slice": sorted(phi_a),
        "phi_removed": sorted(phi_b - phi_a), "phi_added": sorted(phi_a - phi_b),
        "phi_shared_count": len(phi_b & phi_a),
        "srcml_line5": srcml_of_line(before, 5), "phi_line5": sorted(feat_set(before, [4])),
    }
    os.makedirs(os.path.join(HERE, "results"), exist_ok=True)
    with open(os.path.join(HERE, "results", "example_trace.json"), "w") as fh:
        json.dump(out, fh, indent=2)
    print(json.dumps(out, indent=2))


if __name__ == "__main__":
    main()
