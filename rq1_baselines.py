"""
rq1_baselines.py
-----------------
RQ1 extension (SANER revision, Phase B1 of Docs/paper_revision_brief.md):

  1. Five size-/RSR-matched baselines for the sink-criterion backward slice
     (random-lines, random-contiguous-window, sink-window +/-k,
     variable-mention, tail heuristic), each compared against the slice via
     a paired Wilcoxon signed-rank test (per-CVE coverage; CVEs are
     independent units, so the pairing is valid here) plus Cliff's delta.
  2. An insertion-anchor extension of ground truth, covering the 106/400
     pairs RQ1 previously excluded for having an empty deleted-line set
     (pure-addition "fixes"): for each insert-only diff hunk, the anchor is
     the func_before line immediately before (and, if present, after) the
     insertion point. A slice/baseline "hits" an insertion if any anchor
     line falls inside it.
  3. lift = slice coverage / (random-lines expected coverage at the same
     RSR), which is the correct, well-defined replacement for the paper's
     previous (arithmetically wrong) "~2x concentration" claim.
  4. An AUTOMATED, rule-based miss taxonomy over the fix lines/anchors the
     slice does not cover. This is NOT manually verified by a second rater
     (out of scope for this revision pass) -- it is disclosed as an
     unvalidated heuristic, not a ground-truth label.

Never touches func_after at slice-computation time (ground truth is used
only for evaluation, after the slice/baseline regions are already fixed).
"""
import os
import sys
import json
import random
import re
import difflib

import numpy as np
from scipy.stats import wilcoxon

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

from consolidated_study import vuln_pairs, load_all_rows, RQ1_CAP  # noqa: E402
from ast_slicer import (  # noqa: E402
    semantic_slice_indices,
    _find_function_body,
    _gather_statements,
    _is_criterion,
    _PARSER,
)

DATA_PATH = os.path.join(HERE, "data", "bigvul", "sample.jsonl")
RESULTS_PATH = os.path.join(HERE, "results", "rq1_baselines.json")
SEED = 1337
N_DRAWS = 100

_IDENT_RE = re.compile(r"[A-Za-z_]\w*")
_KEYWORDS = {
    "if", "for", "while", "return", "const", "void", "int", "char",
    "unsigned", "long", "struct", "static", "else", "do", "switch", "case",
    "break", "continue", "sizeof", "NULL", "true", "false", "short", "float",
    "double", "goto", "typedef", "enum", "union", "register", "volatile",
}


def changed_lines(before_lines, after_lines):
    sm = difflib.SequenceMatcher(a=before_lines, b=after_lines, autojunk=False)
    dels, adds = set(), set()
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag in ("replace", "delete"):
            dels.update(range(i1, i2))
        if tag in ("replace", "insert"):
            adds.update(range(j1, j2))
    return dels, adds


def insertion_anchors(fb_lines, fa_lines):
    """For pure insert-only diff hunks, anchor = the func_before line index
    immediately before (and after, if present) the insertion point. Returns
    (anchor_set, list of (anchor_idx, inserted_text_lines))."""
    sm = difflib.SequenceMatcher(a=fb_lines, b=fa_lines, autojunk=False)
    anchors = set()
    ins_blocks = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "insert":
            before_anchor = max(0, i1 - 1)
            anchors.add(before_anchor)
            if i1 < len(fb_lines):
                anchors.add(i1)
            ins_blocks.append((before_anchor, fa_lines[j1:j2]))
    return anchors, ins_blocks


def slice_and_criteria(fb):
    S = set()
    try:
        S = set(int(i) for i in (semantic_slice_indices(fb) or set()))
    except Exception:
        pass
    crit_lines, sink_lines = set(), set()
    try:
        code_bytes = fb.encode("utf-8", "replace")
        tree = _PARSER.parse(code_bytes)
        root = tree.root_node
        body, func_node = _find_function_body(root)
        if body is not None:
            stmts, _ = _gather_statements(body, code_bytes)
            crit_stmts = [s for s in stmts if _is_criterion(s.node, code_bytes)]
            for s in crit_stmts:
                crit_lines.update(range(s.node.start_point[0], s.node.end_point[0] + 1))
                sink_lines.add(s.node.start_point[0])
    except Exception:
        pass
    return S, crit_lines, sink_lines


# --------------------------------------------------------------------------- #
# Baselines
# --------------------------------------------------------------------------- #
def random_lines_coverage(n, k, target_del, target_ins, rng):
    if k <= 0 or n <= 0:
        return 0.0, 0.0
    idxs = list(range(n))
    del_hits, ins_hits = [], []
    for _ in range(N_DRAWS):
        sample = set(rng.sample(idxs, min(k, n)))
        if target_del:
            del_hits.append(len(sample & target_del) / len(target_del))
        if target_ins:
            ins_hits.append(1.0 if (sample & target_ins) else 0.0)
    cov_del = sum(del_hits) / len(del_hits) if del_hits else None
    hit_ins = sum(ins_hits) / len(ins_hits) if ins_hits else None
    return cov_del, hit_ins


def random_window_coverage(n, k, target_del, target_ins, rng):
    if k <= 0 or n <= 0:
        return 0.0, 0.0
    k = min(k, n)
    del_hits, ins_hits = [], []
    for _ in range(N_DRAWS):
        start = rng.randint(0, n - k)
        window = set(range(start, start + k))
        if target_del:
            del_hits.append(len(window & target_del) / len(target_del))
        if target_ins:
            ins_hits.append(1.0 if (window & target_ins) else 0.0)
    cov_del = sum(del_hits) / len(del_hits) if del_hits else None
    hit_ins = sum(ins_hits) / len(ins_hits) if ins_hits else None
    return cov_del, hit_ins


def sink_window(n, k, sink_lines):
    window = set()
    for s in sink_lines:
        for d in range(-k, k + 1):
            li = s + d
            if 0 <= li < n:
                window.add(li)
    return window


def variable_mention_region(fb_lines, crit_lines):
    vc = set()
    for li in crit_lines:
        if 0 <= li < len(fb_lines):
            vc.update(_IDENT_RE.findall(fb_lines[li]))
    vc = {v for v in vc if v not in _KEYWORDS and len(v) > 1}
    if not vc:
        return set(), vc
    region = set()
    for i, line in enumerate(fb_lines):
        if set(_IDENT_RE.findall(line)) & vc:
            region.add(i)
    return region, vc


def tail_region(n, k):
    k = min(k, n)
    return set(range(max(0, n - k), n))


# --------------------------------------------------------------------------- #
# Miss taxonomy (automated, rule-based; NOT manually verified)
# --------------------------------------------------------------------------- #
def classify_deletion_miss(fb_lines, line_idx):
    text = fb_lines[line_idx] if 0 <= line_idx < len(fb_lines) else ""
    stripped = text.strip()
    if not stripped or stripped in ("{", "}") or stripped.startswith(("//", "/*", "*")):
        return "cosmetic"
    if re.match(r"^\s*(if|while|for|switch|else)\b", stripped):
        return "control_structure_change"
    if re.match(
        r"^(unsigned\s+|signed\s+|const\s+|static\s+)*"
        r"(int|char|long|short|float|double|size_t|void|struct\s+\w+|\w+_t)\b.*[;=]",
        stripped,
    ):
        return "declaration_type_change"
    return "outside_dependence_cone"


def classify_insertion_miss(inserted_lines):
    joined = " ".join(inserted_lines)
    if re.search(r"\bif\s*\(", joined) or re.search(r"\breturn\b", joined) or re.search(r"\bgoto\b", joined):
        return "absent_guard"
    if re.search(r"\b(free|close|unlock|release|fclose|munmap)\b", joined):
        return "error_handling_cleanup"
    return "other_insertion"


# --------------------------------------------------------------------------- #
# Main
# --------------------------------------------------------------------------- #
def main():
    rng = random.Random(SEED)
    rows = load_all_rows(DATA_PATH)
    pairs = vuln_pairs(rows, RQ1_CAP)
    print("candidates:", len(pairs))

    per_pair = []
    for pr in pairs:
        fb, fa = pr["func_before"], pr["func_after"]
        fb_lines, fa_lines = fb.splitlines(), fa.splitlines()
        n = len(fb_lines)
        if n == 0:
            continue
        del_lines, _add = changed_lines(fb_lines, fa_lines)
        anchors, ins_blocks = insertion_anchors(fb_lines, fa_lines)

        is_insertion_only = (not del_lines) and bool(anchors)
        if not del_lines and not anchors:
            continue  # metadata-only / identical, already excluded upstream

        S, crit_lines, sink_lines = slice_and_criteria(fb)
        if not S:
            continue
        rsr = len(S) / n
        k = max(1, len(S))

        rec = {
            "cwe": pr.get("cwe", ""),
            "n": n,
            "rsr": rsr,
            "is_insertion_only": is_insertion_only,
            "n_del": len(del_lines),
            "n_anchors": len(anchors),
        }

        if del_lines:
            rec["slice_cov_del"] = len(S & del_lines) / len(del_lines)
        if anchors:
            rec["slice_hit_ins"] = 1.0 if (S & anchors) else 0.0

        # combined per-pair "localized" indicator over ALL 400 pairs
        if del_lines and anchors:
            rec["combined_hit"] = 1.0 if ((S & del_lines) or (S & anchors)) else 0.0
        elif del_lines:
            rec["combined_hit"] = 1.0 if (S & del_lines) else 0.0
        else:
            rec["combined_hit"] = rec.get("slice_hit_ins")

        # --- baselines ---
        rl_cov, rl_ins = random_lines_coverage(n, k, del_lines, anchors, rng)
        rw_cov, rw_ins = random_window_coverage(n, k, del_lines, anchors, rng)

        sw_region = sink_window(n, max(1, k // max(1, 2 * len(sink_lines) or 1)), sink_lines) if sink_lines else set()
        # size-calibrate sink-window by growing +/-k until region size >= slice size
        if sink_lines:
            kk = 1
            sw_region = sink_window(n, kk, sink_lines)
            while len(sw_region) < k and kk < n:
                kk += 1
                sw_region = sink_window(n, kk, sink_lines)
        vm_region, _vc = variable_mention_region(fb_lines, crit_lines)
        tail_reg = tail_region(n, k)

        for bname, region in (
            ("sink_window", sw_region),
            ("variable_mention", vm_region),
            ("tail", tail_reg),
        ):
            if del_lines:
                rec["%s_cov_del" % bname] = (len(region & del_lines) / len(del_lines)) if region else 0.0
            if anchors:
                rec["%s_hit_ins" % bname] = (1.0 if (region & anchors) else 0.0)
            rec["%s_rsr" % bname] = (len(region) / n) if region else 0.0

        rec["random_lines_cov_del"] = rl_cov
        rec["random_lines_hit_ins"] = rl_ins
        rec["random_window_cov_del"] = rw_cov
        rec["random_window_hit_ins"] = rw_ins

        # --- miss taxonomy (automated) ---
        misses = []
        if del_lines:
            for li in sorted(del_lines - S):
                misses.append(classify_deletion_miss(fb_lines, li))
        if anchors and not (S & anchors):
            for _anchor_idx, ins_text in ins_blocks:
                misses.append(classify_insertion_miss(ins_text))
        rec["misses"] = misses

        per_pair.append(rec)

    print("pairs used:", len(per_pair))

    def _mean(xs):
        xs = [x for x in xs if x is not None]
        return sum(xs) / len(xs) if xs else None

    slice_cov_del = [r["slice_cov_del"] for r in per_pair if "slice_cov_del" in r]
    slice_hit_ins = [r["slice_hit_ins"] for r in per_pair if "slice_hit_ins" in r]
    combined = [r["combined_hit"] for r in per_pair if r.get("combined_hit") is not None]
    mean_rsr = _mean([r["rsr"] for r in per_pair])

    summary = {
        "n_pairs": len(per_pair),
        "n_deletion_pairs": len(slice_cov_del),
        "n_insertion_only_pairs": len(slice_hit_ins),
        "mean_rsr": mean_rsr,
        "slice": {
            "mean_coverage_deletion": _mean(slice_cov_del),
            "mean_hitrate_insertion": _mean(slice_hit_ins),
            "mean_combined": _mean(combined),
        },
        "baselines": {},
    }

    baseline_names = ["random_lines", "random_window", "sink_window", "variable_mention", "tail"]
    for bname in baseline_names:
        cov_key, ins_key = "%s_cov_del" % bname, "%s_hit_ins" % bname
        cov_vals = [r[cov_key] for r in per_pair if cov_key in r and r[cov_key] is not None]
        ins_vals = [r[ins_key] for r in per_pair if ins_key in r and r[ins_key] is not None]
        entry = {
            "mean_coverage_deletion": _mean(cov_vals),
            "mean_hitrate_insertion": _mean(ins_vals),
        }
        if bname in ("sink_window", "variable_mention"):
            rsr_vals = [r["%s_rsr" % bname] for r in per_pair if "%s_rsr" % bname in r]
            entry["mean_rsr"] = _mean(rsr_vals)

        # paired Wilcoxon + Cliff's delta on per-CVE deletion coverage
        paired_a, paired_b = [], []
        for r in per_pair:
            if "slice_cov_del" in r and cov_key in r and r[cov_key] is not None:
                paired_a.append(r["slice_cov_del"])
                paired_b.append(r[cov_key])
        if len(paired_a) >= 2 and any(abs(x - y) > 1e-12 for x, y in zip(paired_a, paired_b)):
            try:
                stat, p = wilcoxon(paired_a, paired_b)
                stat, p = float(stat), float(p)
            except Exception:
                stat, p = None, None
        else:
            stat, p = None, None

        def cliffs_delta(a, b):
            gt = sum(1 for x in a for y in b if x > y)
            lt = sum(1 for x in a for y in b if x < y)
            nn = len(a) * len(b)
            return (gt - lt) / nn if nn else 0.0

        entry["wilcoxon_stat"] = stat
        entry["wilcoxon_p"] = p
        entry["cliffs_delta"] = cliffs_delta(paired_a, paired_b) if paired_a else None
        entry["n_paired"] = len(paired_a)
        summary["baselines"][bname] = entry

    # lift = slice coverage / random-lines expected coverage at the same RSR
    rl_cov_mean = summary["baselines"]["random_lines"]["mean_coverage_deletion"]
    summary["lift_vs_random_lines"] = (
        (summary["slice"]["mean_coverage_deletion"] / rl_cov_mean) if rl_cov_mean else None
    )
    summary["concentration_ratio"] = (
        (summary["slice"]["mean_coverage_deletion"] / mean_rsr) if mean_rsr else None
    )

    # miss taxonomy aggregate
    from collections import Counter
    all_misses = Counter()
    for r in per_pair:
        all_misses.update(r["misses"])
    total_misses = sum(all_misses.values())
    taxonomy = {
        k: {"n": v, "pct": (100.0 * v / total_misses if total_misses else 0.0)}
        for k, v in all_misses.most_common()
    }
    summary["miss_taxonomy_automated"] = {
        "note": (
            "Rule-based, automated classification of fix lines/insertion anchors "
            "the slice does not cover. NOT manually verified by a second rater "
            "(out of scope for this revision pass) -- reported as an "
            "unvalidated heuristic, not a ground-truth label."
        ),
        "total_misses": total_misses,
        "categories": taxonomy,
    }

    os.makedirs(os.path.dirname(RESULTS_PATH), exist_ok=True)
    with open(RESULTS_PATH, "w") as fh:
        json.dump({"summary": summary, "per_pair": per_pair}, fh, indent=2)

    print(json.dumps(summary, indent=2, default=str))
    print("written to", RESULTS_PATH)


if __name__ == "__main__":
    main()
