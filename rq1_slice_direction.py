"""
rq1_slice_direction.py
----------------------
SANER revision: does slice DIRECTION matter for fix localization?

The main study slices BACKWARD from sinks (the SySeVR convention: which
statements can influence a potentially unsafe operation). This script adds
the FORWARD view with srcSlice, which computes forward slices per variable:

  forward  F = srcSlice forward slice of the function's parameters (its
               external inputs): the def/use lines of each parameter, closed
               transitively over srcSlice's dependent variables (dvars) and
               aliases (pointers). This is the taint-style "what can the input
               affect" region.
  chop     F & B, with B the backward slice from sinks (source-to-sink chop).
  union    F | B.
  backward_data  B with control dependences disabled, to separate slice
                 direction from control dependence (srcSlice slices are
                 data-flow only).

srcml is run with --position so srcSlice reports source line numbers (without
it this srcSlice build reports line 0 for every def/use). Coverage/RSR/lift
follow rq1_selective_sinks.py; random-lines expected coverage is exactly |R|/n.
Runs on the main 294-pair sample and on all qualifying deletion pairs of the
full test split. Writes results/rq1_slice_direction.json.
"""
import os
import sys
import json
import tempfile
import subprocess

import numpy as np
from scipy.stats import wilcoxon

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

import ast_slicer  # noqa: E402
from consolidated_study import vuln_pairs, load_all_rows, RQ1_CAP  # noqa: E402
from proto_dualsig import parse_srcslice_output, SRCML_BIN, SRCSLICE_BIN  # noqa: E402
from rq1_baselines import changed_lines, DATA_PATH  # noqa: E402

FULL_JSONL = os.path.join(HERE, "data", "bigvul_full", "test.jsonl")
RESULTS_PATH = os.path.join(HERE, "results", "rq1_slice_direction.json")


def srcslice_forward_profiles(code):
    fd, cpath = tempfile.mkstemp(suffix=".c")
    with os.fdopen(fd, "w", encoding="utf-8", errors="replace") as fh:
        fh.write(code if code.endswith("\n") else code + "\n")
    xpath = cpath + ".xml"
    try:
        r = subprocess.run([SRCML_BIN, "--position", cpath, "-o", xpath],
                           capture_output=True, timeout=30)
        if r.returncode != 0 or not os.path.exists(xpath):
            return None
        r = subprocess.run([SRCSLICE_BIN, xpath], capture_output=True, text=True, timeout=30)
        return parse_srcslice_output(r.stdout) if r.stdout.strip() else None
    except Exception:
        return None
    finally:
        for p in (cpath, xpath):
            try:
                os.unlink(p)
            except OSError:
                pass


def parameters(code):
    try:
        cb = code.encode("utf-8", "replace")
        root = ast_slicer._PARSER.parse(cb).root_node
        _, fn = ast_slicer._find_function_body(root)
        return set(ast_slicer._function_parameters(fn, cb)) if fn is not None else set()
    except Exception:
        return set()


def forward_from(profiles, sources, n):
    by_var = {}
    for p in profiles:
        by_var.setdefault(p["variable"], []).append(p)
    seen, work, lines = set(), [v for v in sources if v in by_var], set()
    while work:
        v = work.pop()
        if v in seen:
            continue
        seen.add(v)
        for p in by_var.get(v, []):
            lines |= {ln - 1 for ln in (p["def"] | p["use"]) if 1 <= ln <= n}
            work.extend(x for x in (p["dvars"] | p["pointers"]) if x in by_var and x not in seen)
    return lines


def _p(x, y):
    d = np.asarray(x) - np.asarray(y)
    return float(wilcoxon(x, y).pvalue) if np.any(d) else None


def summarize(recs, key):
    rs = [r for r in recs if r.get(key + "_rsr")]
    if not rs:
        return {"n": 0}
    cov = [r[key + "_cov"] for r in rs]
    rsr = [r[key + "_rsr"] for r in rs]
    return {"n": len(rs), "mean_coverage": float(np.mean(cov)), "median_coverage": float(np.median(cov)),
            "mean_rsr": float(np.mean(rsr)), "lift_vs_random": float(np.mean(cov) / np.mean(rsr)),
            "p_vs_random": _p(cov, rsr)}


def run(pairs, label):
    recs, n_fail, n_noparam = [], 0, 0
    for pr in pairs:
        fb, fa = pr["func_before"], pr["func_after"]
        fbl = fb.splitlines()
        n = len(fbl)
        dels, _ = changed_lines(fbl, fa.splitlines())
        if not n or not dels:
            continue
        B = set(ast_slicer.semantic_slice_indices(fb))
        _orig = ast_slicer._enclosing_predicates
        ast_slicer._enclosing_predicates = lambda *_a, **_k: []   # data dependences only
        try:
            Bd = set(ast_slicer.semantic_slice_indices(fb))
        finally:
            ast_slicer._enclosing_predicates = _orig
        prof = srcslice_forward_profiles(fb)
        rec = {"n": n, "lang": pr.get("lang") or "?", "backward_cov": len(B & dels) / len(dels), "backward_rsr": len(B) / n}
        if prof is None:
            n_fail += 1
            recs.append(rec)
            continue
        params = parameters(fb)
        if not params:
            # tree-sitter's C grammar cannot parse C++ method signatures; fall back to the
            # variables srcSlice itself reports as defined on the signature lines.
            header_end = next((i + 1 for i, ln in enumerate(fbl) if "{" in ln), 1)
            params = {p["variable"] for p in prof if any(1 <= d <= header_end for d in p["def"])}
        F = forward_from(prof, params, n)
        if not F:
            n_noparam += 1
            recs.append(rec)
            continue
        for key, R in (("forward", F), ("chop", F & B), ("union", F | B), ("backward_data", Bd)):
            if R:
                rec[key + "_cov"] = len(R & dels) / len(dels)
                rec[key + "_rsr"] = len(R) / n
        recs.append(rec)
    both = [r for r in recs if "forward_rsr" in r]
    langs = {}
    for r in recs:
        langs.setdefault(r["lang"], [0, 0])
        langs[r["lang"]][0] += 1
        langs[r["lang"]][1] += "forward_rsr" in r
    out = {"label": label, "n_deletion_pairs": len(recs), "srcslice_failed": n_fail,
           "no_forward_slice": n_noparam, "n_with_forward": len(both),
           "by_language_total_and_with_forward": langs}
    for key in ("backward", "backward_data", "forward", "chop", "union"):
        out[key] = summarize(both, key)          # same pairs for every region
    out["backward_all_pairs"] = summarize(recs, "backward")
    fc = [r["forward_cov"] for r in both]
    bc = [r["backward_cov"] for r in both]
    out["forward_vs_backward_coverage_p"] = _p(fc, bc)
    out["forward_vs_backward_data_coverage_p"] = _p(fc, [r["backward_data_cov"] for r in both])
    return out


def pairs_with_lang(rows, cap):
    """vuln_pairs() plus the language field (same filter, same order)."""
    out = []
    for r in rows:
        fb, fa = r.get("func_before"), r.get("func_after")
        if str(r.get("vul")) != "1" or not isinstance(fb, str) or not isinstance(fa, str):
            continue
        if not fa.strip() or fa == fb:
            continue
        out.append({"func_before": fb, "func_after": fa, "lang": r.get("lang") or "?"})
        if len(out) >= cap:
            break
    return out


def main():
    main_pairs = pairs_with_lang(load_all_rows(DATA_PATH), RQ1_CAP)
    full_pairs = pairs_with_lang(load_all_rows(FULL_JSONL), 10 ** 9)
    assert len(main_pairs) == len(vuln_pairs(load_all_rows(DATA_PATH), RQ1_CAP))
    out = {"main_sample": run(main_pairs, "first 400 pairs (main)"),
           "full_population": run(full_pairs, "all qualifying pairs, full test split")}
    with open(RESULTS_PATH, "w") as f:
        json.dump(out, f, indent=2)
    print(json.dumps(out, indent=2))


if __name__ == "__main__":
    main()
