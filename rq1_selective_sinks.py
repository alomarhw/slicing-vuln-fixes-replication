"""
rq1_selective_sinks.py
----------------------
SANER revision follow-up to rq1_baselines.py.

  1. Selective-sink sensitivity analysis. The paper's slicer treats ANY call
     expression (plus subscripts, dereferences, -> fields and pointer
     arithmetic) as a sink, so the slice keeps ~57% of the function. Here we
     re-run the identical backward slicer with two narrower criterion sets:
       - "memory": calls to the fixed memory/string/exec API list
                   (ast_slicer._SENSITIVE_CALLS), array subscripts, and
                   pointer dereferences;
       - "api":    calls to the fixed API list only.
     Functions with no matching criterion fall back to the last statement
     (same rule as the main slicer); the fallback rate is reported and the
     metrics are also given on the matched-only subset.
     Each variant is compared to (a) random lines of the same size, whose
     expected deletion coverage is exactly |S|/n, and (b) a +/-k sink window
     around the variant's own sinks, grown until it reaches |S| lines.

  2. Per-stage wall-clock timing (tree-sitter slice; srcML feature
     extraction) on the same 400 functions.

The original slicer and rq1_baselines.json are not modified.
"""
import os
import sys
import json
import time
import statistics

import numpy as np
from scipy.stats import wilcoxon

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

import ast_slicer  # noqa: E402
from consolidated_study import vuln_pairs, load_all_rows, RQ1_CAP  # noqa: E402
from rq1_baselines import changed_lines, sink_window, DATA_PATH  # noqa: E402

RESULTS_PATH = os.path.join(HERE, "results", "rq1_selective_sinks.json")
_ORIG_IS_CRITERION = ast_slicer._is_criterion


def _calls_sensitive(node, code_bytes):
    stack = [node]
    while stack:
        n = stack.pop()
        if n.type == "call_expression":
            fn = n.child_by_field_name("function")
            if fn is not None and ast_slicer._node_text(fn, code_bytes).strip() in ast_slicer._SENSITIVE_CALLS:
                return True
        stack.extend(n.children)
    return False


def _crit_memory(stmt, code_bytes):
    return (_calls_sensitive(stmt, code_bytes)
            or ast_slicer._contains_type(stmt, {"subscript_expression"}, code_bytes)
            or ast_slicer._contains_type(stmt, {"pointer_expression"}, code_bytes))


def _crit_api(stmt, code_bytes):
    return _calls_sensitive(stmt, code_bytes)


VARIANTS = {"all_sinks": _ORIG_IS_CRITERION, "memory": _crit_memory, "api": _crit_api}


def _sinks(fb, crit):
    code_bytes = fb.encode("utf-8", "replace")
    root = ast_slicer._PARSER.parse(code_bytes).root_node
    body, _ = ast_slicer._find_function_body(root)
    if body is None:
        return set()
    stmts, _ = ast_slicer._gather_statements(body, code_bytes)
    return {s.node.start_point[0] for s in stmts if crit(s.node, code_bytes)}


def _cliffs(a, b):
    a, b = np.asarray(a), np.asarray(b)
    gt = sum((x > b).sum() for x in a)
    lt = sum((x < b).sum() for x in a)
    return float((gt - lt) / (len(a) * len(b)))


def _test(x, y):
    d = np.asarray(x) - np.asarray(y)
    if not np.any(d):
        return None
    return float(wilcoxon(x, y, zero_method="wilcox").pvalue)


def run_variant(name, crit, pairs):
    ast_slicer._is_criterion = crit
    recs = []
    for pr in pairs:
        fb, fa = pr["func_before"], pr["func_after"]
        fb_lines = fb.splitlines()
        n = len(fb_lines)
        dels, _ = changed_lines(fb_lines, fa.splitlines())
        if n == 0 or not dels:
            continue
        S = set(ast_slicer.semantic_slice_indices(fb))
        sinks = _sinks(fb, crit)
        k = len(S)
        sw = set()
        if sinks:
            kk = 0
            sw = sink_window(n, kk, sinks)
            while len(sw) < k and kk < n:
                kk += 1
                sw = sink_window(n, kk, sinks)
        recs.append({
            "matched": bool(sinks),
            "cov": len(S & dels) / len(dels),
            "rsr": k / n,
            "rand": k / n,  # exact expectation for a uniform random k-subset
            "sw_cov": (len(sw & dels) / len(dels)) if sinks else None,
            "sw_rsr": (len(sw) / n) if sinks else None,
        })
    ast_slicer._is_criterion = _ORIG_IS_CRITERION

    def summ(rs):
        if not rs:
            return {"n": 0}
        cov = [r["cov"] for r in rs]
        rnd = [r["rand"] for r in rs]
        out = {
            "n": len(rs),
            "mean_coverage": float(np.mean(cov)),
            "median_coverage": float(np.median(cov)),
            "mean_rsr": float(np.mean([r["rsr"] for r in rs])),
            "random_lines_expected_coverage": float(np.mean(rnd)),
            "lift_vs_random": float(np.mean(cov) / np.mean(rnd)) if np.mean(rnd) else None,
            "p_vs_random": _test(cov, rnd),
            "cliffs_vs_random": _cliffs(cov, rnd),
        }
        sw = [r for r in rs if r["sw_cov"] is not None]
        if sw:
            out.update({
                "sink_window_n": len(sw),
                "sink_window_mean_coverage": float(np.mean([r["sw_cov"] for r in sw])),
                "sink_window_mean_rsr": float(np.mean([r["sw_rsr"] for r in sw])),
                "slice_mean_coverage_same_subset": float(np.mean([r["cov"] for r in sw])),
                "slice_mean_rsr_same_subset": float(np.mean([r["rsr"] for r in sw])),
                "p_vs_sink_window": _test([r["cov"] for r in sw], [r["sw_cov"] for r in sw]),
                "cliffs_vs_sink_window": _cliffs([r["cov"] for r in sw], [r["sw_cov"] for r in sw]),
            })
        return out

    return {
        "variant": name,
        "fallback_rate": float(np.mean([not r["matched"] for r in recs])),
        "all": summ(recs),
        "matched_only": summ([r for r in recs if r["matched"]]),
    }


def run_timing(pairs):
    from augmented_study import srcml_features  # noqa: PLC0415
    t_slice, t_srcml = [], []
    for pr in pairs:
        fb = pr["func_before"]
        t0 = time.perf_counter()
        ast_slicer.semantic_slice_indices(fb)
        t1 = time.perf_counter()
        try:
            srcml_features(fb)
        except Exception:
            pass
        t2 = time.perf_counter()
        t_slice.append((t1 - t0) * 1000)
        t_srcml.append((t2 - t1) * 1000)

    def q(xs):
        return {"median_ms": statistics.median(xs), "mean_ms": statistics.mean(xs),
                "p95_ms": float(np.percentile(xs, 95))}
    return {"n_functions": len(pairs), "slice": q(t_slice), "srcml_features": q(t_srcml),
            "note": "Wall-clock per function, single process; srcml_features includes the srcml "
                    "subprocess launch. Machine: " + os.uname().machine}


FULL_JSONL = os.path.join(HERE, "data", "bigvul_full", "test.jsonl")
FULL_PARQUET = os.path.join(HERE, "data", "bigvul_full", "test.parquet")


def ensure_full_jsonl():
    """The full-population JSONL is derived from the shipped parquet (as in rq1_full_population.py)."""
    if not os.path.exists(FULL_JSONL) and os.path.exists(FULL_PARQUET):
        import pandas as pd  # noqa: PLC0415
        pd.read_parquet(FULL_PARQUET).to_json(FULL_JSONL, orient="records", lines=True)


def main():
    rows = load_all_rows(DATA_PATH)
    pairs = vuln_pairs(rows, RQ1_CAP)
    out = {"n_candidates": len(pairs),
           "variants": [run_variant(n, c, pairs) for n, c in VARIANTS.items()],
           "timing": run_timing(pairs)}
    ensure_full_jsonl()
    if os.path.exists(FULL_JSONL):
        full = vuln_pairs(load_all_rows(FULL_JSONL), 10 ** 9)
        out["full_population"] = {"n_candidates": len(full),
                                  "variants": [run_variant(n, c, full) for n, c in VARIANTS.items()]}
    with open(RESULTS_PATH, "w") as f:
        json.dump(out, f, indent=2)
    print(json.dumps(out, indent=2))
    print("written to", RESULTS_PATH)


if __name__ == "__main__":
    main()
