#!/usr/bin/env python3
"""
augmented_study.py  —  Contribution C2: srcML-augmented slice representation
(RQ2 signature discrimination, RQ3 core detection representations).

HYPOTHESIS (C2)
---------------
srcSlice produces a *coarse* per-variable profile (called-functions / pointers /
dependent-vars) that throws away the structural CONTENT of the code it summarizes.
A backward static *slice* gives the right SCOPE (which lines matter) but, when fed
to srcSlice, the resulting profile is too lossy to (RQ2) discriminate a fix from
the vulnerable code, or (RQ3) detect vulnerabilities.

C2 claim: if we keep the SLICE for SCOPE but replace the coarse srcSlice profile
with *abstract srcML structural features* over the slice (AUG_SLICE), we recover
the discriminative signal that plain slicing lost.

We compare five representations of a function F:
  WHOLE_TEXT : raw source tokens (the TF-IDF text baseline).
  SLICE_TEXT : the source lines of F kept by semantic_slice_indices(F), joined.
  AUG_WHOLE  : abstract srcML features over the whole function.
  AUG_SLICE  : abstract srcML features over the SLICE          <-- the C2 rep.
  COARSE     : srcSlice profile features (cf:/pt:/dv:/var:)    <-- reused from
               proto_dualsig2.py construction (the ~26% baseline).

Two experiments:
  RQ2-AUG : does augmentation lift fix-signature discrimination beyond ~26%?
  RQ3-AUG : does augmentation recover detection vs the WHOLE_TEXT baseline?

Self-contained. stdlib + numpy + sklearn only. Fixed seed 1337. No network.

NOTE: This script SHELLS OUT to `srcml` and (via proto_dualsig) `srcslice`. It is
NOT run in the authoring environment; the orchestrator runs and verifies it.
Run as:  python3 augmented_study.py
"""

import os
import sys
import json
import random
import tempfile
import subprocess
import xml.etree.ElementTree as ET
from collections import OrderedDict

import numpy as np
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import RepeatedStratifiedKFold
from sklearn.metrics import (
    f1_score,
    precision_score,
    recall_score,
    average_precision_score,
)

# Reused project modules (same directory).
from ast_slicer import (
    semantic_slice_indices,
    _find_function_body,
    _gather_statements,
    _is_criterion,
    _PARSER,
)
from proto_dualsig import (
    srcslice_profiles,
    changed_lines,
    idents_in_lines,
)
import re as _re

# --------------------------------------------------------------------------- #
# Constants / knobs
# --------------------------------------------------------------------------- #
SEED = 1337
random.seed(SEED)
np.random.seed(SEED)

HERE = os.path.dirname(os.path.abspath(__file__))
DATA_PATH = os.path.join(HERE, "data", "bigvul", "sample.jsonl")
RESULTS_DIR = os.path.join(HERE, "results")
RESULTS_PATH = os.path.join(RESULTS_DIR, "augmented_results.json")

RQ2_CAP = 400            # vuln pairs for RQ2
RQ2_TRAIN_FRAC = 0.70    # 70/30 split for the generalization test (Test B)
DET_NEG_RATIO = 4        # negatives : positives (1:4)
DET_CAP = 2500           # total detection-set cap (srcml is the bottleneck)
N_FOLDS = 5
N_REPEATS = 30           # repeated 5-fold CV -> 150 paired samples for RQ3 stats power

SRCML_TIMEOUT = 20       # seconds per srcml call
SRCML_BIN = "srcml"      # resolved on PATH by subprocess

# srcML namespaces.
NS_SRC = "http://www.srcML.org/srcML/src"
NS_POS = "http://www.srcML.org/srcML/position"
POS_START = "{%s}start" % NS_POS   # attribute key for pos:start

# Tags we treat as control/structure features.
CTRL_TAGS = {
    "if", "else", "for", "while", "switch", "do", "case",
    "condition", "ternary", "goto", "break", "continue", "return",
}

# Baselines reported by prior runs (printed for context only).
RQ2_COARSE_BASELINE_PCT = 26.0
RQ3_WHOLE_F1 = 0.408
RQ3_WHOLE_PRAUC = 0.365


# --------------------------------------------------------------------------- #
# srcML abstract feature extractor  (the core of C2)
# --------------------------------------------------------------------------- #
def _local(tag):
    """Strip the {namespace} prefix from an ElementTree tag -> local name."""
    if not isinstance(tag, str):
        return ""
    return tag.split("}")[-1]


def _start_line(el):
    """Read pos:start='LINE:COL' -> int LINE, or None if absent/malformed."""
    val = el.get(POS_START)
    if not val:
        return None
    try:
        return int(val.split(":")[0])
    except Exception:
        return None


def srcml_features(code):
    """
    Run `srcml --position file.c`, parse the XML, and emit a list of
    (line, feature) pairs where `line` is the pos:start line (int) of the
    element that produced the feature.

    Features are ABSTRACT (variable identifiers are symbolized away) so they
    generalize across CVEs:
      operator text   -> "op:" + text         (op:<=, op:++, op:=, ...)
      literal         -> "lit:" + type         (lit:number, lit:string, ...)
      control/struct  -> "ctrl:" + tag         (ctrl:if, ctrl:for, ...)
      <type> <name>   -> "type:" + name        (type:int, type:size_t, ...)
      <type> <modifier> -> "mod:" + text       (mod:*, mod:&)
      index present   -> "idx"
      call            -> "call"
      call function   -> "callee:" + name      (callee:memcpy, ...)
      decl_stmt       -> "decl"
    Bare <name> identifiers (variable names) are intentionally skipped.

    On any tool/parse failure returns []. Never raises.
    """
    if not code:
        return []

    cpath = None
    try:
        fd, cpath = tempfile.mkstemp(suffix=".c")
        with os.fdopen(fd, "w", encoding="utf-8", errors="replace") as fh:
            fh.write(code)
            if not code.endswith("\n"):
                fh.write("\n")
    except Exception:
        if cpath:
            try:
                os.unlink(cpath)
            except Exception:
                pass
        return []

    try:
        try:
            proc = subprocess.run(
                [SRCML_BIN, "--position", cpath],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                timeout=SRCML_TIMEOUT,
                check=False,
            )
        except Exception:
            return []
        if proc.returncode != 0 or not proc.stdout:
            return []
        xml_text = proc.stdout.decode("utf-8", "replace")
        if not xml_text.strip():
            return []

        try:
            root = ET.fromstring(xml_text)
        except Exception:
            return []

        feats = []
        for el in root.iter():
            tag = _local(el.tag)
            line = _start_line(el)
            # `line` may be None for some elements (e.g. the unit root or
            # synthesized nodes); we still emit but with a sentinel that the
            # line_filter will never match. Use -1 so feat_set(None) keeps it
            # while a positive filter drops it.
            ln = line if line is not None else -1
            text = (el.text or "").strip()

            if tag == "operator":
                if text:
                    feats.append((ln, "op:" + text))
            elif tag == "literal":
                feats.append((ln, "lit:" + (el.get("type") or "?")))
            elif tag in CTRL_TAGS:
                feats.append((ln, "ctrl:" + tag))
            elif tag == "type":
                # Children: <name> tokens (keep) and <modifier> tokens (* &).
                for child in list(el):
                    ctag = _local(child.tag)
                    ctext = (child.text or "").strip()
                    if ctag == "name" and ctext:
                        feats.append((ln, "type:" + ctext))
                    elif ctag == "modifier" and ctext:
                        feats.append((ln, "mod:" + ctext))
            elif tag == "index":
                feats.append((ln, "idx"))
            elif tag == "decl_stmt":
                feats.append((ln, "decl"))
            elif tag == "call":
                feats.append((ln, "call"))
                # The callee name is the call's <name>. It can be a compound
                # name (a<name><name>::<name></name>) so take the deepest leaf
                # name text if a flat .text is empty.
                callee = _callee_name(el)
                if callee:
                    feats.append((ln, "callee:" + callee))
            # Bare <name> identifiers and everything else are skipped.
        return feats
    finally:
        if cpath:
            try:
                os.unlink(cpath)
            except Exception:
                pass


def _callee_name(call_el):
    """
    Extract a callee identifier from a <call> element. The first child of a
    <call> is its <name> (possibly compound, e.g. obj.method or ns::fn). We
    return the last simple-name leaf text, which is the actual function name.
    """
    try:
        for child in list(call_el):
            if _local(child.tag) != "name":
                continue
            # Flat simple name: <name>memcpy</name>.
            flat = (child.text or "").strip()
            leaves = [
                (sub.text or "").strip()
                for sub in child.iter()
                if _local(sub.tag) == "name" and (sub.text or "").strip()
            ]
            if leaves:
                return leaves[-1]
            if flat:
                return flat
            return None
    except Exception:
        return None
    return None


def feat_set(code, line_filter=None):
    """
    Set of abstract srcML features.

    If line_filter is None -> all features for the function.
    Else line_filter is an iterable of 0-based line indices; we keep a feature
    iff (its pos:start line) - 1 is in the filter. Features with no position
    (sentinel line -1) are dropped when a filter is supplied.
    """
    pairs = srcml_features(code)
    if not pairs:
        return set()
    if line_filter is None:
        return {f for (_, f) in pairs}
    keep = set(line_filter)
    return {f for (ln, f) in pairs if ln >= 1 and (ln - 1) in keep}


# --------------------------------------------------------------------------- #
# COARSE srcSlice features  (reused construction from proto_dualsig2.feats)
# --------------------------------------------------------------------------- #
def coarse_feats(profiles, restrict=None):
    """Build COARSE feature set from srcslice profiles (cf:/pt:/dv:/var:)."""
    f = set()
    if not profiles:
        return f
    for p in profiles:
        try:
            if restrict is not None and p["variable"] not in restrict:
                continue
            for cf in p.get("cfuncs", []):
                f.add("cf:" + cf)
            for pt in p.get("pointers", []):
                f.add("pt:" + pt)
            for dv in p.get("dvars", []):
                f.add("dv:" + dv)
            f.add("var:" + p["variable"])
        except Exception:
            continue
    return f


def containment(sig, q):
    """Fraction of signature features contained in query set q."""
    return (len(sig & q) / len(sig)) if sig else 0.0


# --------------------------------------------------------------------------- #
# Caches keyed on raw function text (functions recur across rows).
# --------------------------------------------------------------------------- #
_slice_cache = {}        # code -> frozenset(slice indices)
_srcml_cache = {}        # code -> list[(line, feat)]
_coarse_cache = {}       # code -> list[profile] or None

_fail = {"slice": 0, "srcml": 0, "srcslice": 0}


def slice_idx(code):
    c = _slice_cache.get(code)
    if c is None:
        try:
            c = frozenset(semantic_slice_indices(code))
        except Exception:
            _fail["slice"] += 1
            c = frozenset(range(len(code.splitlines())))
        _slice_cache[code] = c
    return c


def srcml_pairs(code):
    c = _srcml_cache.get(code)
    if c is None:
        c = srcml_features(code)
        if not c:
            _fail["srcml"] += 1
        _srcml_cache[code] = c
    return c


def aug_whole(code):
    pairs = srcml_pairs(code)
    return {f for (_, f) in pairs}


def aug_slice(code):
    pairs = srcml_pairs(code)
    if not pairs:
        return set()
    keep = slice_idx(code)
    return {f for (ln, f) in pairs if ln >= 1 and (ln - 1) in keep}


def slice_text(code):
    keep = slice_idx(code)
    lines = code.splitlines()
    return "\n".join(lines[i] for i in sorted(keep) if 0 <= i < len(lines))


# --------------------------------------------------------------------------- #
# Variable-mention region (Phase-15 critique response): a lexical-heuristic
# region built from the SAME criterion-statement seed as the backward slice
# (reuses ast_slicer's criterion-finding step, exactly like
# rq1_baselines.py::variable_mention_region / slice_and_criteria) but with the
# dataflow-propagation step replaced by "any line mentioning an
# identifier that appears on a criterion line." Used as a drop-in substitute
# for slice_idx() to test whether the backward slice's specific SCOPE adds
# anything over this simpler heuristic once both get the same srcML
# augmentation (RQ2 Test A / RQ3 AUG_VARMENTION).
# --------------------------------------------------------------------------- #
_VARMENTION_IDENT_RE = _re.compile(r"[A-Za-z_]\w*")
_VARMENTION_KEYWORDS = {
    "if", "for", "while", "return", "const", "void", "int", "char",
    "unsigned", "long", "struct", "static", "else", "do", "switch", "case",
    "break", "continue", "sizeof", "NULL", "true", "false", "short", "float",
    "double", "goto", "typedef", "enum", "union", "register", "volatile",
}
_varmention_cache = {}


def _crit_lines_for(code):
    """Criterion-statement line set for `code` -- identical criterion-finding
    step ast_slicer uses to seed the backward slice (same as
    rq1_baselines.slice_and_criteria's crit_lines), reused here (not
    reimplemented) so the two regions share a seed and differ only in how
    they grow from it."""
    crit_lines = set()
    try:
        code_bytes = code.encode("utf-8", "replace")
        tree = _PARSER.parse(code_bytes)
        root = tree.root_node
        body, _func_node = _find_function_body(root)
        if body is not None:
            stmts, _ = _gather_statements(body, code_bytes)
            crit_stmts = [s for s in stmts if _is_criterion(s.node, code_bytes)]
            for s in crit_stmts:
                crit_lines.update(range(s.node.start_point[0], s.node.end_point[0] + 1))
    except Exception:
        pass
    return crit_lines


def _variable_mention_region(code):
    fb_lines = code.splitlines()
    crit_lines = _crit_lines_for(code)
    vc = set()
    for li in crit_lines:
        if 0 <= li < len(fb_lines):
            vc.update(_VARMENTION_IDENT_RE.findall(fb_lines[li]))
    vc = {v for v in vc if v not in _VARMENTION_KEYWORDS and len(v) > 1}
    if not vc:
        return set()
    region = set()
    for i, line in enumerate(fb_lines):
        if set(_VARMENTION_IDENT_RE.findall(line)) & vc:
            region.add(i)
    return region


def varmention_idx(code):
    c = _varmention_cache.get(code)
    if c is None:
        c = frozenset(_variable_mention_region(code))
        _varmention_cache[code] = c
    return c


def aug_varmention(code):
    pairs = srcml_pairs(code)
    if not pairs:
        return set()
    keep = varmention_idx(code)
    return {f for (ln, f) in pairs if ln >= 1 and (ln - 1) in keep}


def varmention_text(code):
    keep = varmention_idx(code)
    lines = code.splitlines()
    return "\n".join(lines[i] for i in sorted(keep) if 0 <= i < len(lines))


def coarse_profiles(code):
    if code in _coarse_cache:
        return _coarse_cache[code]
    try:
        prof = srcslice_profiles(code)
    except Exception:
        prof = None
    if prof is None:
        _fail["srcslice"] += 1
    _coarse_cache[code] = prof
    return prof


# --------------------------------------------------------------------------- #
# Data loading
# --------------------------------------------------------------------------- #
def _is_vuln(v):
    return v in (1, "1", True)


def load_rows():
    rows = []
    if not os.path.exists(DATA_PATH):
        sys.stderr.write("FATAL: data file not found: %s\n" % DATA_PATH)
        return rows
    with open(DATA_PATH, "r", encoding="utf-8", errors="replace") as fh:
        for raw in fh:
            raw = raw.strip()
            if not raw:
                continue
            try:
                rows.append(json.loads(raw))
            except Exception:
                continue
    return rows


# --------------------------------------------------------------------------- #
# RQ2-AUG
# --------------------------------------------------------------------------- #
def run_rq2(rows):
    print("=" * 78)
    print("RQ2-AUG — fix-signature discrimination: COARSE vs AUG (srcML over slice)")
    print("=" * 78)

    pairs = [
        r for r in rows
        if _is_vuln(r.get("vul"))
        and isinstance(r.get("func_before"), str)
        and isinstance(r.get("func_after"), str)
        and r["func_after"].strip()
        and r["func_after"] != r["func_before"]
    ][:RQ2_CAP]

    items = []
    skipped = 0
    for r in pairs:
        fb, fa = r["func_before"], r["func_after"]
        fbl, fal = fb.splitlines(), fa.splitlines()
        if not fbl or not fal:
            skipped += 1
            continue
        try:
            dell, addl = changed_lines(fbl, fal)
            vra = idents_in_lines(fal, addl)
        except Exception:
            skipped += 1
            continue

        # ---- COARSE arm (the ~26% baseline) ----
        pb = coarse_profiles(fb)
        pa = coarse_profiles(fa)
        if pb is None or pa is None:
            coarse_fb = coarse_fa = coarse_delta = set()
            coarse_ok = False
        else:
            coarse_fb = coarse_feats(pb)               # whole vulnerable profile
            coarse_fa_all = coarse_feats(pa)
            # COARSE delta = patched profile over added-line vars MINUS func_before.
            coarse_fa = coarse_feats(pa, vra)
            coarse_delta = coarse_fa - coarse_fb
            coarse_ok = True

        # ---- AUG arm (C2: abstract srcML over the slice) ----
        aug_fb_slice = aug_slice(fb)
        aug_fa_slice = aug_slice(fa)
        aug_delta = aug_fa_slice - aug_fb_slice        # features the fix adds in slice
        aug_fb_whole = aug_whole(fb)
        aug_fa_whole = aug_whole(fa)
        aug_delta_whole = aug_fa_whole - aug_fb_whole  # (Phase 15) whole-scope delta,
        # deconfounds scope from features: same rich srcML features as aug_delta,
        # but computed over the WHOLE FUNCTION instead of the slice.

        # (Phase 15) variable-mention-region delta: same rich srcML features,
        # but the region is the simpler lexical-mention heuristic instead of
        # the backward slice -- tests whether the slice's specific scope adds
        # anything over this heuristic once both get srcML augmentation.
        aug_fb_varmention = aug_varmention(fb)
        aug_fa_varmention = aug_varmention(fa)
        aug_delta_varmention = aug_fa_varmention - aug_fb_varmention

        items.append({
            "cwe": (r.get("CWE ID") or "unlabeled"),
            "coarse_ok": coarse_ok,
            "coarse_fb": coarse_fb,
            "coarse_fa": coarse_fa,            # patched, restricted to added-line vars
            "coarse_fa_all": coarse_fa_all if coarse_ok else set(),
            "coarse_delta": coarse_delta,
            "aug_fb_slice": aug_fb_slice,
            "aug_fa_slice": aug_fa_slice,
            "aug_delta": aug_delta,
            "aug_fb_whole": aug_fb_whole,
            "aug_fa_whole": aug_fa_whole,
            "aug_delta_whole": aug_delta_whole,
            "aug_fb_varmention": aug_fb_varmention,
            "aug_fa_varmention": aug_fa_varmention,
            "aug_delta_varmention": aug_delta_varmention,
            # (Phase 15) negative-control flag: True iff this pair's diff has
            # an EMPTY deleted-line set (pure addition/refactor -- close to a
            # negative/near-null case for "did the fix change anything here").
            "deleted_empty": (not bool(dell)),
        })

    used = len(items)
    coarse_items = [it for it in items if it["coarse_ok"]]
    print("pairs used: %d | skipped: %d | coarse-usable: %d"
          % (used, skipped, len(coarse_items)))

    out = {
        "n_pairs": used,
        "n_skipped": skipped,
        "n_coarse_usable": len(coarse_items),
    }

    if used < 8:
        print("Too few usable pairs for RQ2.")
        out["status"] = "insufficient"
        return out

    # ---------------- Test A: non-empty delta + correct direction ------------
    def test_a(arm_items, delta_key, fb_key, fa_key):
        n = len(arm_items)
        nonempty = [it for it in arm_items if it[delta_key]]
        higher = 0
        for it in nonempty:
            s_fb = containment(it[delta_key], it[fb_key])
            s_fa = containment(it[delta_key], it[fa_key])
            if s_fa > s_fb:
                higher += 1
        return {
            "n": n,
            "nonempty": len(nonempty),
            "nonempty_pct": (100.0 * len(nonempty) / n) if n else 0.0,
            "higher_of_nonempty": higher,
            "higher_pct": (100.0 * higher / len(nonempty)) if nonempty else 0.0,
        }

    coarse_a = test_a(coarse_items, "coarse_delta", "coarse_fb", "coarse_fa_all")
    aug_a = test_a(items, "aug_delta", "aug_fb_slice", "aug_fa_slice")
    # (Phase 15) deconfound scope (whole-function vs slice) from features
    # (coarse vs rich srcML): same rich features as aug_a, but whole-function
    # scope, so the 3-way comparison COARSE (whole+coarse) / AUG-whole
    # (whole+rich) / AUG-slice (slice+rich) isolates which variable drives
    # the lift.
    aug_whole_a = test_a(items, "aug_delta_whole", "aug_fb_whole", "aug_fa_whole")
    # (Phase 15) the pivotal comparison: same rich features, but the region is
    # the variable-mention lexical heuristic instead of the backward slice.
    aug_varmention_a = test_a(items, "aug_delta_varmention", "aug_fb_varmention", "aug_fa_varmention")

    print("-" * 78)
    print("TEST A — does the fix introduce distinguishing features?")
    print("  COARSE       : non-empty delta %d/%d = %.1f%% | patched-higher %d/%d = %.1f%%"
          % (coarse_a["nonempty"], coarse_a["n"], coarse_a["nonempty_pct"],
             coarse_a["higher_of_nonempty"], coarse_a["nonempty"], coarse_a["higher_pct"]))
    print("  AUG-slice    : non-empty delta %d/%d = %.1f%% | patched-higher %d/%d = %.1f%%"
          % (aug_a["nonempty"], aug_a["n"], aug_a["nonempty_pct"],
             aug_a["higher_of_nonempty"], aug_a["nonempty"], aug_a["higher_pct"]))
    print("  AUG-whole    : non-empty delta %d/%d = %.1f%% | patched-higher %d/%d = %.1f%%"
          % (aug_whole_a["nonempty"], aug_whole_a["n"], aug_whole_a["nonempty_pct"],
             aug_whole_a["higher_of_nonempty"], aug_whole_a["nonempty"], aug_whole_a["higher_pct"]))
    print("  AUG-varment. : non-empty delta %d/%d = %.1f%% | patched-higher %d/%d = %.1f%%"
          % (aug_varmention_a["nonempty"], aug_varmention_a["n"], aug_varmention_a["nonempty_pct"],
             aug_varmention_a["higher_of_nonempty"], aug_varmention_a["nonempty"], aug_varmention_a["higher_pct"]))
    print("  HEADLINE (Test A non-empty delta): coarse %.1f%% -> aug-slice %.1f%% "
          "(aug-whole %.1f%%, aug-varmention %.1f%%)"
          % (coarse_a["nonempty_pct"], aug_a["nonempty_pct"],
             aug_whole_a["nonempty_pct"], aug_varmention_a["nonempty_pct"]))

    out["test_a"] = {
        "coarse": coarse_a, "aug": aug_a,
        "aug_whole": aug_whole_a, "aug_varmention": aug_varmention_a,
    }
    out["headline_test_a"] = {
        "coarse_pct": coarse_a["nonempty_pct"],
        "aug_pct": aug_a["nonempty_pct"],
        "aug_whole_pct": aug_whole_a["nonempty_pct"],
        "aug_varmention_pct": aug_varmention_a["nonempty_pct"],
        "coarse_published_baseline_pct": RQ2_COARSE_BASELINE_PCT,
        "aug_lifts_past_baseline": aug_a["nonempty_pct"] > RQ2_COARSE_BASELINE_PCT,
        "scope_deconfound_note": (
            "aug_pct (slice scope + rich features) vs aug_whole_pct (whole-"
            "function scope + rich features) isolates whether slicing's "
            "scope, not just srcML's richer features, drives the lift over "
            "coarse_pct (whole-function scope + coarse features). "
            "aug_varmention_pct (variable-mention-region scope + rich "
            "features) tests the slice's scope specifically against a "
            "simpler lexical-heuristic region of the same feature richness."
        ),
    }

    # ---------------- Test A by CWE category (Reviewer-requested stratification) --
    # Confound check: is the discrimination-rate lift concentrated in one CWE, or
    # broad across categories? Only report CWEs with >=5 pairs in BOTH arms so a
    # single-pair category can't produce a misleading 0%/100%.
    MIN_CWE_N = 5

    def _cwe_counts(arm_items):
        counts = {}
        for it in arm_items:
            counts[it["cwe"]] = counts.get(it["cwe"], 0) + 1
        return counts

    coarse_cwe_n = _cwe_counts(coarse_items)
    aug_cwe_n = _cwe_counts(items)
    reportable_cwes = sorted(
        cwe for cwe in aug_cwe_n
        if aug_cwe_n.get(cwe, 0) >= MIN_CWE_N and coarse_cwe_n.get(cwe, 0) >= MIN_CWE_N
    )
    by_cwe = []
    for cwe in reportable_cwes:
        c_sub = [it for it in coarse_items if it["cwe"] == cwe]
        a_sub = [it for it in items if it["cwe"] == cwe]
        c_res = test_a(c_sub, "coarse_delta", "coarse_fb", "coarse_fa_all")
        a_res = test_a(a_sub, "aug_delta", "aug_fb_slice", "aug_fa_slice")
        by_cwe.append({
            "cwe": cwe, "n_coarse": c_res["n"], "n_aug": a_res["n"],
            "coarse_nonempty_pct": c_res["nonempty_pct"],
            "aug_nonempty_pct": a_res["nonempty_pct"],
        })
    n_below_threshold = len(aug_cwe_n) - len(reportable_cwes)
    out["test_a_by_cwe"] = {
        "min_n_per_arm": MIN_CWE_N,
        "n_cwe_categories_total": len(aug_cwe_n),
        "n_cwe_categories_reportable": len(reportable_cwes),
        "n_cwe_categories_below_threshold": n_below_threshold,
        "by_cwe": by_cwe,
    }
    print("-" * 78)
    print("TEST A by CWE (n>=%d per arm, %d/%d categories reportable):" %
          (MIN_CWE_N, len(reportable_cwes), len(aug_cwe_n)))
    for row in by_cwe:
        print("  %-16s n=%3d/%3d  coarse=%.1f%%  aug=%.1f%%" %
              (row["cwe"], row["n_coarse"], row["n_aug"],
               row["coarse_nonempty_pct"], row["aug_nonempty_pct"]))

    # ---------------- Test B: generalization (70/30 held-out DB) -------------
    ndb = max(1, min(used - 1, int(round(used * RQ2_TRAIN_FRAC))))
    db, test = items[:ndb], items[ndb:]
    print("-" * 78)
    print("TEST B — generalization: DB=%d TEST=%d (each pair -> 1 vuln +1 patched query)"
          % (len(db), len(test)))

    def eval_arm(label, vuln_key, delta_key, query_key, db_items, test_items):
        """
        For each test query (whole-function feature set):
          sim_vuln  = max containment of any DB vuln_sig in the query.
          sim_patch = containment of that matched CVE's fix delta in the query.
        Rule A (vuln-only): flag iff sim_vuln >= tau.
        Rule B (dual)     : flag iff sim_vuln >= tau AND sim_patch < tau.
        Positive = func_before (vulnerable); negative = func_after (patched).
        Sweep tau; report best-F1 for each rule. Track func_after FPs.
        """
        # Precompute (sim_vuln, sim_patch, label) for every query.
        scored = []
        db_sigs = [(d[vuln_key], d[delta_key]) for d in db_items]
        for t in test_items:
            for q, lbl in ((t[query_key + "_vuln"], 1), (t[query_key + "_patch"], 0)):
                best_v, best_delta = 0.0, set()
                for sig, delta in db_sigs:
                    c = containment(sig, q)
                    if c > best_v:
                        best_v, best_delta = c, delta
                sp = containment(best_delta, q) if best_delta else 0.0
                scored.append((lbl, best_v, sp))

        def evalrule(tau, dual):
            tp = fp = fn = tn = 0
            for lbl, sv, sp in scored:
                flag = (sv >= tau) and ((sp < tau) if dual else True)
                if lbl == 1:
                    tp += int(flag); fn += int(not flag)
                else:
                    fp += int(flag); tn += int(not flag)
            P = tp / (tp + fp) if (tp + fp) else 0.0
            R = tp / (tp + fn) if (tp + fn) else 0.0
            F1 = (2 * P * R / (P + R)) if (P + R) else 0.0
            # (Phase 15 follow-up) balanced accuracy / MCC alongside F1:
            # positive-class F1 on a balanced set is structurally unkind to
            # any rule that ever predicts positive -- a constant "always
            # patched" classifier scores 0.67 F1 and can't be beaten by an
            # honest rule that sometimes says "vulnerable." BA puts a
            # constant classifier at 0.50 instead, so a genuinely-above-
            # chance rule shows up in BA/MCC even when F1 makes it look weak.
            tpr = tp / (tp + fn) if (tp + fn) else 0.0
            tnr = tn / (tn + fp) if (tn + fp) else 0.0
            BA = (tpr + tnr) / 2.0
            mcc_denom = ((tp + fp) * (tp + fn) * (tn + fp) * (tn + fn)) ** 0.5
            MCC = ((tp * tn - fp * fn) / mcc_denom) if mcc_denom else 0.0
            return {"tau": tau, "P": P, "R": R, "F1": F1, "BA": BA, "MCC": MCC,
                    "func_after_fp": fp, "tp": tp, "fp": fp, "fn": fn, "tn": tn}

        taus = [0.3, 0.4, 0.5, 0.6, 0.7, 0.8]
        ruleA = [evalrule(t, False) for t in taus]
        ruleB = [evalrule(t, True) for t in taus]
        bestA = max(ruleA, key=lambda d: d["F1"])
        bestB = max(ruleB, key=lambda d: d["F1"])
        fixedB = ruleB[taus.index(0.5)]
        print("  [%s] RuleA best-F1 tau=%.1f P=%.2f R=%.2f F1=%.2f fa_FP=%d | "
              "RuleB tau=%.1f P=%.2f R=%.2f F1=%.2f BA=%.2f MCC=%.2f fa_FP=%d | "
              "RuleB fixed-tau=0.5 F1=%.2f BA=%.2f MCC=%.2f"
              % (label, bestA["tau"], bestA["P"], bestA["R"], bestA["F1"],
                 bestA["func_after_fp"], bestB["tau"], bestB["P"], bestB["R"],
                 bestB["F1"], bestB["BA"], bestB["MCC"], bestB["func_after_fp"],
                 fixedB["F1"], fixedB["BA"], fixedB["MCC"]))
        return {"ruleA_best": bestA, "ruleB_best": bestB, "ruleB_fixed_tau_0.5": fixedB,
                "scored": scored}

    # Build per-test query feature sets for each arm.
    # COARSE arm query = COARSE-of-whole (use coarse_fa_all for patched, and the
    # whole vulnerable profile coarse_fb for the vulnerable query). vuln_sig is
    # the whole vulnerable profile; delta is coarse_delta.
    for it in test:
        it["coarse_q_vuln"] = it["coarse_fb"]
        it["coarse_q_patch"] = it["coarse_fa_all"]
        it["aug_q_vuln"] = it["aug_fb_whole"]
        it["aug_q_patch"] = it["aug_fa_whole"]

    # We pass vuln_sig/delta keys directly; query keys resolve to *_vuln/*_patch.
    coarse_db = [d for d in db if d["coarse_ok"]]
    coarse_b = eval_arm(
        "COARSE", vuln_key="coarse_fb", delta_key="coarse_delta",
        query_key="coarse_q",
        db_items=coarse_db,
        test_items=[t for t in test if t["coarse_ok"]],
    )
    aug_b = eval_arm(
        "AUG", vuln_key="aug_fb_whole", delta_key="aug_delta",
        query_key="aug_q",
        db_items=db,
        test_items=test,
    )

    print("-" * 78)
    cf1 = coarse_b["ruleB_best"]["F1"]
    af1 = aug_b["ruleB_best"]["F1"]
    cba = coarse_b["ruleB_best"]["BA"]
    aba = aug_b["ruleB_best"]["BA"]
    cmcc = coarse_b["ruleB_best"]["MCC"]
    amcc = aug_b["ruleB_best"]["MCC"]
    cfp = coarse_b["ruleB_best"]["func_after_fp"]
    afp = aug_b["ruleB_best"]["func_after_fp"]
    print("  HEADLINE (Test B dual): AUG F1=%.2f vs COARSE F1=%.2f | "
          "AUG BA=%.2f vs COARSE BA=%.2f | AUG MCC=%.2f vs COARSE MCC=%.2f | "
          "func_after FP AUG=%d vs COARSE=%d -> AUG generalizes %s"
          % (af1, cf1, aba, cba, amcc, cmcc, afp, cfp,
             "better" if (af1 > cf1 or afp < cfp) else "no better"))

    out["test_b"] = {"coarse": coarse_b, "aug": aug_b}
    out["headline_test_b"] = {
        "aug_dual_f1": af1, "coarse_dual_f1": cf1,
        "aug_dual_ba": aba, "coarse_dual_ba": cba,
        "aug_dual_mcc": amcc, "coarse_dual_mcc": cmcc,
        "aug_func_after_fp": afp, "coarse_func_after_fp": cfp,
        "aug_generalizes_better": bool(af1 > cf1 or afp < cfp),
    }

    # ---------------- Real statistics (Reviewer-requested) --------------------
    # Test A: McNemar's test on paired per-CVE non-empty-delta outcomes (same
    # CVE subset -- coarse_items -- evaluated under both representations).
    from scipy.stats import binomtest  # noqa: PLC0415

    def mcnemar_paired(bool_a, bool_b):
        """Exact McNemar's test (binomial on discordant pairs) for two paired
        binary outcome sequences of equal length."""
        assert len(bool_a) == len(bool_b)
        b01 = sum(1 for a, b in zip(bool_a, bool_b) if (not a) and b)   # a=0,b=1
        b10 = sum(1 for a, b in zip(bool_a, bool_b) if a and (not b))   # a=1,b=0
        n_discordant = b01 + b10
        if n_discordant == 0:
            return {"n_discordant": 0, "b01": b01, "b10": b10, "p_value": 1.0}
        res = binomtest(min(b01, b10), n_discordant, 0.5)
        return {"n_discordant": n_discordant, "b01": b01, "b10": b10,
                "p_value": float(res.pvalue)}

    coarse_nonempty_paired = [bool(it["coarse_delta"]) for it in coarse_items]
    aug_nonempty_paired = [bool(it["aug_delta"]) for it in coarse_items]
    test_a_mcnemar = mcnemar_paired(coarse_nonempty_paired, aug_nonempty_paired)

    # Test B: bootstrap 95% CI on dual-rule F1 for each arm (query pools differ
    # in size between arms -- coarse restricted to coarse_ok test items -- so we
    # report independent bootstrap CIs rather than force an invalid pairing).
    def bootstrap_f1_ci(label_pred_pairs, n_boot=1000, seed=SEED):
        if not label_pred_pairs:
            return {"status": "no_data"}
        rng_local = random.Random(seed)
        labels_arr = [lp[0] for lp in label_pred_pairs]
        preds_arr = [lp[1] for lp in label_pred_pairs]
        n = len(labels_arr)
        boot_f1s = []
        for _ in range(n_boot):
            idxs = [rng_local.randrange(n) for _ in range(n)]
            tp = sum(1 for i in idxs if labels_arr[i] == 1 and preds_arr[i] == 1)
            fp = sum(1 for i in idxs if labels_arr[i] == 0 and preds_arr[i] == 1)
            fn = sum(1 for i in idxs if labels_arr[i] == 1 and preds_arr[i] == 0)
            p = tp / (tp + fp) if (tp + fp) else 0.0
            r = tp / (tp + fn) if (tp + fn) else 0.0
            f1 = (2 * p * r / (p + r)) if (p + r) else 0.0
            boot_f1s.append(f1)
        boot_f1s.sort()
        lo = boot_f1s[int(0.025 * n_boot)]
        hi = boot_f1s[int(0.975 * n_boot) - 1]
        return {"n": n, "n_boot": n_boot, "ci95": [float(lo), float(hi)],
                "mean": float(np.mean(boot_f1s))}

    # eval_arm's `scored` list is local to that closure; recompute label/pred
    # pairs at each arm's own best tau by re-deriving from ruleB_best.
    def scored_to_labelpred(items_db, items_test, vuln_key, delta_key, query_key, tau):
        db_sigs = [(d[vuln_key], d[delta_key]) for d in items_db]
        out_lp = []
        for t in items_test:
            for q, lbl in ((t[query_key + "_vuln"], 1), (t[query_key + "_patch"], 0)):
                best_v, best_delta = 0.0, set()
                for sig, delta in db_sigs:
                    c = containment(sig, q)
                    if c > best_v:
                        best_v, best_delta = c, delta
                sp = containment(best_delta, q) if best_delta else 0.0
                pred = int((best_v >= tau) and (sp < tau))
                out_lp.append((lbl, pred))
        return out_lp

    coarse_lp = scored_to_labelpred(
        coarse_db, [t for t in test if t["coarse_ok"]],
        "coarse_fb", "coarse_delta", "coarse_q", coarse_b["ruleB_best"]["tau"])
    aug_lp = scored_to_labelpred(
        db, test, "aug_fb_whole", "aug_delta", "aug_q", aug_b["ruleB_best"]["tau"])

    out["statistics"] = {
        "test_a_mcnemar": test_a_mcnemar,
        "test_b_coarse_bootstrap_ci": bootstrap_f1_ci(coarse_lp),
        "test_b_aug_bootstrap_ci": bootstrap_f1_ci(aug_lp),
        "note": ("Test A: exact McNemar's test (binomial on discordant pairs) on "
                 "paired non-empty-delta outcomes, same CVE subset for both arms. "
                 "Test B: 1000-resample bootstrap 95% CI on dual-rule F1 per arm "
                 "(reported independently, not paired, because the coarse arm's "
                 "test-query pool is a strict subset of the aug arm's -- coarse "
                 "extraction fails for some CVEs -- so per-query pairing across "
                 "arms is not valid)."),
    }
    print("-" * 78)
    print("STATISTICS: Test A McNemar p=%s (n_discordant=%d) | Test B coarse F1 CI=%s aug F1 CI=%s"
          % (test_a_mcnemar["p_value"], test_a_mcnemar["n_discordant"],
             out["statistics"]["test_b_coarse_bootstrap_ci"].get("ci95"),
             out["statistics"]["test_b_aug_bootstrap_ci"].get("ci95")))

    # ---------------- Trivial baselines (Phase 15 critique response) ---------
    # Test B's query set is exactly balanced by construction (one vulnerable +
    # one patched query per held-out CVE). Report the two textbook trivial
    # baselines for a balanced binary set, computed exactly (closed-form
    # expectation for a uniform-random predictor, not a seeded simulation) so
    # F1=0.37/0.53 can be read against a coin flip and an always-positive
    # classifier instead of in isolation.
    def trivial_baselines(scored):
        labels = [lbl for (lbl, _sv, _sp) in scored]
        n = len(labels)
        n_pos = sum(labels)
        n_neg = n - n_pos
        # Always predict positive (vulnerable).
        tp, fp, fn = n_pos, n_neg, 0
        p_always = tp / (tp + fp) if (tp + fp) else 0.0
        r_always = tp / (tp + fn) if (tp + fn) else 0.0
        f1_always_positive = (2 * p_always * r_always / (p_always + r_always)) if (p_always + r_always) else 0.0
        # Uniform-random guess (flag probability 0.5): closed-form expected P/R/F1.
        etp, efp, efn = n_pos * 0.5, n_neg * 0.5, n_pos * 0.5
        p_rand = etp / (etp + efp) if (etp + efp) else 0.0
        r_rand = etp / (etp + efn) if (etp + efn) else 0.0
        f1_random_guess = (2 * p_rand * r_rand / (p_rand + r_rand)) if (p_rand + r_rand) else 0.0
        # BA/MCC for both trivial classifiers, closed-form: always-positive
        # has TPR=1.0, TNR=0.0 -> BA=0.5, MCC=0.0 exactly (not 0.667 the way
        # F1 reports it); uniform-random has TPR=TNR=0.5 -> BA=0.5, MCC=0.0.
        # Both trivial baselines are exactly at chance under BA/MCC on a
        # balanced set, unlike F1 where always-positive scores 0.667.
        return {
            "n": n, "n_pos": n_pos, "n_neg": n_neg, "balanced": (n_pos == n_neg),
            "always_positive_f1": f1_always_positive,
            "random_guess_f1": f1_random_guess,
            "always_positive_ba": 0.5, "random_guess_ba": 0.5,
            "always_positive_mcc": 0.0, "random_guess_mcc": 0.0,
        }

    coarse_trivial = trivial_baselines(coarse_b["scored"])
    aug_trivial = trivial_baselines(aug_b["scored"])
    out["test_b_trivial_baselines"] = {
        "coarse": coarse_trivial,
        "aug": aug_trivial,
        "note": ("Positive-class (binary) F1 -- the same definition used "
                 "throughout this script -- not macro-averaged. Computed "
                 "exactly (closed-form expectation), not by seeded "
                 "simulation, since Test B's query set is exactly balanced "
                 "by construction (one vulnerable + one patched query per "
                 "held-out CVE)."),
    }
    print("TRIVIAL BASELINES (Test B, balanced by construction): "
          "always-positive F1=%.3f BA=%.2f MCC=%.2f | random-guess F1=%.3f BA=%.2f MCC=%.2f | "
          "COARSE dual-rule F1=%.2f BA=%.2f MCC=%.2f | AUG dual-rule F1=%.2f BA=%.2f MCC=%.2f"
          % (aug_trivial["always_positive_f1"], aug_trivial["always_positive_ba"], aug_trivial["always_positive_mcc"],
             aug_trivial["random_guess_f1"], aug_trivial["random_guess_ba"], aug_trivial["random_guess_mcc"],
             cf1, cba, cmcc, af1, aba, amcc))

    # ---------------- Robustness checks (Reviewer-requested) -----------------
    # (a) Fixed, pre-specified tau=0.5 (not the swept-per-arm optimum) for both
    #     arms, so readers can bound how much of the headline gap is tau-search
    #     optimism vs a real effect at an operating point chosen in advance.
    coarse_fixed = coarse_b["ruleB_fixed_tau_0.5"]
    aug_fixed = aug_b["ruleB_fixed_tau_0.5"]

    # (b) AUG re-evaluated on the SAME CVE-pair subset the coarse arm is
    #     restricted to (coarse extraction fails on some pairs), instead of
    #     AUG's full pool, so the headline 0.37->0.53 improvement is also
    #     checked on a matched, apples-to-apples subset.
    aug_b_restricted = eval_arm(
        "AUG-on-coarse-subset", vuln_key="aug_fb_whole", delta_key="aug_delta",
        query_key="aug_q",
        db_items=coarse_db,
        test_items=[t for t in test if t["coarse_ok"]],
    )
    aug_restricted_lp = scored_to_labelpred(
        coarse_db, [t for t in test if t["coarse_ok"]],
        "aug_fb_whole", "aug_delta", "aug_q", aug_b_restricted["ruleB_best"]["tau"])

    # (c) Test-A non-empty-delta RATE (not just dual-rule F1) for the AUG arm,
    #     restricted to exactly the 316-pair coarse-usable subset -- the direct
    #     apples-to-apples counterpart to the coarse arm's 25.6% headline, since
    #     the raw 25.6% (n=316) vs 52.8% (n=400) headline compares different
    #     denominators.
    aug_a_same_subset = test_a(coarse_items, "aug_delta", "aug_fb_slice", "aug_fa_slice")
    aug_whole_a_same_subset = test_a(coarse_items, "aug_delta_whole", "aug_fb_whole", "aug_fa_whole")
    aug_varmention_a_same_subset = test_a(
        coarse_items, "aug_delta_varmention", "aug_fb_varmention", "aug_fa_varmention")

    # (d) Negative control (Phase 15 critique response): the AUG-slice arm's
    #     non-empty-delta RATE on the 106-ish pairs whose diff has an EMPTY
    #     deleted-line set (pure addition/refactor, near-negative-class for
    #     "did the fix change anything here"). Matching denominators (as in
    #     (c) above) controls sample composition, not vocabulary cardinality;
    #     this is the actual control for "a richer feature vocabulary
    #     mechanically inflates the discrimination rate" -- if AUG fires
    #     non-empty nearly as often on these near-negatives as on real fixes,
    #     the metric is measuring vocabulary richness, not a real signal.
    neg_control_items = [it for it in items if it["deleted_empty"]]
    neg_control_a = test_a(neg_control_items, "aug_delta", "aug_fb_slice", "aug_fa_slice") \
        if neg_control_items else {"n": 0, "nonempty": 0, "nonempty_pct": 0.0}
    # (Phase 15 follow-up) run the SAME negative control on the coarse arm --
    # if coarse's false-fire rate on refactor-only pairs is comparably high,
    # the coarse-vs-augmented COMPARATIVE gap on real fixes is not itself a
    # vocabulary-size artifact (both arms would share the same inflation);
    # if coarse's rate is much lower, the comparative claim inherits the
    # negative-control problem and must be qualified further.
    neg_control_coarse_items = [it for it in neg_control_items if it["coarse_ok"]]
    neg_control_coarse_a = test_a(
        neg_control_coarse_items, "coarse_delta", "coarse_fb", "coarse_fa_all") \
        if neg_control_coarse_items else {"n": 0, "nonempty": 0, "nonempty_pct": 0.0}
    # Same-subset AUG-slice rate, so the two negative-control rates are over
    # an identical pair set (coarse-usable refactor-only pairs only).
    neg_control_aug_same_subset = test_a(
        neg_control_coarse_items, "aug_delta", "aug_fb_slice", "aug_fa_slice") \
        if neg_control_coarse_items else {"n": 0, "nonempty": 0, "nonempty_pct": 0.0}

    out["robustness"] = {
        "fixed_tau_0.5": {
            "coarse_F1": coarse_fixed["F1"], "coarse_func_after_fp": coarse_fixed["func_after_fp"],
            "aug_F1": aug_fixed["F1"], "aug_func_after_fp": aug_fixed["func_after_fp"],
            "note": "Dual-rule (Rule B) F1 at a single pre-specified tau=0.5, "
                    "not each arm's swept-best tau -- bounds test-set-selection optimism.",
        },
        "intersection_restricted": {
            "n_pairs": len(coarse_db) + len([t for t in test if t["coarse_ok"]]),
            "coarse_dual_f1_swept": cf1,
            "aug_dual_f1_swept_same_subset": aug_b_restricted["ruleB_best"]["F1"],
            "aug_dual_f1_ci95_same_subset": bootstrap_f1_ci(aug_restricted_lp).get("ci95"),
            "aug_nonempty_pct_same_subset": aug_a_same_subset["nonempty_pct"],
            "aug_whole_nonempty_pct_same_subset": aug_whole_a_same_subset["nonempty_pct"],
            "aug_varmention_nonempty_pct_same_subset": aug_varmention_a_same_subset["nonempty_pct"],
            "coarse_nonempty_pct_same_subset": coarse_a["nonempty_pct"],
            "note": "AUG re-run restricted to exactly the CVE pairs the coarse arm "
                    "can compute (coarse srcSlice extraction fails on some pairs), "
                    "instead of AUG's full pool, as a robustness check on the "
                    "headline comparison. aug_nonempty_pct_same_subset is Test A's "
                    "non-empty-delta rate for AUG computed on the identical 316-pair "
                    "subset as coarse_nonempty_pct_same_subset (== the coarse Test A "
                    "rate), the matched denominator counterpart to the raw 25.6% "
                    "(n=316) vs 52.8% (n=400) headline comparison. "
                    "aug_whole_nonempty_pct_same_subset and "
                    "aug_varmention_nonempty_pct_same_subset are the same matched-"
                    "subset check for the whole-function-scope and variable-"
                    "mention-region-scope conditions respectively.",
        },
        "negative_control": {
            "n_pairs": neg_control_a["n"],
            "aug_slice_nonempty_pct": neg_control_a["nonempty_pct"],
            "real_fix_aug_slice_nonempty_pct": aug_a["nonempty_pct"],
            "n_pairs_coarse_usable": neg_control_coarse_a["n"],
            "coarse_nonempty_pct": neg_control_coarse_a["nonempty_pct"],
            "aug_slice_nonempty_pct_same_subset": neg_control_aug_same_subset["nonempty_pct"],
            "note": "AUG-slice's non-empty-delta rate on the pairs whose diff has "
                    "an EMPTY deleted-line set (pure addition/refactor, a near-"
                    "negative-class for whether the fix changed the vulnerable "
                    "region at all), vs. the same rate on real fixes "
                    "(real_fix_aug_slice_nonempty_pct, == the aug headline). "
                    "This is the actual control against vocabulary-size "
                    "inflation of the discrimination-rate metric -- matching "
                    "denominators (as above) controls sample composition, not "
                    "vocabulary cardinality; a high rate here would mean the "
                    "metric is picking up srcML's larger feature vocabulary "
                    "rather than a genuine fix signal. coarse_nonempty_pct is the "
                    "SAME negative control run on the coarse arm, restricted to "
                    "the coarse-usable subset of these pairs (n_pairs_coarse_usable), "
                    "with aug_slice_nonempty_pct_same_subset as AUG-slice's rate on "
                    "that identical subset -- this tests whether the comparative "
                    "coarse-vs-augmented gap on real fixes is itself a vocabulary-"
                    "size artifact (both arms inflated similarly) or whether coarse "
                    "is comparatively immune (only augmented is inflated).",
        },
    }
    print("-" * 78)
    print("ROBUSTNESS: fixed tau=0.5 F1 coarse=%.2f aug=%.2f | "
          "intersection-restricted (n=%d) coarse=%.2f aug=%.2f (CI=%s) | "
          "Test-A nonempty%% coarse=%.1f aug-slice(same subset)=%.1f "
          "aug-whole(same subset)=%.1f aug-varmention(same subset)=%.1f"
          % (coarse_fixed["F1"], aug_fixed["F1"],
             out["robustness"]["intersection_restricted"]["n_pairs"], cf1,
             out["robustness"]["intersection_restricted"]["aug_dual_f1_swept_same_subset"],
             out["robustness"]["intersection_restricted"]["aug_dual_f1_ci95_same_subset"],
             coarse_a["nonempty_pct"], aug_a_same_subset["nonempty_pct"],
             aug_whole_a_same_subset["nonempty_pct"], aug_varmention_a_same_subset["nonempty_pct"]))
    print("NEGATIVE CONTROL (coarse arm, n=%d coarse-usable of %d): coarse=%.1f%% "
          "vs aug-slice(same subset)=%.1f%%"
          % (neg_control_coarse_a["n"], neg_control_a["n"],
             neg_control_coarse_a["nonempty_pct"], neg_control_aug_same_subset["nonempty_pct"]))
    print("NEGATIVE CONTROL: AUG-slice non-empty-delta on empty-deleted-line pairs "
          "(n=%d) = %.1f%% vs on real fixes = %.1f%%"
          % (neg_control_a["n"], neg_control_a["nonempty_pct"], aug_a["nonempty_pct"]))

    return out


# --------------------------------------------------------------------------- #
# RQ3-AUG
# --------------------------------------------------------------------------- #
def build_detection_set(rows):
    """
    Positives: all vul==1 func_before. Negatives: vul==0 functions, sampled at
    1:4, capped to ~DET_CAP total. Returns (codes, labels) lists.
    """
    pos = [r["func_before"] for r in rows
           if _is_vuln(r.get("vul")) and isinstance(r.get("func_before"), str)
           and r["func_before"].strip()]
    neg_pool = [r["func_before"] for r in rows
                if (r.get("vul") in (0, "0", False))
                and isinstance(r.get("func_before"), str)
                and r["func_before"].strip()]

    rng = random.Random(SEED)
    rng.shuffle(pos)
    rng.shuffle(neg_pool)

    # Respect the total cap while keeping the 1:4 positive:negative ratio.
    max_pos = max(1, DET_CAP // (1 + DET_NEG_RATIO))
    n_pos = min(len(pos), max_pos)
    n_neg = min(len(neg_pool), n_pos * DET_NEG_RATIO)
    pos = pos[:n_pos]
    neg = neg_pool[:n_neg]

    codes = pos + neg
    labels = [1] * len(pos) + [0] * len(neg)
    # Shuffle jointly so folds aren't ordered by class.
    idx = list(range(len(codes)))
    rng.shuffle(idx)
    codes = [codes[i] for i in idx]
    labels = [labels[i] for i in idx]
    return codes, labels


def run_rq3(rows):
    print("=" * 78)
    print("RQ3-AUG — detection: does srcML augmentation recover the signal?")
    print("=" * 78)

    codes, labels = build_detection_set(rows)
    y = np.array(labels, dtype=int)
    n_pos = int(y.sum())
    n_neg = int(len(y) - n_pos)
    print("detection set: %d total | %d positive | %d negative (ratio ~1:%d)"
          % (len(y), n_pos, n_neg, DET_NEG_RATIO))

    # (Phase 15) trivial baselines for this IMBALANCED (~1:DET_NEG_RATIO) set --
    # NOT the balanced-set 0.50/0.667 constants (those apply to RQ2 Test B
    # only, computed separately there); here the "always predict vulnerable"
    # F1 is 2p/(1+p) with p = n_pos/(n_pos+n_neg).
    _p_pos = (n_pos / float(n_pos + n_neg)) if (n_pos + n_neg) else 0.0
    _always_pos_f1_imbalanced = (2.0 * _p_pos / (1.0 + _p_pos)) if (1.0 + _p_pos) else 0.0

    out = {
        "n_total": int(len(y)),
        "n_positive": n_pos,
        "n_negative": n_neg,
        "whole_text_published_baseline": {"f1": RQ3_WHOLE_F1, "pr_auc": RQ3_WHOLE_PRAUC},
        "trivial_baseline_always_positive_f1": _always_pos_f1_imbalanced,
        "trivial_baseline_note": (
            "Closed-form F1 for an 'always predict vulnerable' classifier on "
            "this set's actual (imbalanced, ~1:%d) class ratio -- NOT the "
            "balanced 0.50/0.667 constants reported for RQ2 Test B, which is "
            "a separately-constructed, genuinely-balanced 50/50 query set."
            % DET_NEG_RATIO
        ),
    }

    if len(y) < 50 or n_pos < N_FOLDS or n_neg < N_FOLDS:
        print("Too few samples / class members for RQ3 CV.")
        out["status"] = "insufficient"
        return out

    # Precompute string/feature representations once per function (cached).
    whole_text = []
    sl_text = []
    aug_w = []   # space-joined abstract feature tokens (whole)
    aug_s = []   # space-joined abstract feature tokens (slice)
    aug_vm = []  # space-joined abstract feature tokens (variable-mention region)
    for c in codes:
        whole_text.append(c)
        sl_text.append(slice_text(c) or " ")
        aw = sorted(aug_whole(c))
        asl = sorted(aug_slice(c))
        avm = sorted(aug_varmention(c))
        # Feature tokens may contain characters TfidfVectorizer would split on;
        # token_pattern=r"[^ ]+" treats each space-separated chunk as one token,
        # and our features contain no spaces, so this is lossless.
        aug_w.append(" ".join(aw) if aw else "EMPTYFEAT")
        aug_s.append(" ".join(asl) if asl else "EMPTYFEAT")
        aug_vm.append(" ".join(avm) if avm else "EMPTYFEAT")

    def make_vectorizer(kind):
        if kind in ("text",):
            return TfidfVectorizer(max_features=2000, ngram_range=(1, 2))
        # abstract-feature bag: one token per feature, unigram only.
        return TfidfVectorizer(max_features=2000, ngram_range=(1, 1),
                               token_pattern=r"[^ ]+", lowercase=False)

    reps = OrderedDict([
        ("WHOLE_TEXT", (whole_text, "text")),
        ("SLICE_TEXT", (sl_text, "text")),
        ("AUG_WHOLE", (aug_w, "feat")),
        ("AUG_SLICE", (aug_s, "feat")),
        # (Phase 15) pivotal comparison: same rich srcML features as AUG_SLICE,
        # but over the variable-mention-region heuristic instead of the
        # backward slice -- tests whether the slice's scope adds anything.
        ("AUG_VARMENTION", (aug_vm, "feat")),
    ])

    # Repeated stratified k-fold: N_REPEATS independent 5-fold partitions (50
    # total train/test splits) instead of a single 5-fold split, so paired
    # per-split statistics (Wilcoxon, bootstrap CIs) have real power instead
    # of being capped at n=5 (min two-sided Wilcoxon p = 0.0625 at n=5).
    skf = RepeatedStratifiedKFold(n_splits=N_FOLDS, n_repeats=N_REPEATS, random_state=SEED)
    folds = list(skf.split(np.zeros(len(y)), y))

    rep_results = OrderedDict()
    for name, (corpus, kind) in reps.items():
        f1s, praucs, precs, recs = [], [], [], []
        corpus = np.array(corpus, dtype=object)
        for tr, te in folds:
            vec = make_vectorizer(kind)
            try:
                Xtr = vec.fit_transform([corpus[i] for i in tr])
                Xte = vec.transform([corpus[i] for i in te])
            except ValueError:
                # Empty vocabulary (degenerate fold); skip this fold.
                continue
            clf = LogisticRegression(max_iter=500, class_weight="balanced")
            clf.fit(Xtr, y[tr])
            pred = clf.predict(Xte)
            try:
                proba = clf.predict_proba(Xte)[:, 1]
            except Exception:
                proba = pred.astype(float)
            f1s.append(f1_score(y[te], pred, zero_division=0))
            precs.append(precision_score(y[te], pred, zero_division=0))
            recs.append(recall_score(y[te], pred, zero_division=0))
            try:
                praucs.append(average_precision_score(y[te], proba))
            except Exception:
                praucs.append(0.0)

        def m(xs):
            return float(np.mean(xs)) if xs else 0.0

        def s(xs):
            return float(np.std(xs)) if xs else 0.0

        res = {"f1": m(f1s), "pr_auc": m(praucs),
               "precision": m(precs), "recall": m(recs),
               "f1_std": s(f1s), "pr_auc_std": s(praucs),
               "n_folds": len(f1s),
               "per_fold_f1": [float(x) for x in f1s],
               "per_fold_pr_auc": [float(x) for x in praucs]}
        rep_results[name] = res
        print("  %-11s F1=%.3f  PR-AUC=%.3f  P=%.3f  R=%.3f  (folds=%d)"
              % (name, res["f1"], res["pr_auc"], res["precision"],
                 res["recall"], res["n_folds"]))

    out["representations"] = rep_results

    # ---------------- Real paired statistics (Reviewer-requested) ------------
    # Paired Wilcoxon signed-rank test on per-fold F1 (same 5 folds across all
    # representations, so comparisons are paired), plus Cliff's delta effect
    # size and a normal-approximation 95% CI on the mean F1 difference.
    from scipy.stats import wilcoxon, t as _t_dist  # noqa: PLC0415

    def cliffs_delta(a, b):
        a, b = list(a), list(b)
        gt = sum(1 for x in a for y in b if x > y)
        lt = sum(1 for x in a for y in b if x < y)
        n = len(a) * len(b)
        return (gt - lt) / n if n else 0.0

    def nadeau_bengio_test(a, b, n_train_folds=N_FOLDS - 1, n_test_folds=1):
        """Nadeau & Bengio (2003) corrected resampled paired t-test for
        repeated k-fold CV. The naive paired t-test (and Wilcoxon) treats the
        N_FOLDS*N_REPEATS splits as independent, which understates variance
        because splits from the same repeated-CV design share overlapping
        train/test data; this correction inflates the variance term by the
        test/train size ratio to account for that dependence."""
        diffs = np.asarray(a, dtype=float) - np.asarray(b, dtype=float)
        n = len(diffs)
        if n < 2:
            return {"stat": None, "p": None, "df": n - 1}
        mean_diff = float(np.mean(diffs))
        var_diff = float(np.var(diffs, ddof=1))
        test_train_ratio = n_test_folds / float(n_train_folds)
        denom = float(np.sqrt((1.0 / n + test_train_ratio) * var_diff))
        df = n - 1
        if denom == 0.0:
            return {"stat": None, "p": (1.0 if mean_diff == 0.0 else 0.0), "df": df}
        stat = mean_diff / denom
        p = float(2.0 * (1.0 - _t_dist.cdf(abs(stat), df)))
        return {"stat": float(stat), "p": p, "df": df}

    def paired_stats(name_a, name_b):
        fa = rep_results.get(name_a, {}).get("per_fold_f1") or []
        fb = rep_results.get(name_b, {}).get("per_fold_f1") or []
        if len(fa) != len(fb) or len(fa) < 2:
            return {"status": "insufficient_paired_folds", "n": len(fa)}
        diffs = np.array(fa) - np.array(fb)
        mean_diff = float(np.mean(diffs))
        se = float(np.std(diffs, ddof=1) / np.sqrt(len(diffs))) if len(diffs) > 1 else 0.0
        ci95 = (mean_diff - 1.96 * se, mean_diff + 1.96 * se)
        try:
            stat, p = wilcoxon(fa, fb)
            stat, p = float(stat), float(p)
        except Exception as e:
            stat, p = None, None
        nb = nadeau_bengio_test(fa, fb)
        return {
            "n_folds": len(fa),
            "mean_f1_a": float(np.mean(fa)), "mean_f1_b": float(np.mean(fb)),
            "mean_diff": mean_diff, "ci95_diff": [float(ci95[0]), float(ci95[1])],
            "wilcoxon_stat": stat, "wilcoxon_p": p,
            "nb_stat": nb["stat"], "nb_p": nb["p"], "nb_df": nb["df"],
            "cliffs_delta": cliffs_delta(fa, fb),
        }

    def holm_bonferroni(named_pvalues):
        """Holm-Bonferroni step-down correction. named_pvalues: list of
        (name, p) with p possibly None (skipped). Returns {name: p_corrected}."""
        items = [(n, p) for n, p in named_pvalues if p is not None]
        items.sort(key=lambda np_: np_[1])
        m = len(items)
        corrected = {}
        running_max = 0.0
        for i, (n, p) in enumerate(items):
            adj = min(1.0, p * (m - i))
            running_max = max(running_max, adj)
            corrected[n] = running_max
        for n, p in named_pvalues:
            if p is None:
                corrected[n] = None
        return corrected

    pair_names = [
        ("aug_slice_vs_whole_text", paired_stats("AUG_SLICE", "WHOLE_TEXT")),
        ("aug_slice_vs_slice_text", paired_stats("AUG_SLICE", "SLICE_TEXT")),
        ("aug_whole_vs_whole_text", paired_stats("AUG_WHOLE", "WHOLE_TEXT")),
        ("slice_text_vs_whole_text", paired_stats("SLICE_TEXT", "WHOLE_TEXT")),
        # (Phase 15) the pivotal comparison: does the backward slice's scope
        # beat the simpler variable-mention-region heuristic once both get
        # the same srcML augmentation?
        ("aug_slice_vs_aug_varmention", paired_stats("AUG_SLICE", "AUG_VARMENTION")),
    ]
    holm_p_wilcoxon = holm_bonferroni(
        [(n, d.get("wilcoxon_p")) for n, d in pair_names]
    )
    holm_p_nb = holm_bonferroni(
        [(n, d.get("nb_p")) for n, d in pair_names]
    )
    for n, d in pair_names:
        if "wilcoxon_p" in d:
            d["wilcoxon_p_holm"] = holm_p_wilcoxon.get(n)
        if "nb_p" in d:
            d["nb_p_holm"] = holm_p_nb.get(n)

    out["statistics"] = dict(pair_names)
    out["statistics"]["note"] = (
        "Repeated stratified 5-fold CV, %d repeats (%d total paired train/test "
        "splits per representation, seed=%d) for the five lightweight "
        "TF-IDF/logistic-regression representations (WHOLE_TEXT, SLICE_TEXT, "
        "AUG_WHOLE, AUG_SLICE, AUG_VARMENTION). PRIMARY inferential test: "
        "Nadeau-Bengio (2003) corrected resampled paired t-test (nb_stat/nb_p), "
        "which inflates the naive paired-t variance by the test/train size "
        "ratio (1/(k-1) for k-fold CV) to account for the non-independence of "
        "splits drawn from the same repeated-CV design -- the raw Wilcoxon "
        "signed-rank test (wilcoxon_stat/wilcoxon_p) is retained only as a "
        "secondary, non-primary reference and is known to overstate "
        "significance under this dependence structure. Cliff's delta is the "
        "reported effect size; 95%% CI on the mean F1 difference via normal "
        "approximation. nb_p_holm is the Holm-Bonferroni-corrected p-value "
        "(primary) across all 5 pairwise comparisons reported here (the 4 "
        "original comparisons plus aug_slice_vs_aug_varmention, the pivotal "
        "test of whether the backward slice's scope beats a simpler "
        "variable-mention-region heuristic once both get the same srcML "
        "augmentation); wilcoxon_p_holm is kept for reference only. "
        "GNN/CodeBERT/fusion baselines are unaffected (separate scripts, "
        "single 5-fold CV each, reported without this correction)."
        % (N_REPEATS, N_FOLDS * N_REPEATS, SEED)
    )
    print("-" * 78)
    print("STATISTICS (paired, per-fold):")
    for k, v in out["statistics"].items():
        if isinstance(v, dict) and "wilcoxon_p" in v:
            print("  %-28s diff=%.3f p=%s cliffs_delta=%.3f"
                  % (k, v["mean_diff"], v["wilcoxon_p"], v["cliffs_delta"]))

    wt = rep_results.get("WHOLE_TEXT", {}).get("f1", 0.0)
    aus = rep_results.get("AUG_SLICE", {}).get("f1", 0.0)
    auw = rep_results.get("AUG_WHOLE", {}).get("f1", 0.0)
    slt = rep_results.get("SLICE_TEXT", {}).get("f1", 0.0)
    avm = rep_results.get("AUG_VARMENTION", {}).get("f1", 0.0)
    print("-" * 78)
    print("  HEADLINE (RQ3): WHOLE_TEXT F1=%.3f | SLICE_TEXT F1=%.3f | "
          "AUG_WHOLE F1=%.3f | AUG_SLICE F1=%.3f | AUG_VARMENTION F1=%.3f"
          % (wt, slt, auw, aus, avm))
    print("  AUG_SLICE %s WHOLE_TEXT ; AUG_WHOLE %s WHOLE_TEXT ; "
          "AUG_SLICE %s AUG_VARMENTION"
          % ("matches/beats" if aus >= wt else "below",
             "matches/beats" if auw >= wt else "below",
             "beats" if aus > avm else ("ties" if aus == avm else "LOSES TO")))
    print("  trivial baseline (always-predict-vulnerable, this set's ratio): F1=%.3f"
          % _always_pos_f1_imbalanced)
    out["headline_rq3"] = {
        "whole_text_f1": wt, "slice_text_f1": slt,
        "aug_whole_f1": auw, "aug_slice_f1": aus, "aug_varmention_f1": avm,
        "aug_slice_beats_whole_text": bool(aus >= wt),
        "aug_whole_beats_whole_text": bool(auw >= wt),
        "aug_slice_beats_aug_varmention": bool(aus > avm),
        "trivial_baseline_always_positive_f1": _always_pos_f1_imbalanced,
    }
    return out


# --------------------------------------------------------------------------- #
# Main
# --------------------------------------------------------------------------- #
def main():
    print("#" * 78)
    print("# augmented_study.py — C2: srcML augmentation of the slice")
    print("# seed=%d  data=%s" % (SEED, DATA_PATH))
    print("#" * 78)

    rows = load_rows()
    print("loaded %d rows" % len(rows))

    results = {
        "seed": SEED,
        "data_path": os.path.relpath(DATA_PATH, os.path.dirname(os.path.abspath(__file__))),
        "n_rows": len(rows),
    }

    if not rows:
        results["status"] = "no_data"
        _write(results)
        return

    # RQ2 first.
    try:
        results["rq2_aug"] = run_rq2(rows)
    except Exception as e:
        results["rq2_aug"] = {"status": "error", "error": repr(e)}
        print("RQ2 error: %r" % e)

    # RQ3.
    try:
        results["rq3_aug"] = run_rq3(rows)
    except Exception as e:
        results["rq3_aug"] = {"status": "error", "error": repr(e)}
        print("RQ3 error: %r" % e)

    results["tool_failures"] = dict(_fail)

    # Write the JSON before anything optional.
    _write(results)

    # ---- Clean summary with the two headline comparisons ----
    print("=" * 78)
    print("SUMMARY")
    print("=" * 78)
    h2a = results.get("rq2_aug", {}).get("headline_test_a")
    h2b = results.get("rq2_aug", {}).get("headline_test_b")
    h3 = results.get("rq3_aug", {}).get("headline_rq3")
    if h2a:
        print("RQ2 (Test A discrimination): COARSE %.1f%% -> AUG %.1f%%  "
              "(baseline %.0f%%, lifts past baseline: %s)"
              % (h2a["coarse_pct"], h2a["aug_pct"],
                 RQ2_COARSE_BASELINE_PCT, h2a["aug_lifts_past_baseline"]))
    if h2b:
        print("RQ2 (Test B generalization): AUG dual F1=%.2f vs COARSE F1=%.2f "
              "| fa-FP AUG=%d vs COARSE=%d (AUG better: %s)"
              % (h2b["aug_dual_f1"], h2b["coarse_dual_f1"],
                 h2b["aug_func_after_fp"], h2b["coarse_func_after_fp"],
                 h2b["aug_generalizes_better"]))
    if h3:
        print("RQ3 (detection): WHOLE_TEXT F1=%.3f -> AUG_SLICE F1=%.3f, "
              "AUG_WHOLE F1=%.3f (AUG_SLICE recovers signal: %s)"
              % (h3["whole_text_f1"], h3["aug_slice_f1"], h3["aug_whole_f1"],
                 h3["aug_slice_beats_whole_text"]))
    print("tool failures:", dict(_fail))
    print("results written to:", RESULTS_PATH)


def _write(results):
    try:
        os.makedirs(RESULTS_DIR, exist_ok=True)
        with open(RESULTS_PATH, "w", encoding="utf-8") as fh:
            json.dump(results, fh, indent=2, default=_json_default)
    except Exception as e:
        sys.stderr.write("WARN: could not write results JSON: %r\n" % e)


def _json_default(o):
    if isinstance(o, (set, frozenset)):
        return sorted(o)
    if isinstance(o, (np.integer,)):
        return int(o)
    if isinstance(o, (np.floating,)):
        return float(o)
    return str(o)


if __name__ == "__main__":
    main()
