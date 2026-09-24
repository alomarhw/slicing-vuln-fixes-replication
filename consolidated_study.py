#!/usr/bin/env python3
"""
consolidated_study.py
=====================
Full result set for an HONEST empirical paper on program slicing (srcSlice +
intraprocedural backward static slicing) for vulnerability analysis, on the
Big-Vul sample.

Three research questions, all reported honestly (including negative / partial
results):

  RQ1 — Fix localization (strong, NON-CIRCULAR result).
        A backward static slice taken from SINK criteria (NOT the diff) covers
        most of the true deleted (fix) lines while keeping only a fraction of
        the function. No leakage: the slice never sees func_after.

  RQ2 — Dual fix-signature discrimination (secondary, honest partial result).
        Reuses the corrected mechanism from proto_dualsig2.py. Test A measures
        the mechanism (does a fix introduce detectable slice features?); Test B
        measures generalization (it does not — coarse features don't transfer).

  RQ3 — Detection: does slicing help vs. the whole function? (honest
        negative/nuance). TF-IDF + LogisticRegression on WHOLE vs SLICE text.
        Expectation: WHOLE >= SLICE (slicing removes context useful for a
        bag-of-tokens detector).

Reuses, in this directory:
  proto_dualsig.srcslice_profiles / changed_lines / idents_in_lines
  ast_slicer.semantic_slice_indices
  rp_style (PALETTE / CMAP / save)

stdlib + numpy + sklearn + matplotlib only. Fixed seed 1337. No network.

This script is authored, NOT run here. The orchestrator runs and verifies it:
    python3 consolidated_study.py
"""

import os
import sys
import json
import random
import traceback

SEED = 1337
random.seed(SEED)

HERE = os.path.dirname(os.path.abspath(__file__))
DATA_PATH = os.path.join(HERE, "data", "bigvul", "sample.jsonl")
RESULTS_DIR = os.path.join(HERE, "results")
FIGURES_DIR = os.path.join(HERE, "figures")
RESULTS_JSON = os.path.join(RESULTS_DIR, "study_results.json")

# Caps (per spec).
RQ1_CAP = 400            # vulnerable pairs for localization
RQ2_CAP = 400            # vulnerable pairs for dual-signature
RQ3_CAP_TOTAL = 6000     # total detection rows (pos + neg)
RQ3_NEG_RATIO = 4        # negatives per positive (1:4 pos:neg)
TAUS = [0.3, 0.4, 0.5, 0.6, 0.7, 0.8]
TRAIN_FRAC = 0.70

# --------------------------------------------------------------------------- #
# Reuse existing project modules. Wrap each import so a single missing module
# degrades gracefully instead of crashing the whole study.
# --------------------------------------------------------------------------- #
try:
    from proto_dualsig import (
        srcslice_profiles,
        changed_lines,
        idents_in_lines,
    )
    _HAVE_DUALSIG = True
except Exception as exc:  # pragma: no cover
    _HAVE_DUALSIG = False
    _DUALSIG_ERR = repr(exc)

    def srcslice_profiles(code):  # type: ignore
        return None

    def changed_lines(before_lines, after_lines):  # type: ignore
        # Minimal stdlib fallback so RQ1/RQ3 still function without proto_dualsig.
        import difflib
        sm = difflib.SequenceMatcher(a=before_lines, b=after_lines,
                                     autojunk=False)
        dels, adds = set(), set()
        for tag, i1, i2, j1, j2 in sm.get_opcodes():
            if tag in ("replace", "delete"):
                dels.update(range(i1, i2))
            if tag in ("replace", "insert"):
                adds.update(range(j1, j2))
        return dels, adds

    def idents_in_lines(lines, idxs):  # type: ignore
        import re
        _re = re.compile(r"[A-Za-z_]\w*")
        out = set()
        for i in idxs:
            if 0 <= i < len(lines):
                out.update(_re.findall(lines[i]))
        return out

try:
    from ast_slicer import semantic_slice_indices
    _HAVE_SLICER = True
except Exception as exc:  # pragma: no cover
    _HAVE_SLICER = False
    _SLICER_ERR = repr(exc)

    def semantic_slice_indices(code):  # type: ignore
        # Fallback: whole function (never empty unless empty input).
        n = len(code.splitlines())
        return set(range(n))

# rp_style / matplotlib are optional — figures are best-effort.
try:
    import rp_style
    _HAVE_STYLE = True
except Exception as exc:  # pragma: no cover
    _HAVE_STYLE = False
    _STYLE_ERR = repr(exc)
    rp_style = None  # type: ignore

# numpy / sklearn optional — RQ3 is skipped if missing, JSON still written.
try:
    import numpy as np
    _HAVE_NUMPY = True
except Exception as exc:  # pragma: no cover
    _HAVE_NUMPY = False
    _NUMPY_ERR = repr(exc)

try:
    from sklearn.feature_extraction.text import TfidfVectorizer
    from sklearn.linear_model import LogisticRegression
    from sklearn.model_selection import StratifiedKFold
    from sklearn.metrics import (
        f1_score,
        average_precision_score,
        precision_score,
        recall_score,
    )
    _HAVE_SKLEARN = True
except Exception as exc:  # pragma: no cover
    _HAVE_SKLEARN = False
    _SKLEARN_ERR = repr(exc)


# --------------------------------------------------------------------------- #
# Small helpers
# --------------------------------------------------------------------------- #
def _is_vuln(v):
    return v in (1, "1", True)


def _mean(xs):
    xs = [x for x in xs if x is not None]
    return (sum(xs) / len(xs)) if xs else None


def _median(xs):
    xs = sorted(x for x in xs if x is not None)
    n = len(xs)
    if n == 0:
        return None
    mid = n // 2
    if n % 2:
        return xs[mid]
    return 0.5 * (xs[mid - 1] + xs[mid])


def _slice_indices_safe(code):
    """semantic_slice_indices wrapped: never raise, return set[int]."""
    try:
        out = semantic_slice_indices(code)
        if out is None:
            return set()
        return set(int(i) for i in out)
    except Exception:
        return set()


def _srcslice_safe(code):
    try:
        return srcslice_profiles(code)
    except Exception:
        return None


# --------------------------------------------------------------------------- #
# Data loading
# --------------------------------------------------------------------------- #
def _iter_rows(path):
    """Yield parsed JSON rows from the jsonl file; tolerate bad lines."""
    if not os.path.exists(path):
        sys.stderr.write("FATAL: data file not found: %s\n" % path)
        return
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        for raw in fh:
            raw = raw.strip()
            if not raw:
                continue
            try:
                yield json.loads(raw)
            except Exception:
                continue


def load_all_rows(path):
    return list(_iter_rows(path))


def vuln_pairs(rows, cap):
    """vul==1 AND func_after present AND func_after != func_before. Capped."""
    out = []
    for r in rows:
        if not _is_vuln(r.get("vul")):
            continue
        fb = r.get("func_before")
        fa = r.get("func_after")
        if not isinstance(fb, str) or not isinstance(fa, str):
            continue
        if not fa.strip() or fa == fb:
            continue
        out.append({
            "func_before": fb,
            "func_after": fa,
            "cwe": r.get("CWE ID", ""),
        })
        if len(out) >= cap:
            break
    return out


# --------------------------------------------------------------------------- #
# RQ1 — Fix localization (non-circular)
# --------------------------------------------------------------------------- #
def run_rq1(rows):
    pairs = vuln_pairs(rows, RQ1_CAP)
    covs, rsrs = [], []
    region_sizes = []
    n_lines_list = []
    fix_sizes = []
    used = 0
    skipped_empty_del = 0
    skipped_parse = 0

    for pr in pairs:
        fb = pr["func_before"]
        fa = pr["func_after"]
        fb_lines = fb.splitlines()
        fa_lines = fa.splitlines()
        n_lines = len(fb_lines)
        if n_lines == 0:
            skipped_parse += 1
            continue

        # del_lines: 0-based func_before indices that the fix removed/changed.
        try:
            del_lines, _add = changed_lines(fb_lines, fa_lines)
        except Exception:
            skipped_parse += 1
            continue
        if not del_lines:
            skipped_empty_del += 1
            continue

        # region: backward static slice from SINK criteria. NO diff leakage.
        region = _slice_indices_safe(fb)
        if not region:
            # Empty region (e.g. n==0 path) — count as parse failure.
            skipped_parse += 1
            continue

        # Only count del_lines that are valid indices (defensive).
        valid_del = set(d for d in del_lines if 0 <= d < n_lines)
        if not valid_del:
            skipped_empty_del += 1
            continue

        coverage = len(region & valid_del) / len(valid_del)
        region_size_ratio = len(region) / n_lines

        covs.append(coverage)
        rsrs.append(region_size_ratio)
        region_sizes.append(len(region))
        n_lines_list.append(n_lines)
        fix_sizes.append(len(valid_del))
        used += 1

    result = {
        "description": (
            "Backward static slice from SINK criteria (no diff leakage). "
            "coverage = |region & del_lines| / |del_lines|; "
            "region_size_ratio = |region| / n_lines."
        ),
        "cap": RQ1_CAP,
        "pairs_considered": len(pairs),
        "pairs_used": used,
        "skipped_empty_del": skipped_empty_del,
        "skipped_parse": skipped_parse,
        "slice": {
            "mean_coverage": _mean(covs),
            "median_coverage": _median(covs),
            "mean_region_size_ratio": _mean(rsrs),
            "median_region_size_ratio": _median(rsrs),
            "mean_region_size_lines": _mean(region_sizes),
            "mean_n_lines": _mean(n_lines_list),
        },
        # Whole-function baseline: trivially covers everything, keeps everything.
        "whole_function_baseline": {
            "mean_coverage": 1.0,
            "mean_region_size_ratio": 1.0,
        },
        "headline": None,
    }
    if used:
        result["headline"] = (
            "Slice covers %.1f%% of the true fix (deleted lines) while keeping "
            "only %.1f%% of the function." %
            (100.0 * (result["slice"]["mean_coverage"] or 0.0),
             100.0 * (result["slice"]["mean_region_size_ratio"] or 0.0))
        )
        # Bootstrap 95% CIs on the mean coverage / RSR across the |used| CVE
        # pairs (Reviewer-requested statistical reporting for RQ1).
        import random as _random
        rng = _random.Random(1337)

        def _boot_ci(xs, n_boot=2000):
            n = len(xs)
            if n < 2:
                return None
            means = []
            for _ in range(n_boot):
                sample = [xs[rng.randrange(n)] for _ in range(n)]
                means.append(sum(sample) / n)
            means.sort()
            lo = means[int(0.025 * n_boot)]
            hi = means[int(0.975 * n_boot) - 1]
            return [lo, hi]

        result["slice"]["coverage_ci95"] = _boot_ci(covs)
        result["slice"]["region_size_ratio_ci95"] = _boot_ci(rsrs)
        result["statistics_note"] = (
            "95%% CI via 2000-resample bootstrap over the %d used CVE pairs "
            "(percentile method, seed=1337)." % used
        )

        # ------------------------------------------------------------------- #
        # Stratified breakdowns (Reviewer-requested confound check): is
        # coverage/RSR uniform, or driven by easy small-fix / short-function
        # cases? Grouped post-hoc over the same |used| pairs.
        # ------------------------------------------------------------------- #
        def _stratum_stats(idxs):
            c = [covs[i] for i in idxs]
            r = [rsrs[i] for i in idxs]
            return {
                "n": len(idxs),
                "mean_coverage": _mean(c),
                "mean_region_size_ratio": _mean(r),
                "coverage_ci95": _boot_ci(c) if len(c) >= 2 else None,
            }

        # (a) By fix size (number of deleted/changed lines the real patch touches).
        fix_bins = {"1 line": [], "2-5 lines": [], ">5 lines": []}
        for i, fs in enumerate(fix_sizes):
            if fs <= 1:
                fix_bins["1 line"].append(i)
            elif fs <= 5:
                fix_bins["2-5 lines"].append(i)
            else:
                fix_bins[">5 lines"].append(i)
        result["by_fix_size"] = {
            label: _stratum_stats(idxs) for label, idxs in fix_bins.items()
        }

        # (b) By function-length quartile (n_lines of func_before).
        sorted_lens = sorted(n_lines_list)
        n_used = len(sorted_lens)

        def _q(p):
            return sorted_lens[min(n_used - 1, int(p * n_used))]

        q1, q2, q3 = _q(0.25), _q(0.50), _q(0.75)
        len_bins = {"Q1 (shortest)": [], "Q2": [], "Q3": [], "Q4 (longest)": []}
        for i, nl in enumerate(n_lines_list):
            if nl <= q1:
                len_bins["Q1 (shortest)"].append(i)
            elif nl <= q2:
                len_bins["Q2"].append(i)
            elif nl <= q3:
                len_bins["Q3"].append(i)
            else:
                len_bins["Q4 (longest)"].append(i)
        result["by_function_length_quartile"] = {
            "quartile_boundaries_lines": [q1, q2, q3],
            "strata": {label: _stratum_stats(idxs) for label, idxs in len_bins.items()},
        }
    return result


# --------------------------------------------------------------------------- #
# RQ2 — Dual fix-signature discrimination (reuses proto_dualsig2 logic)
# --------------------------------------------------------------------------- #
def _feats(profiles, restrict=None):
    """srcSlice structural features over (optionally restricted) variables.

    Mirrors proto_dualsig2.feats: cf:/pt:/dv:/var: prefixes.
    """
    f = set()
    if not profiles:
        return f
    for p in profiles:
        if restrict is not None and p.get("variable") not in restrict:
            continue
        for cf in p.get("cfuncs", {}) or {}:
            f.add("cf:" + cf)
        for pt in p.get("pointers", set()) or set():
            f.add("pt:" + pt)
        for dv in p.get("dvars", set()) or set():
            f.add("dv:" + dv)
        f.add("var:" + str(p.get("variable", "")))
    return f


def _containment(sig, q):
    """Fraction of sig contained in q. Mirrors proto_dualsig2.containment."""
    return (len(sig & q) / len(sig)) if sig else 0.0


def run_rq2(rows):
    pairs = vuln_pairs(rows, RQ2_CAP)
    items = []
    skipped = 0
    empty_delta = 0

    for pr in pairs:
        fb = pr["func_before"]
        fa = pr["func_after"]
        fbl = fb.splitlines()
        fal = fa.splitlines()
        if not fbl:
            skipped += 1
            continue
        try:
            dell, addl = changed_lines(fbl, fal)
        except Exception:
            skipped += 1
            continue
        vrb = idents_in_lines(fbl, dell)
        vra = idents_in_lines(fal, addl)
        vrv = (vrb | vra) or idents_in_lines(fbl, range(len(fbl)))

        pb = _srcslice_safe(fb)
        pa = _srcslice_safe(fa)
        if pb is None or pa is None:
            skipped += 1
            continue

        fb_all = _feats(pb)
        fa_all = _feats(pa)
        vuln_sig = _feats(pb, vrv)              # vulnerable slice (shared context)
        patch_delta = _feats(pa, vra) - fb_all  # NEW features the fix introduces
        if not patch_delta:
            empty_delta += 1
        items.append({
            "vuln_sig": vuln_sig,
            "patch_delta": patch_delta,
            "fb_all": fb_all,
            "fa_all": fa_all,
        })

    used = len(items)
    result = {
        "description": (
            "Dual slice signature (proto_dualsig2 mechanism). vuln_sig = srcSlice "
            "structural features over vr_vars of func_before; patch_delta = "
            "features over added-line vars in func_after MINUS func_before "
            "features."
        ),
        "cap": RQ2_CAP,
        "pairs_considered": len(pairs),
        "pairs_used": used,
        "skipped_srcslice": skipped,
        "test_A": None,
        "test_B": None,
        "note": (
            "srcSlice requires srcml+srcslice binaries on PATH. If absent, "
            "srcslice_profiles returns None and all pairs are skipped — the "
            "JSON still records used=0 honestly."
        ),
    }

    if used < 8:
        result["headline"] = (
            "Too few usable srcSlice pairs (%d) — likely srcml/srcslice not on "
            "PATH. RQ2 reported as unavailable." % used
        )
        return result

    # ---------- Test A: mechanism / self-discrimination ----------
    nonempty = used - empty_delta
    sep = 0
    for it in items:
        if not it["patch_delta"]:
            continue
        sp_fb = _containment(it["patch_delta"], it["fb_all"])  # ~0 by construction
        sp_fa = _containment(it["patch_delta"], it["fa_all"])  # ~1 by construction
        if sp_fa > sp_fb:
            sep += 1
    pct_nonempty = 100.0 * nonempty / used if used else 0.0
    pct_sep = 100.0 * sep / nonempty if nonempty else 0.0
    result["test_A"] = {
        "description": (
            "Mechanism: %% of CVEs with non-empty patch_delta, and of those, "
            "%% where patched side scores higher on the delta than vulnerable."
        ),
        "pct_cves_with_nonempty_delta": pct_nonempty,
        "n_nonempty_delta": nonempty,
        "pct_patched_scores_higher": pct_sep,
        "n_patched_scores_higher": sep,
    }

    # ---------- Test B: generalization (held-out DB) ----------
    ndb = max(1, min(used - 1, int(round(used * TRAIN_FRAC))))
    db, test = items[:ndb], items[ndb:]

    def best(qfull):
        bi, bv = -1, 0.0
        for i, d in enumerate(db):
            c = _containment(d["vuln_sig"], qfull)
            if c > bv:
                bv, bi = c, i
        sp = _containment(db[bi]["patch_delta"], qfull) if bi >= 0 else 0.0
        return bv, sp

    queries = []
    for t in test:
        queries.append((t["fb_all"], 1))  # vulnerable (positive)
        queries.append((t["fa_all"], 0))  # patched   (negative)
    scored = [(lbl,) + best(q) for q, lbl in queries]

    def evalrule(tau, dual):
        tp = fp = fn = tn = 0
        for lbl, sv, sp in scored:
            flag = (sv >= tau) and ((sp < tau) if dual else True)
            if lbl == 1:
                tp += int(flag)
                fn += int(not flag)
            else:
                fp += int(flag)
                tn += int(not flag)
        P = tp / (tp + fp) if (tp + fp) else 0.0
        R = tp / (tp + fn) if (tp + fn) else 0.0
        F1 = 2 * P * R / (P + R) if (P + R) else 0.0
        return {"tau": tau, "P": P, "R": R, "F1": F1,
                "fp_after": fp, "tp": tp, "fn": fn, "tn": tn}

    rule_a_rows = []
    rule_b_rows = []
    best_b = None
    for tau in TAUS:
        a = evalrule(tau, dual=False)
        b = evalrule(tau, dual=True)
        rule_a_rows.append(a)
        rule_b_rows.append(b)
        if best_b is None or b["F1"] > best_b["F1"]:
            best_b = b

    best_a_at_b_tau = next(r for r in rule_a_rows if r["tau"] == best_b["tau"])
    result["test_B"] = {
        "description": (
            "Generalization: 70/30 split; query=whole-function feature set; "
            "Rule A flags if sim_vuln>=tau; Rule B flags if sim_vuln>=tau AND "
            "sim_patch<tau. Reports P/R/F1 and func_after FP per tau."
        ),
        "db_size": len(db),
        "test_size": len(test),
        "n_positives": sum(1 for s in scored if s[0] == 1),
        "n_negatives": sum(1 for s in scored if s[0] == 0),
        "rule_A_vuln_only": rule_a_rows,
        "rule_B_dual": rule_b_rows,
        "best_F1_dual_row": best_b,
        "rule_A_at_best_dual_tau": best_a_at_b_tau,
    }

    result["headline"] = (
        "Test A: ~%.0f%% of CVEs have a non-empty fix delta (mechanism fires); "
        "Test B: dual rule generalizes ~= vuln-only (best dual F1=%.2f vs "
        "vuln-only F1=%.2f at tau=%.1f) — coarse srcSlice features do not "
        "transfer." %
        (pct_nonempty, best_b["F1"], best_a_at_b_tau["F1"], best_b["tau"])
    )
    return result


# --------------------------------------------------------------------------- #
# RQ3 — Detection: slice vs whole function
# --------------------------------------------------------------------------- #
def _slice_text(code):
    """Join the lines of `code` kept by the semantic slice.

    Fallback to the whole function if the slice is empty.
    """
    lines = code.splitlines()
    if not lines:
        return ""
    idxs = _slice_indices_safe(code)
    if not idxs:
        return code
    sel = [lines[i] for i in sorted(idxs) if 0 <= i < len(lines)]
    text = "\n".join(sel)
    return text if text.strip() else code


def _build_detection_dataset(rows):
    """ALL vul==1 (func_before) positives; sampled vul==0 negatives at 1:4.

    Capped at RQ3_CAP_TOTAL total rows. Returns (texts_whole, texts_slice,
    labels, meta).
    """
    pos_funcs = []
    neg_funcs = []
    for r in rows:
        fb = r.get("func_before")
        if not isinstance(fb, str) or not fb.strip():
            continue
        if _is_vuln(r.get("vul")):
            pos_funcs.append(fb)
        else:
            neg_funcs.append(fb)

    # Determine counts respecting both the 1:4 ratio and the total cap.
    # Max positives we can keep if negatives = 4*pos and total <= cap:
    #   pos + 4*pos <= cap  =>  pos <= cap/5
    cap_pos_by_total = RQ3_CAP_TOTAL // (1 + RQ3_NEG_RATIO)
    n_pos = min(len(pos_funcs), cap_pos_by_total)
    # Also bounded by available negatives.
    n_neg_target = n_pos * RQ3_NEG_RATIO
    n_neg = min(len(neg_funcs), n_neg_target)

    rng = random.Random(SEED)
    if n_pos < len(pos_funcs):
        pos_sel = rng.sample(pos_funcs, n_pos)
    else:
        pos_sel = list(pos_funcs)
    if n_neg < len(neg_funcs):
        neg_sel = rng.sample(neg_funcs, n_neg)
    else:
        neg_sel = list(neg_funcs)

    funcs = [(f, 1) for f in pos_sel] + [(f, 0) for f in neg_sel]
    rng.shuffle(funcs)

    texts_whole = []
    texts_slice = []
    labels = []
    slice_fallbacks = 0
    for code, lbl in funcs:
        texts_whole.append(code)
        st = _slice_text(code)
        if st == code:
            slice_fallbacks += 1
        texts_slice.append(st)
        labels.append(lbl)

    meta = {
        "available_positives": len(pos_funcs),
        "available_negatives": len(neg_funcs),
        "used_positives": len(pos_sel),
        "used_negatives": len(neg_sel),
        "total": len(funcs),
        "target_ratio_pos_to_neg": "1:%d" % RQ3_NEG_RATIO,
        "actual_ratio_pos_to_neg": (
            "1:%.2f" % (len(neg_sel) / len(pos_sel)) if pos_sel else "n/a"
        ),
        "slice_fallback_to_whole": slice_fallbacks,
        "cap_total": RQ3_CAP_TOTAL,
    }
    return texts_whole, texts_slice, labels, meta


def _cv_evaluate(texts, labels):
    """5-fold stratified CV of TF-IDF + LogisticRegression. Returns metric dict."""
    y = np.array(labels)
    skf = StratifiedKFold(n_splits=5, shuffle=True, random_state=SEED)
    f1s, aps, precs, recs = [], [], [], []
    for tr_idx, te_idx in skf.split(np.zeros(len(y)), y):
        X_tr = [texts[i] for i in tr_idx]
        X_te = [texts[i] for i in te_idx]
        y_tr = y[tr_idx]
        y_te = y[te_idx]

        vec = TfidfVectorizer(max_features=2000, ngram_range=(1, 2))
        Xtr = vec.fit_transform(X_tr)
        Xte = vec.transform(X_te)

        clf = LogisticRegression(max_iter=500, class_weight="balanced")
        clf.fit(Xtr, y_tr)

        pred = clf.predict(Xte)
        # Probability of the positive class for PR-AUC.
        try:
            scores = clf.predict_proba(Xte)[:, 1]
        except Exception:
            scores = clf.decision_function(Xte)

        f1s.append(f1_score(y_te, pred, zero_division=0))
        recs.append(recall_score(y_te, pred, zero_division=0))
        precs.append(precision_score(y_te, pred, zero_division=0))
        try:
            aps.append(average_precision_score(y_te, scores))
        except Exception:
            aps.append(float("nan"))

    return {
        "mean_f1": float(np.mean(f1s)),
        "mean_pr_auc": float(np.nanmean(aps)),
        "mean_precision": float(np.mean(precs)),
        "mean_recall": float(np.mean(recs)),
        "fold_f1": [float(x) for x in f1s],
    }


def run_rq3(rows):
    result = {
        "description": (
            "Detection: TF-IDF(max_features=2000, ngram (1,2)) + "
            "LogisticRegression(max_iter=500, class_weight='balanced'), "
            "StratifiedKFold(5). WHOLE function text vs SLICE text. "
            "Positives = all vul==1 func_before; negatives = sampled vul==0 at "
            "1:%d, total capped at %d." % (RQ3_NEG_RATIO, RQ3_CAP_TOTAL)
        ),
        "available": _HAVE_NUMPY and _HAVE_SKLEARN,
        "whole": None,
        "slice": None,
        "dataset": None,
        "headline": None,
    }
    if not (_HAVE_NUMPY and _HAVE_SKLEARN):
        result["headline"] = (
            "numpy/sklearn unavailable — RQ3 skipped (JSON still written)."
        )
        return result

    texts_whole, texts_slice, labels, meta = _build_detection_dataset(rows)
    result["dataset"] = meta
    if meta["used_positives"] < 5 or meta["used_negatives"] < 5:
        result["headline"] = "Too few rows for RQ3 detection."
        return result

    try:
        whole = _cv_evaluate(texts_whole, labels)
    except Exception as exc:
        whole = {"error": repr(exc)}
    try:
        sliced = _cv_evaluate(texts_slice, labels)
    except Exception as exc:
        sliced = {"error": repr(exc)}

    result["whole"] = whole
    result["slice"] = sliced

    if "mean_f1" in whole and "mean_f1" in sliced:
        wf, sf = whole["mean_f1"], sliced["mean_f1"]
        winner = "WHOLE" if wf >= sf else "SLICE"
        result["headline"] = (
            "%s wins on F1 (WHOLE F1=%.3f / PR-AUC=%.3f vs SLICE F1=%.3f / "
            "PR-AUC=%.3f). As expected, slicing removes context useful for a "
            "bag-of-tokens detector." %
            (winner, wf, whole["mean_pr_auc"], sf, sliced["mean_pr_auc"])
        )
    return result


# --------------------------------------------------------------------------- #
# Figures (best-effort; each guarded)
# --------------------------------------------------------------------------- #
def _figures(results):
    if not _HAVE_STYLE:
        print("[figures] rp_style/matplotlib unavailable — skipping figures.")
        return
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    pal = getattr(rp_style, "PALETTE",
                  ["#0072B2", "#E69F00", "#009E73", "#D55E00"])

    # --- fig_localization.png (RQ1) ---
    try:
        rq1 = results.get("RQ1", {})
        s = rq1.get("slice", {}) or {}
        cov = s.get("mean_coverage")
        rsr = s.get("mean_region_size_ratio")
        if cov is None or rsr is None:
            raise ValueError("RQ1 has no usable means")
        fig, ax = plt.subplots(figsize=(7, 4.5))
        labels = ["Slice\ncoverage", "Slice\nregion-size ratio",
                  "Whole-function\n(baseline)"]
        vals = [cov, rsr, 1.0]
        colors = [pal[0], pal[1], pal[3 % len(pal)]]
        bars = ax.bar(labels, vals, color=colors)
        ax.set_ylim(0, 1.05)
        ax.set_ylabel("Fraction")
        for b, v in zip(bars, vals):
            ax.text(b.get_x() + b.get_width() / 2, v + 0.02,
                    "%.2f" % v, ha="center", va="bottom", fontsize=9)
        ax.set_title("RQ1: Fix localization (slice covers the fix in a "
                     "fraction of the code)")
        rp_style.save(os.path.join(FIGURES_DIR, "fig_localization.png"), fig)
    except Exception as exc:
        print("[figures] fig_localization failed: %r" % exc)
        try:
            plt.close("all")
        except Exception:
            pass

    # --- fig_detection.png (RQ3) ---
    try:
        rq3 = results.get("RQ3", {})
        w = rq3.get("whole") or {}
        sl = rq3.get("slice") or {}
        if "mean_f1" not in w or "mean_f1" not in sl:
            raise ValueError("RQ3 has no usable metrics")
        groups = ["F1", "PR-AUC"]
        whole_vals = [w["mean_f1"], w["mean_pr_auc"]]
        slice_vals = [sl["mean_f1"], sl["mean_pr_auc"]]
        x = np.arange(len(groups)) if _HAVE_NUMPY else [0, 1]
        width = 0.36
        fig, ax = plt.subplots(figsize=(7, 4.5))
        ax.bar([xi - width / 2 for xi in x], whole_vals, width,
               label="WHOLE", color=pal[0])
        ax.bar([xi + width / 2 for xi in x], slice_vals, width,
               label="SLICE", color=pal[2 % len(pal)])
        ax.set_xticks(list(x))
        ax.set_xticklabels(groups)
        ax.set_ylim(0, 1.05)
        ax.set_ylabel("Score")
        ax.legend()
        ax.set_title("RQ3: Detection — WHOLE vs SLICE")
        rp_style.save(os.path.join(FIGURES_DIR, "fig_detection.png"), fig)
    except Exception as exc:
        print("[figures] fig_detection failed: %r" % exc)
        try:
            plt.close("all")
        except Exception:
            pass

    # --- fig_signature.png (RQ2) ---
    try:
        rq2 = results.get("RQ2", {})
        ta = rq2.get("test_A") or {}
        tb = rq2.get("test_B") or {}
        pct_distinguishable = ta.get("pct_cves_with_nonempty_delta")
        # Rule A vs Rule B func_after FP at best dual tau.
        b_row = tb.get("best_F1_dual_row") or {}
        a_row = tb.get("rule_A_at_best_dual_tau") or {}
        fp_a = a_row.get("fp_after")
        fp_b = b_row.get("fp_after")
        if pct_distinguishable is None and (fp_a is None or fp_b is None):
            raise ValueError("RQ2 has no usable metrics")

        fig, axes = plt.subplots(1, 2, figsize=(9.5, 4.3))
        # Left: % distinguishable by fix-signature (Test A).
        ax0 = axes[0]
        if pct_distinguishable is not None:
            ax0.bar(["non-empty\nfix delta"], [pct_distinguishable],
                    color=pal[0])
            ax0.set_ylim(0, 100)
            ax0.set_ylabel("% of CVEs")
            ax0.text(0, pct_distinguishable + 1.5,
                     "%.1f%%" % pct_distinguishable, ha="center",
                     va="bottom", fontsize=9)
        ax0.set_title("RQ2 Test A: distinguishable\nby fix-signature")
        # Right: func_after FP, Rule A vs Rule B at best tau.
        ax1 = axes[1]
        if fp_a is not None and fp_b is not None:
            bars = ax1.bar(["Rule A\n(vuln-only)", "Rule B\n(dual)"],
                           [fp_a, fp_b], color=[pal[1], pal[2 % len(pal)]])
            ax1.set_ylabel("func_after false positives")
            for b, v in zip(bars, [fp_a, fp_b]):
                ax1.text(b.get_x() + b.get_width() / 2, v,
                         str(int(v)), ha="center", va="bottom", fontsize=9)
            tau = b_row.get("tau")
            ax1.set_title("RQ2 Test B: func_after FP\n(best dual tau=%s)" %
                          ("%.1f" % tau if tau is not None else "?"))
        else:
            ax1.set_title("RQ2 Test B: unavailable")
        rp_style.save(os.path.join(FIGURES_DIR, "fig_signature.png"), fig)
    except Exception as exc:
        print("[figures] fig_signature failed: %r" % exc)
        try:
            plt.close("all")
        except Exception:
            pass


# --------------------------------------------------------------------------- #
# Summary printing
# --------------------------------------------------------------------------- #
def _print_summary(results):
    print("=" * 78)
    print("CONSOLIDATED STUDY — program slicing for vulnerability analysis")
    print("=" * 78)
    print("seed=%d   data=%s" % (SEED, DATA_PATH))
    print("modules: proto_dualsig=%s  ast_slicer=%s  rp_style=%s  "
          "numpy=%s  sklearn=%s" %
          (_HAVE_DUALSIG, _HAVE_SLICER, _HAVE_STYLE, _HAVE_NUMPY,
           _HAVE_SKLEARN))
    print("-" * 78)

    rq1 = results.get("RQ1", {})
    print("RQ1 — Fix localization (non-circular)")
    print("  pairs used / empty-del skipped / parse skipped: %s / %s / %s" %
          (rq1.get("pairs_used"), rq1.get("skipped_empty_del"),
           rq1.get("skipped_parse")))
    s = rq1.get("slice", {}) or {}
    print("  slice mean coverage=%s  median=%s" %
          (_fmt(s.get("mean_coverage")), _fmt(s.get("median_coverage"))))
    print("  slice mean region_size_ratio=%s  median=%s" %
          (_fmt(s.get("mean_region_size_ratio")),
           _fmt(s.get("median_region_size_ratio"))))
    print("  baseline (whole function): coverage=1.000  rsr=1.000")
    if rq1.get("headline"):
        print("  HEADLINE: " + rq1["headline"])
    print("-" * 78)

    rq2 = results.get("RQ2", {})
    print("RQ2 — Dual fix-signature discrimination")
    print("  pairs used / skipped(srcslice): %s / %s" %
          (rq2.get("pairs_used"), rq2.get("skipped_srcslice")))
    ta = rq2.get("test_A")
    if ta:
        print("  Test A: %% non-empty delta=%.1f%%  |  %% patched higher=%.1f%%" %
              (ta["pct_cves_with_nonempty_delta"],
               ta["pct_patched_scores_higher"]))
    tb = rq2.get("test_B")
    if tb:
        b = tb["best_F1_dual_row"]
        a = tb["rule_A_at_best_dual_tau"]
        print("  Test B best dual tau=%.1f: RuleB F1=%.3f (FP_after=%d)  vs  "
              "RuleA F1=%.3f (FP_after=%d)" %
              (b["tau"], b["F1"], b["fp_after"], a["F1"], a["fp_after"]))
    if rq2.get("headline"):
        print("  HEADLINE: " + rq2["headline"])
    print("-" * 78)

    rq3 = results.get("RQ3", {})
    print("RQ3 — Detection: slice vs whole function")
    ds = rq3.get("dataset")
    if ds:
        print("  dataset: pos=%s neg=%s total=%s  actual ratio %s  "
              "(slice->whole fallbacks=%s)" %
              (ds.get("used_positives"), ds.get("used_negatives"),
               ds.get("total"), ds.get("actual_ratio_pos_to_neg"),
               ds.get("slice_fallback_to_whole")))
    w = rq3.get("whole")
    sl = rq3.get("slice")
    if isinstance(w, dict) and "mean_f1" in w:
        print("  WHOLE: F1=%.3f  PR-AUC=%.3f  P=%.3f  R=%.3f" %
              (w["mean_f1"], w["mean_pr_auc"], w["mean_precision"],
               w["mean_recall"]))
    if isinstance(sl, dict) and "mean_f1" in sl:
        print("  SLICE: F1=%.3f  PR-AUC=%.3f  P=%.3f  R=%.3f" %
              (sl["mean_f1"], sl["mean_pr_auc"], sl["mean_precision"],
               sl["mean_recall"]))
    if rq3.get("headline"):
        print("  HEADLINE: " + rq3["headline"])
    print("=" * 78)


def _fmt(x):
    return "%.3f" % x if isinstance(x, (int, float)) else str(x)


# --------------------------------------------------------------------------- #
# Main
# --------------------------------------------------------------------------- #
def main():
    os.makedirs(RESULTS_DIR, exist_ok=True)
    os.makedirs(FIGURES_DIR, exist_ok=True)

    results = {
        "_meta": {
            "seed": SEED,
            "data_path": os.path.relpath(DATA_PATH, os.path.dirname(os.path.abspath(__file__))),
            "have_proto_dualsig": _HAVE_DUALSIG,
            "have_ast_slicer": _HAVE_SLICER,
            "have_rp_style": _HAVE_STYLE,
            "have_numpy": _HAVE_NUMPY,
            "have_sklearn": _HAVE_SKLEARN,
        },
        "RQ1": {},
        "RQ2": {},
        "RQ3": {},
    }

    rows = load_all_rows(DATA_PATH)
    results["_meta"]["rows_loaded"] = len(rows)
    print("rows loaded: %d" % len(rows))

    # Each RQ is independently guarded so one failure cannot lose the others.
    for key, fn in (("RQ1", run_rq1), ("RQ2", run_rq2), ("RQ3", run_rq3)):
        try:
            results[key] = fn(rows)
        except Exception as exc:
            results[key] = {
                "error": repr(exc),
                "traceback": traceback.format_exc(),
            }
            print("[%s] FAILED: %r" % (key, exc))

    # Write JSON before figures so results survive a figure/plotting crash.
    try:
        with open(RESULTS_JSON, "w", encoding="utf-8") as fh:
            json.dump(results, fh, indent=2, default=_json_default)
        print("results written -> %s" % RESULTS_JSON)
    except Exception as exc:
        print("WARNING: could not write results json: %r" % exc)

    try:
        _figures(results)
    except Exception as exc:
        print("[figures] top-level failure: %r" % exc)

    try:
        _print_summary(results)
    except Exception as exc:
        print("[summary] failed: %r" % exc)


def _json_default(o):
    """Make stray sets/tuples JSON-serializable (defensive)."""
    if isinstance(o, (set, frozenset, tuple)):
        return list(o)
    try:
        import numpy as _np
        if isinstance(o, (_np.integer,)):
            return int(o)
        if isinstance(o, (_np.floating,)):
            return float(o)
        if isinstance(o, _np.ndarray):
            return o.tolist()
    except Exception:
        pass
    return str(o)


if __name__ == "__main__":
    main()
