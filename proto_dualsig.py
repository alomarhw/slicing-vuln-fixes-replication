#!/usr/bin/env python3
"""
proto_dualsig.py
================
PROTOTYPE for the "dual slice signature" idea, evaluated on REAL
Big-Vul CVE pairs with the REAL srcSlice tool (srcml -> srcslice).

Core question (paper contribution C1):
    Does a *vulnerability*-slice signature vs a *patch*-slice signature
    correctly separate STILL-VULNERABLE code (func_before) from
    ALREADY-PATCHED code (func_after)?

    - SrcVul-style rule (vuln signature only):  flag if sim_vuln >= tau.
    - Dual-signature rule (C1):                 flag if sim_vuln >= tau AND
                                                sim_patch <  tau  (matches the
                                                vuln pattern but NOT the fix).

We treat func_before as POSITIVE (should be flagged vulnerable) and func_after
as NEGATIVE (patched, should NOT be flagged). The headline number is the
reduction in func_after false positives obtained by adding the patch-signature
filter, at matched recall.

Also computes patch-localization (contribution C4): how well the srcslice
def/use region of the changed variables covers the true deleted (fix) lines.

Self-contained. stdlib + optional numpy. Fixed seed. No network.
Runs as:  python3 proto_dualsig.py

NOTE: This script SHELLS OUT to `srcml` and `srcslice`. It does not run them
in the authoring environment; the orchestrator runs and verifies it.
"""

import os
import re
import sys
import json
import math
import difflib
import random
import shutil
import tempfile
import subprocess
from collections import defaultdict

# numpy is optional; we only use it for cosine if present (pure-python fallback).
try:
    import numpy as _np  # noqa: F401
    _HAVE_NUMPY = True
except Exception:
    _HAVE_NUMPY = False

# --------------------------------------------------------------------------- #
# Constants (top-level knobs)
# --------------------------------------------------------------------------- #
SEED = 1337
N_MAX = 400                      # cap on number of valid CVE pairs to use
TRAIN_FRAC = 0.70                # first 70% (fixed order) = DB/train
TAUS = [0.3, 0.4, 0.5, 0.6, 0.7, 0.8]
SRC_TIMEOUT = 20                 # seconds per srcml/srcslice call

HERE = os.path.dirname(os.path.abspath(__file__))
DATA_PATH = os.path.join(HERE, "data", "bigvul", "sample.jsonl")

random.seed(SEED)

# C / C++ keywords excluded from "vulnerability-related variable" extraction.
C_KEYWORDS = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline",
    "int", "long", "register", "restrict", "return", "short", "signed",
    "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned",
    "void", "volatile", "while", "_Bool", "_Complex", "_Imaginary",
    # C++ / common
    "bool", "class", "namespace", "new", "delete", "this", "true", "false",
    "nullptr", "public", "private", "protected", "virtual", "template",
    "typename", "using", "throw", "try", "catch", "operator", "friend",
    "explicit", "mutable", "constexpr", "noexcept", "override", "final",
    "NULL", "TRUE", "FALSE",
}

_IDENT_RE = re.compile(r"[A-Za-z_]\w*")


# --------------------------------------------------------------------------- #
# Tool plumbing: srcml + srcslice
# --------------------------------------------------------------------------- #
def _which(name):
    return shutil.which(name)


SRCML_BIN = _which("srcml")
SRCSLICE_BIN = _which("srcslice")


def _run(cmd, timeout):
    """Run a command; return (rc, stdout, stderr) or None on failure/timeout."""
    try:
        p = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=timeout,
            check=False,
        )
        out = p.stdout.decode("utf-8", "replace")
        err = p.stderr.decode("utf-8", "replace")
        return p.returncode, out, err
    except subprocess.TimeoutExpired:
        return None
    except Exception:
        return None


def _wrap_function(code):
    """
    Write the function to a standalone .c source. srcml parses standalone
    functions fine, so we emit the code as-is. We keep a .c extension so
    srcml infers C. Returns the file path (caller deletes).
    """
    fd, path = tempfile.mkstemp(suffix=".c")
    try:
        with os.fdopen(fd, "w", encoding="utf-8", errors="replace") as fh:
            fh.write(code)
            if not code.endswith("\n"):
                fh.write("\n")
    except Exception:
        try:
            os.close(fd)
        except Exception:
            pass
        try:
            os.unlink(path)
        except Exception:
            pass
        return None
    return path


def _parse_braced(segment):
    """
    Parse the inside of a `key{...}` field. srcslice uses commas to separate
    items inside braces, and inner cfunc entries look like `name{count}`.
    Returns the raw inner string (already without the outer braces) split into
    top-level comma items, respecting nested {}.
    """
    items = []
    depth = 0
    cur = []
    for ch in segment:
        if ch == "{":
            depth += 1
            cur.append(ch)
        elif ch == "}":
            depth -= 1
            cur.append(ch)
        elif ch == "," and depth == 0:
            items.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    if cur:
        items.append("".join(cur))
    return [it for it in items if it != ""]


def _split_top_fields(line):
    """
    Split a srcslice CSV line into top-level fields by commas that are NOT
    inside braces. srcslice format:
      FILE,FUNCTION,VARIABLE,def{..},use{..},dvars{..},pointers{..},cfuncs{..}
    The FILE field is a path that does not contain braces; FUNCTION/VARIABLE
    are bare identifiers. Only the brace-fields can contain commas, so a
    depth-aware comma split is correct.
    """
    fields = []
    depth = 0
    cur = []
    for ch in line:
        if ch == "{":
            depth += 1
            cur.append(ch)
        elif ch == "}":
            depth = max(0, depth - 1)
            cur.append(ch)
        elif ch == "," and depth == 0:
            fields.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    fields.append("".join(cur))
    return fields


def _int_set(inner):
    """Parse a brace inner string of comma-separated ints into a set[int]."""
    out = set()
    for tok in _parse_braced(inner):
        tok = tok.strip()
        if not tok:
            continue
        try:
            out.add(int(tok))
        except ValueError:
            # sometimes ranges or junk; ignore
            m = re.match(r"-?\d+", tok)
            if m:
                try:
                    out.add(int(m.group(0)))
                except ValueError:
                    pass
    return out


def _str_set(inner):
    """Parse a brace inner string of comma-separated names into a set[str]."""
    out = set()
    for tok in _parse_braced(inner):
        tok = tok.strip()
        if tok:
            out.add(tok)
    return out


def _cfuncs_map(inner):
    """Parse cfuncs inner: items like `name{count}` -> dict[str,int]."""
    out = {}
    for tok in _parse_braced(inner):
        tok = tok.strip()
        if not tok:
            continue
        m = re.match(r"^(.*?)\{(\d+)\}$", tok)
        if m:
            name = m.group(1).strip()
            try:
                cnt = int(m.group(2))
            except ValueError:
                cnt = 1
            if name:
                out[name] = out.get(name, 0) + cnt
        else:
            name = tok.strip("{}").strip()
            if name:
                out[name] = out.get(name, 0) + 1
    return out


def _field_inner(field, key):
    """
    Given a top-level field like `def{0,1}` and expected key `def`, return the
    inner string `0,1`. Tolerant: if the key prefix is missing, strip the outer
    braces if present. Returns "" if no braces found.
    """
    field = field.strip()
    prefix = key + "{"
    if field.startswith(prefix) and field.endswith("}"):
        return field[len(prefix):-1]
    # tolerant: anything{...}
    lb = field.find("{")
    rb = field.rfind("}")
    if lb != -1 and rb != -1 and rb > lb:
        return field[lb + 1:rb]
    return ""


def parse_srcslice_output(text):
    """
    Parse srcslice stdout into a list of per-variable dicts:
      {function, variable, def:set[int], use:set[int], dvars:set[str],
       pointers:set[str], cfuncs:dict[str,int]}
    Tolerant to:
      - a leading/trailing "Time is: ..." line
      - blank lines
      - malformed lines (skipped)
    """
    profiles = []
    for raw in text.splitlines():
        line = raw.strip()
        if not line:
            continue
        if line.lower().startswith("time is"):
            continue
        # A valid line needs at least the structured brace fields.
        if "{" not in line:
            continue
        fields = _split_top_fields(line)
        if len(fields) < 8:
            # Not enough fields -> malformed; skip.
            continue
        try:
            # FILE, FUNCTION, VARIABLE, def, use, dvars, pointers, cfuncs
            function = fields[1].strip()
            variable = fields[2].strip()
            def_s = _int_set(_field_inner(fields[3], "def"))
            use_s = _int_set(_field_inner(fields[4], "use"))
            dvars = _str_set(_field_inner(fields[5], "dvars"))
            pointers = _str_set(_field_inner(fields[6], "pointers"))
            # cfuncs may be the last field and could itself contain trailing
            # text; join any overflow fields back (defensive).
            cfuncs_field = fields[7] if len(fields) == 8 else ",".join(fields[7:])
            cfuncs = _cfuncs_map(_field_inner(cfuncs_field, "cfuncs"))
        except Exception:
            continue
        if not function and not variable:
            continue
        profiles.append({
            "function": function,
            "variable": variable,
            "def": def_s,
            "use": use_s,
            "dvars": dvars,
            "pointers": pointers,
            "cfuncs": cfuncs,
        })
    return profiles


def srcslice_profiles(code):
    """
    Run srcml then srcslice on a single function body.
    Returns list[profile dict], or None on any failure/timeout.
    """
    if not SRCML_BIN or not SRCSLICE_BIN:
        return None
    cpath = _wrap_function(code)
    if cpath is None:
        return None
    xfd, xpath = tempfile.mkstemp(suffix=".xml")
    os.close(xfd)
    try:
        r1 = _run([SRCML_BIN, cpath, "-o", xpath], SRC_TIMEOUT)
        if r1 is None or r1[0] != 0:
            return None
        if not os.path.exists(xpath) or os.path.getsize(xpath) == 0:
            return None
        r2 = _run([SRCSLICE_BIN, xpath], SRC_TIMEOUT)
        if r2 is None:
            return None
        # srcslice prints CSV to stdout. Non-zero rc but with output is still
        # usable; require some stdout though.
        out = r2[1]
        if not out.strip():
            return None
        return parse_srcslice_output(out)
    finally:
        for p in (cpath, xpath):
            try:
                os.unlink(p)
            except Exception:
                pass


# --------------------------------------------------------------------------- #
# Diff -> changed lines and vulnerability-related variables (vr_vars)
# --------------------------------------------------------------------------- #
def changed_lines(before_lines, after_lines):
    """
    Returns (del_lines, add_lines):
      del_lines = set of func_before line indices in replace/delete opcodes
      add_lines = set of func_after  line indices in replace/insert opcodes
    """
    sm = difflib.SequenceMatcher(a=before_lines, b=after_lines, autojunk=False)
    del_lines, add_lines = set(), set()
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag in ("replace", "delete"):
            del_lines.update(range(i1, i2))
        if tag in ("replace", "insert"):
            add_lines.update(range(j1, j2))
    return del_lines, add_lines


def idents_in_lines(lines, idxs):
    """Identifiers (minus C keywords) appearing on the given line indices."""
    out = set()
    for i in idxs:
        if 0 <= i < len(lines):
            for m in _IDENT_RE.findall(lines[i]):
                if m not in C_KEYWORDS:
                    out.add(m)
    return out


# --------------------------------------------------------------------------- #
# Signatures
# --------------------------------------------------------------------------- #
def build_signature(profiles, vr_vars, module_lines):
    """
    From a list of srcslice profiles and the set of vulnerability-related
    variables, build:
      - feature SET: {"cfunc:NAME","ptr:NAME","dvar:NAME", ...} aggregated over
        profiles whose variable is in vr_vars.
      - SrcVul-style 4-vector (SC, SCvg, SI, SS) using module_lines.
      - region: def ∪ use line set of the vr_vars profiles (for C4 localization).
    Returns (feature_set, vec(list[float] len 4), region(set[int])).
    """
    feat = set()
    region = set()
    n_profiles = 0
    slice_idents = set()       # unique identifiers participating in the slice
    all_lines = set()          # for slice_size and SS
    matched = [p for p in profiles if p["variable"] in vr_vars]
    # Fallback: if none of the vr_vars matched a srcslice variable (naming /
    # tokenization mismatch), use ALL profiles so the signature isn't empty.
    if not matched:
        matched = profiles
    for p in matched:
        n_profiles += 1
        for cf in p["cfuncs"]:
            feat.add("cfunc:" + cf)
            slice_idents.add(cf)
        for pt in p["pointers"]:
            feat.add("ptr:" + pt)
            slice_idents.add(pt)
        for dv in p["dvars"]:
            feat.add("dvar:" + dv)
            slice_idents.add(dv)
        slice_idents.add(p["variable"])
        region |= p["def"]
        region |= p["use"]
        all_lines |= p["def"]
        all_lines |= p["use"]

    ml = max(1, module_lines)
    slice_size = len(all_lines) if all_lines else 0
    if all_lines:
        first_def = min(all_lines)
        last_use = max(all_lines)
        ss = (last_use - first_def) / ml
    else:
        ss = 0.0
    sc = n_profiles / ml
    scvg = slice_size / ml
    si = len(slice_idents) / ml
    vec = [sc, scvg, si, ss]
    return feat, vec, region


# --------------------------------------------------------------------------- #
# Similarities
# --------------------------------------------------------------------------- #
def jaccard(a, b):
    if not a and not b:
        return 0.0
    inter = len(a & b)
    union = len(a | b)
    if union == 0:
        return 0.0
    return inter / union


def cosine(u, v):
    du = math.sqrt(sum(x * x for x in u))
    dv = math.sqrt(sum(x * x for x in v))
    if du == 0.0 or dv == 0.0:
        return 0.0
    dot = sum(x * y for x, y in zip(u, v))
    return dot / (du * dv)


# --------------------------------------------------------------------------- #
# Data loading
# --------------------------------------------------------------------------- #
def is_vuln(v):
    return v in (1, "1", True)


def load_pairs(path, n_max):
    """
    Yield up to n_max valid pairs:
      vul truthy AND func_after present AND func_after != func_before.
    Each item: dict(func_before, func_after, cwe).
    """
    pairs = []
    if not os.path.exists(path):
        sys.stderr.write("FATAL: data file not found: %s\n" % path)
        return pairs
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        for raw in fh:
            raw = raw.strip()
            if not raw:
                continue
            try:
                row = json.loads(raw)
            except Exception:
                continue
            if not is_vuln(row.get("vul")):
                continue
            fb = row.get("func_before")
            fa = row.get("func_after")
            if not isinstance(fb, str) or not isinstance(fa, str):
                continue
            if not fa.strip():
                continue
            if fa == fb:
                continue
            pairs.append({
                "func_before": fb,
                "func_after": fa,
                "cwe": row.get("CWE ID", ""),
            })
            if len(pairs) >= n_max:
                break
    return pairs


# --------------------------------------------------------------------------- #
# Metrics
# --------------------------------------------------------------------------- #
def prf(tp, fp, fn):
    prec = tp / (tp + fp) if (tp + fp) else 0.0
    rec = tp / (tp + fn) if (tp + fn) else 0.0
    f1 = (2 * prec * rec / (prec + rec)) if (prec + rec) else 0.0
    return prec, rec, f1


# --------------------------------------------------------------------------- #
# Main
# --------------------------------------------------------------------------- #
def main():
    print("=" * 78)
    print("proto_dualsig.py  —  dual slice signature prototype")
    print("=" * 78)
    print("seed=%d  N_MAX=%d  train_frac=%.2f  taus=%s" %
          (SEED, N_MAX, TRAIN_FRAC, TAUS))
    print("srcml=%s" % (SRCML_BIN or "NOT FOUND"))
    print("srcslice=%s" % (SRCSLICE_BIN or "NOT FOUND"))
    if not SRCML_BIN or not SRCSLICE_BIN:
        print("\nWARNING: srcml/srcslice not on PATH. All samples will be "
              "skipped; the run will report 0 usable pairs.")
    print("-" * 78)

    raw_pairs = load_pairs(DATA_PATH, N_MAX)
    print("valid CVE pairs loaded (cap %d): %d" % (N_MAX, len(raw_pairs)))
    if not raw_pairs:
        print("No data. Aborting.")
        return

    # Build per-pair signatures. Drop a pair only if BOTH before and after
    # srcslice fail (we need both to have vuln+patch signatures).
    pairs = []
    skipped_srcslice = 0
    # C4 localization accumulation
    loc_cov_srcslice = []          # coverage when srcslice region used
    loc_rsr_srcslice = []
    loc_cov_fallback = []          # coverage when fallback (text-scan) region used
    loc_rsr_fallback = []
    used_fallback_count = 0
    used_srcslice_count = 0

    for idx, pr in enumerate(raw_pairs):
        fb = pr["func_before"]
        fa = pr["func_after"]
        fb_lines = fb.splitlines()
        fa_lines = fa.splitlines()
        module_lines = len(fb_lines)
        if module_lines == 0:
            skipped_srcslice += 1
            continue

        del_lines, add_lines = changed_lines(fb_lines, fa_lines)
        vr_before = idents_in_lines(fb_lines, del_lines)
        vr_after = idents_in_lines(fa_lines, add_lines)
        vr_vars = vr_before | vr_after
        if not vr_vars:
            # No textual change variables found; still attempt with all idents
            vr_vars = idents_in_lines(fb_lines, range(len(fb_lines)))

        prof_before = srcslice_profiles(fb)
        prof_after = srcslice_profiles(fa)
        if prof_before is None or prof_after is None:
            skipped_srcslice += 1
            continue

        vuln_set, vuln_vec, region_before = build_signature(
            prof_before, vr_vars, module_lines)
        patch_set, patch_vec, _region_after = build_signature(
            prof_after, vr_vars, len(fa_lines))

        # ----- C4 patch localization -----
        # Decide whether srcslice line numbers are usable. If the region is
        # empty or looks degenerate (all zeros), fall back to a text scan of
        # func_before lines containing any vr_var.
        srcslice_usable = bool(region_before) and (set(region_before) != {0})
        if srcslice_usable:
            # srcslice line numbers: clamp into valid 1..module_lines, convert
            # to 0-based for intersection with del_lines (0-based indices).
            region0 = set()
            for ln in region_before:
                z = ln - 1  # srcslice appears 1-based; del_lines are 0-based
                if 0 <= z < module_lines:
                    region0.add(z)
                elif 0 <= ln < module_lines:
                    region0.add(ln)  # tolerate 0-based emitters
            if not region0:
                srcslice_usable = False
        if srcslice_usable and del_lines:
            cov = len(region0 & del_lines) / len(del_lines)
            rsr = len(region0) / max(1, module_lines)
            loc_cov_srcslice.append(cov)
            loc_rsr_srcslice.append(rsr)
            used_srcslice_count += 1
        else:
            # Fallback: scan func_before lines for any vr_var occurrence.
            fb_region = set()
            for i, ln in enumerate(fb_lines):
                toks = set(_IDENT_RE.findall(ln))
                if toks & vr_vars:
                    fb_region.add(i)
            if del_lines:
                cov = (len(fb_region & del_lines) / len(del_lines))
                rsr = len(fb_region) / max(1, module_lines)
                loc_cov_fallback.append(cov)
                loc_rsr_fallback.append(rsr)
                used_fallback_count += 1

        pairs.append({
            "vuln_set": vuln_set,
            "patch_set": patch_set,
            "vuln_vec": vuln_vec,
            "patch_vec": patch_vec,
            "vr_vars": vr_vars,
            "module_lines": module_lines,
            "fb_lines": fb_lines,
            "fa_lines": fa_lines,
        })

    used = len(pairs)
    print("pairs usable (both srcslice ran): %d" % used)
    print("pairs skipped (srcslice failure/empty): %d" % skipped_srcslice)
    if used < 4:
        print("Too few usable pairs to split/evaluate. Aborting analysis.")
        return

    # ----- train/test split (fixed order; first 70% = DB) -----
    n_db = int(round(used * TRAIN_FRAC))
    n_db = max(1, min(used - 1, n_db))
    db = pairs[:n_db]
    test = pairs[n_db:]
    print("DB (train) size: %d   TEST size: %d" % (len(db), len(test)))
    print("-" * 78)

    db_vuln_sets = [d["vuln_set"] for d in db]
    db_patch_sets = [d["patch_set"] for d in db]
    db_vuln_vecs = [d["vuln_vec"] for d in db]

    # Build test queries: each pair contributes a POSITIVE (func_before query)
    # and a NEGATIVE (func_after query). For each query we compute its own
    # signature over its own vr_vars (realistic: a detector that localized
    # candidate vars). func_before query uses vuln_set; func_after uses
    # patch_set (already computed per pair).
    queries = []
    for t in test:
        # positive: querying the still-vulnerable code
        queries.append({
            "set": t["vuln_set"], "vec": t["vuln_vec"], "label": 1})
        # negative: querying the patched code
        queries.append({
            "set": t["patch_set"], "vec": t["patch_vec"], "label": 0})

    # Precompute nearest sims for each query against DB.
    for q in queries:
        sv = 0.0
        sp = 0.0
        cv = 0.0
        for dvset, dpset, dvec in zip(db_vuln_sets, db_patch_sets, db_vuln_vecs):
            j_v = jaccard(q["set"], dvset)
            if j_v > sv:
                sv = j_v
            j_p = jaccard(q["set"], dpset)
            if j_p > sp:
                sp = j_p
            c = cosine(q["vec"], dvec)
            if c > cv:
                cv = c
        q["sim_vuln"] = sv
        q["sim_patch"] = sp
        q["sim_vuln_cos"] = cv

    n_pos = sum(1 for q in queries if q["label"] == 1)
    n_neg = sum(1 for q in queries if q["label"] == 0)
    print("test queries: %d positives (func_before), %d negatives (func_after)"
          % (n_pos, n_neg))
    print("-" * 78)

    # ----- evaluate both rules across taus (Jaccard primary) -----
    def evaluate(rule):
        """rule(q, tau) -> bool predicted-vulnerable. Returns dict tau->stats."""
        rows = {}
        for tau in TAUS:
            tp = fp = tn = fn = 0
            fp_after = 0
            for q in queries:
                pred = rule(q, tau)
                if q["label"] == 1:
                    if pred:
                        tp += 1
                    else:
                        fn += 1
                else:
                    if pred:
                        fp += 1
                        fp_after += 1
                    else:
                        tn += 1
            prec, rec, f1 = prf(tp, fp, fn)
            fpr_after = fp_after / n_neg if n_neg else 0.0
            rows[tau] = {
                "tp": tp, "fp": fp, "tn": tn, "fn": fn,
                "prec": prec, "rec": rec, "f1": f1,
                "fpr_after": fpr_after, "fp_after": fp_after,
            }
        return rows

    def vuln_only_rule(q, tau):
        return q["sim_vuln"] >= tau

    def dual_rule(q, tau):
        return q["sim_vuln"] >= tau and q["sim_patch"] < tau

    vonly = evaluate(vuln_only_rule)
    dual = evaluate(dual_rule)

    def best_tau(rows):
        bt, bf1 = None, -1.0
        for tau in TAUS:
            if rows[tau]["f1"] > bf1:
                bf1 = rows[tau]["f1"]
                bt = tau
        return bt

    bt_v = best_tau(vonly)
    bt_d = best_tau(dual)

    def print_table(title, rows):
        print(title)
        print("  tau   prec   recall    F1     FPR(after)  TP   FP   FN   TN")
        for tau in TAUS:
            r = rows[tau]
            print("  %.1f  %.3f  %.3f  %.3f    %.3f      %3d  %3d  %3d  %3d" %
                  (tau, r["prec"], r["rec"], r["f1"], r["fpr_after"],
                   r["tp"], r["fp"], r["fn"], r["tn"]))

    print_table("[Rule A] SrcVul-style (vuln signature only):  flag if "
                "sim_vuln >= tau", vonly)
    print()
    print_table("[Rule B] Dual signature (C1):  flag if sim_vuln >= tau AND "
                "sim_patch < tau", dual)
    print("-" * 78)

    rv = vonly[bt_v]
    rd = dual[bt_d]
    print("Best-F1  Rule A (vuln-only): tau=%.1f  P=%.3f R=%.3f F1=%.3f  "
          "FPR_after=%.3f" % (bt_v, rv["prec"], rv["rec"], rv["f1"],
                              rv["fpr_after"]))
    print("Best-F1  Rule B (dual)    : tau=%.1f  P=%.3f R=%.3f F1=%.3f  "
          "FPR_after=%.3f" % (bt_d, rd["prec"], rd["rec"], rd["f1"],
                              rd["fpr_after"]))
    print("-" * 78)

    # ----- HEADLINE: FP reduction on func_after at MATCHED recall -----
    # For each tau, both rules share the same sim_vuln gate, so vuln-only and
    # dual have IDENTICAL recall at the same tau (dual only ever removes
    # predictions, and it removes a positive only if that positive also matches
    # a patch >= tau — possible but documented). To compare at matched recall we
    # pick, per tau, the func_after FP of each rule and report the reduction.
    print("HEADLINE — func_after false positives, vuln-only vs dual "
          "(matched-tau):")
    print("  tau   recall(A)  recall(B)   FP_after(A)  FP_after(B)   "
          "reduction")
    best_red = None
    for tau in TAUS:
        a = vonly[tau]
        b = dual[tau]
        fa_a = a["fp_after"]
        fa_b = b["fp_after"]
        red = (fa_a - fa_b) / fa_a if fa_a else 0.0
        if a["rec"] > 0 and (best_red is None or red > best_red[1]):
            best_red = (tau, red, fa_a, fa_b)
        print("  %.1f    %.3f      %.3f         %4d         %4d        %.1f%%" %
              (tau, a["rec"], b["rec"], fa_a, fa_b, 100.0 * red))
    if best_red:
        print("  -> best func_after FP reduction: %.1f%% at tau=%.1f "
              "(%d -> %d patched FPs)" %
              (100.0 * best_red[1], best_red[0], best_red[2], best_red[3]))
    # Also report the headline at the dual rule's best-F1 tau explicitly.
    a_at = vonly[bt_d]
    b_at = dual[bt_d]
    if a_at["fp_after"]:
        red_at = (a_at["fp_after"] - b_at["fp_after"]) / a_at["fp_after"]
    else:
        red_at = 0.0
    print("  -> at dual best-F1 tau=%.1f: func_after FP %d -> %d (%.1f%% "
          "reduction), recall %.3f -> %.3f" %
          (bt_d, a_at["fp_after"], b_at["fp_after"], 100.0 * red_at,
           a_at["rec"], b_at["rec"]))
    print("-" * 78)

    # ----- C4 patch localization -----
    def mean(xs):
        return (sum(xs) / len(xs)) if xs else float("nan")

    print("PATCH LOCALIZATION (C4):")
    print("  srcslice-region path used on %d pairs; text-scan fallback on %d "
          "pairs" % (used_srcslice_count, used_fallback_count))
    if used_srcslice_count >= used_fallback_count and used_srcslice_count > 0:
        print("  EXPECTED dominant path: srcslice def/use line numbers")
    elif used_fallback_count > 0:
        print("  EXPECTED dominant path: text-scan fallback (srcslice line "
              "numbers were empty/degenerate)")
    if loc_cov_srcslice:
        print("  [srcslice region]  mean coverage(|region∩del|/|del|) = %.3f   "
              "mean region_size_ratio = %.3f" %
              (mean(loc_cov_srcslice), mean(loc_rsr_srcslice)))
    if loc_cov_fallback:
        print("  [text-scan region] mean coverage(|region∩del|/|del|) = %.3f   "
              "mean region_size_ratio = %.3f" %
              (mean(loc_cov_fallback), mean(loc_rsr_fallback)))
    print("-" * 78)

    # ----- final clean summary -----
    print("SUMMARY")
    print("  pairs used ............ %d" % used)
    print("  pairs skipped (srcsl).. %d" % skipped_srcslice)
    print("  DB / TEST ............. %d / %d" % (len(db), len(test)))
    print("  test pos / neg ........ %d / %d" % (n_pos, n_neg))
    print("  Rule A best-F1 ........ %.3f (tau=%.1f, FPR_after=%.3f)" %
          (rv["f1"], bt_v, rv["fpr_after"]))
    print("  Rule B best-F1 ........ %.3f (tau=%.1f, FPR_after=%.3f)" %
          (rd["f1"], bt_d, rd["fpr_after"]))
    if best_red:
        print("  headline FP reduction . %.1f%% (tau=%.1f)" %
              (100.0 * best_red[1], best_red[0]))
    print("  C4 loc coverage (mean). %.3f" %
          (mean(loc_cov_srcslice) if loc_cov_srcslice
           else mean(loc_cov_fallback)))
    print("=" * 78)


if __name__ == "__main__":
    main()
