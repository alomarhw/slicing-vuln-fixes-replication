"""
rq1_dependence_depth.py
-----------------------
Why does a variable-mention region cover as much of a fix as the dependence-traced slice?
Hypothesis: lexical proximity captures fix lines that are one dependence hop from a sink, and
dependence tracing only pays off on longer chains, which are rare in real fixes.

For every function, a breadth-first replay of the slicer (same helpers, same edges as
ast_slicer.semantic_slice_indices) records the minimum number of worklist hops at which each
statement enters the slice: sink statements are depth 0, a definition or enclosing predicate
pulled in by a depth-d statement is depth d+1. A line takes the minimum depth of the statements
spanning it. The replayed slice is checked against semantic_slice_indices.

Strata, fixed before any comparison was run: depth 0, 1, 2, >=3, and "outside" (deleted fix
lines not in the slice). For each stratum and each pair with at least one deleted line in it,
we compare the share of those lines covered by the slice and by the variable-mention region
(paired Wilcoxon, Cliff's delta, as in Table I). Pairs whose slice fell back to the whole
function (no parsable body) have no dependence depth and are counted separately.

Populations: the 294 main-sample deletion pairs, the 760 full-split deletion pairs, and, when
the repositories are available (~/tools/cve_repos), the 369 recent CVE-fix pairs.
Writes results/rq1_dependence_depth.json.
"""

import collections
import hashlib
import json
import os
import sys

import numpy as np
from scipy.stats import wilcoxon

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

import ast_slicer as A  # noqa: E402
from consolidated_study import vuln_pairs, load_all_rows, RQ1_CAP  # noqa: E402
from rq1_baselines import changed_lines, DATA_PATH, slice_and_criteria, variable_mention_region  # noqa: E402
from rq1_selective_sinks import ensure_full_jsonl, FULL_JSONL  # noqa: E402

OUT = os.path.join(HERE, "results", "rq1_dependence_depth.json")
REPOS = os.path.expanduser("~/tools/cve_repos")
STRATA = ("0", "1", "2", ">=3", "outside")


def line_depths(code):
    """Map 0-based line -> minimum dependence depth, or None if the slicer fell back."""
    code_bytes = code.encode("utf-8", "replace")
    root = A._PARSER.parse(code_bytes).root_node
    body, _ = A._find_function_body(root)
    if body is None:
        return None
    stmts, predicate_index = A._gather_statements(body, code_bytes)
    if not stmts:
        return None
    criteria = [s for s in stmts if A._is_criterion(s.node, code_bytes)] or [stmts[-1]]
    defs_by_var = collections.defaultdict(list)
    for s in stmts:
        for v in s.defs:
            defs_by_var[v].append(s)
    by_id = {s.sid: s for s in stmts}
    depth = {}
    queue = collections.deque()
    for c in criteria:
        if c.sid not in depth:
            depth[c.sid] = 0
            queue.append(c)
    while queue:                      # breadth first, so the first visit is the minimum depth
        s = queue.popleft()
        nxt = []
        for v in s.uses:
            definers = defs_by_var.get(v)
            if definers:
                before = [d for d in definers if d.start_byte <= s.start_byte]
                nxt.extend(before or definers)
        nxt.extend(by_id[p] for p in A._enclosing_predicates(s.node, predicate_index) if p in by_id)
        for d in nxt:
            if d.sid not in depth:
                depth[d.sid] = depth[s.sid] + 1
                queue.append(d)
    n = len(code.splitlines())
    out = {}
    for sid, dep in depth.items():
        s = by_id[sid]
        for li in range(s.start_line, s.end_line + 1):
            if 0 <= li < n:
                out[li] = min(dep, out.get(li, dep))
    return out


def stratum(dep):
    if dep is None:
        return "outside"
    return str(dep) if dep < 3 else ">=3"


def cliffs_delta(a, b):
    gt = sum(1 for x in a for y in b if x > y)
    lt = sum(1 for x in a for y in b if x < y)
    return (gt - lt) / (len(a) * len(b)) if a and b else None


def analyse(pairs):
    per_stratum = {k: {"slice": [], "vm": []} for k in STRATA}
    line_counts = collections.Counter()
    vm_hits = collections.Counter()
    n_pairs = n_fallback = n_mismatch = 0
    for p in pairs:
        fb = p["func_before"]
        fbl = fb.splitlines()
        dels, _ = changed_lines(fbl, p["func_after"].splitlines())
        if not fbl or not dels:
            continue
        n_pairs += 1
        depths = line_depths(fb)
        S, crit_lines, _ = slice_and_criteria(fb)
        if depths is None:
            n_fallback += 1
            continue
        if set(depths) != set(S):
            n_mismatch += 1           # replay must equal the paper's slice; counted, not hidden
        vm, _ = variable_mention_region(fbl, crit_lines)
        groups = collections.defaultdict(list)
        for d in dels:
            groups[stratum(depths.get(d))].append(d)
        for k, lines in groups.items():
            line_counts[k] += len(lines)
            vm_hits[k] += sum(1 for d in lines if d in vm)
            per_stratum[k]["slice"].append(sum(1 for d in lines if d in S) / len(lines))
            per_stratum[k]["vm"].append(sum(1 for d in lines if d in vm) / len(lines))
    total = sum(line_counts.values())
    covered = sum(line_counts[k] for k in STRATA if k != "outside")
    res = {"n_pairs": n_pairs, "n_fallback_whole_function": n_fallback, "n_replay_mismatch": n_mismatch,
           "n_deleted_lines": total, "n_deleted_lines_in_slice": covered, "strata": {}}
    for k in STRATA:
        a, b = per_stratum[k]["slice"], per_stratum[k]["vm"]
        entry = {"n_lines": line_counts[k],
                 "share_of_all_deleted_lines": line_counts[k] / total if total else None,
                 "share_of_covered_lines": (line_counts[k] / covered) if (covered and k != "outside") else None,
                 "n_pairs": len(a),
                 "slice_coverage": float(np.mean(a)) if a else None,
                 "vm_coverage": float(np.mean(b)) if b else None,
                 "vm_line_capture": vm_hits[k] / line_counts[k] if line_counts[k] else None}
        if len(a) >= 2 and any(abs(x - y) > 1e-12 for x, y in zip(a, b)):
            entry["wilcoxon_p"] = float(wilcoxon(a, b).pvalue)
        else:
            entry["wilcoxon_p"] = None
        entry["cliffs_delta"] = cliffs_delta(a, b)
        res["strata"][k] = entry
    return res


def recent_pairs():
    if not os.path.isdir(REPOS):
        return None
    import rq1_recent_cves as rc  # noqa: PLC0415
    pairs, seen = [], set()
    for repo in sorted((os.path.join(REPOS, d) for d in os.listdir(REPOS)), key=os.path.basename):
        for p in rc.mine(repo):
            h = hashlib.sha1((p["func_before"] + "\0" + p["func_after"]).encode("utf-8", "replace")).hexdigest()
            if h not in seen:
                seen.add(h)
                pairs.append(p)
    pairs.sort(key=lambda p: (p["repo"], p["commit"], p["file"], p["function"]))
    return pairs


def main():
    ensure_full_jsonl()
    out = {"strata_definition": "minimum worklist hops from a sink statement; fixed before analysis: 0, 1, 2, >=3, outside",
           "main_sample": analyse(vuln_pairs(load_all_rows(DATA_PATH), RQ1_CAP)),
           "full_population": analyse(vuln_pairs(load_all_rows(FULL_JSONL), 10 ** 9))}
    rp = recent_pairs()
    if rp is not None:
        out["recent_cves"] = analyse(rp)
    with open(OUT, "w") as f:
        json.dump(out, f, indent=2)
    print(json.dumps(out, indent=2))


if __name__ == "__main__":
    main()
