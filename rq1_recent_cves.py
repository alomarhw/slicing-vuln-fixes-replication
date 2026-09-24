"""
rq1_recent_cves.py
------------------
SANER revision: out-of-sample check on CVE fixes made AFTER BigVul's
collection period, in current C projects.

For each repository given on the command line, every non-merge commit since
2020-01-01 whose message cites a CVE id (CVE-20xx-nnnn) is taken as a fix
commit. As in BigVul, each C function whose text differs between the parent
and the fix commit is a (func_before, func_after) pair. Test/fuzz files are
skipped, as are vendored/bundled third-party code and release, version-bump,
and library-update commits
(which cite CVEs without being the fix); identical pairs (e.g., backports)
are de-duplicated. The
unchanged RQ1 slicer, baselines, and inspection-effort metrics are applied;
results are reported per function and commit-weighted (each fix commit once).

Usage: python rq1_recent_cves.py REPO_DIR [REPO_DIR ...]
Writes results/rq1_recent_cves.json (per-project and pooled results) and
results/rq1_recent_cves_pairs.jsonl (commit, CVE ids, file, function).
"""
import os
import re
import sys
import json
import hashlib
import subprocess
import collections

import numpy as np
from scipy.stats import wilcoxon

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

import ast_slicer  # noqa: E402
from rq1_baselines import changed_lines, slice_and_criteria, variable_mention_region  # noqa: E402
from rq1_inspection_effort import metrics, sink_proximity_order  # noqa: E402
from slice_audit import function_nodes, function_name  # noqa: E402

SINCE = "2020-01-01"
CVE_RE = re.compile(r"CVE-20\d\d-\d{4,7}", re.I)
SKIP_PATH = re.compile(r"(^|/)(tests?|fuzz\w*|testsuite|regress|third_?party|vendor|external|deps"
                       r"|libmagic|libgd|libmbfl|libsqlite|pcre2lib)/", re.I)
# Release / version-bump commits list fixed CVEs in their notes without being the fix.
SKIP_SUBJECT = re.compile(r"^\s*(release|prepare|bump|update (news|changes|changelog)"
                          r"|(\S+: )?update \S+ to |\S+: update to )", re.I)
OUT = os.path.join(HERE, "results", "rq1_recent_cves.json")
PAIRS_OUT = os.path.join(HERE, "results", "rq1_recent_cves_pairs.jsonl")


def git(repo, *args):
    return subprocess.run(["git", "-C", repo, *args], capture_output=True, timeout=120).stdout


def functions(src_bytes):
    out = {}
    for node in function_nodes(src_bytes):
        name = function_name(node, src_bytes)
        if name != "?" and name not in out:
            out[name] = src_bytes[node.start_byte:node.end_byte].decode("utf-8", "replace")
    return out


def mine(repo):
    log = git(repo, "log", f"--since={SINCE}", "--no-merges", "-i", "--grep=CVE-20",
              "--format=%H%x1f%s%x1f%b%x1e").decode("utf-8", "replace")
    pairs = []
    for entry in log.split("\x1e"):
        parts = entry.strip().split("\x1f")
        if len(parts) < 2:
            continue
        sha, msg = parts[0], " ".join(parts[1:])
        if SKIP_SUBJECT.search(parts[1]):
            continue
        cves = sorted({c.upper() for c in CVE_RE.findall(msg)})
        if not cves:
            continue
        files = git(repo, "diff", "--name-only", f"{sha}^", sha).decode().split()
        for path in files:
            if not path.endswith(".c") or SKIP_PATH.search(path):
                continue
            before = functions(git(repo, "show", f"{sha}^:{path}"))
            after = functions(git(repo, "show", f"{sha}:{path}"))
            for name, fb in before.items():
                fa = after.get(name)
                if fa is not None and fa != fb:
                    pairs.append({"repo": os.path.basename(repo.rstrip("/")), "commit": sha,
                                  "cves": cves, "file": path, "function": name,
                                  "func_before": fb, "func_after": fa})
    return pairs


def evaluate(pairs):
    rows = []
    for p in pairs:
        fbl = p["func_before"].splitlines()
        n = len(fbl)
        dels, _ = changed_lines(fbl, p["func_after"].splitlines())
        if not n or not dels:
            continue
        S, crit, sinks = slice_and_criteria(p["func_before"])
        vm, _ = variable_mention_region(fbl, crit)
        td = metrics(list(range(n)), dels, n)
        sp = metrics(sink_proximity_order(S, sinks, n), dels, n)
        # Same ordering without the slice: the variable-mention region, and the whole function.
        vsp = metrics(sink_proximity_order(set(vm), sinks, n), dels, n)
        wsp = metrics(sink_proximity_order(set(range(n)), sinks, n), dels, n)
        rows.append({"repo": p["repo"], "commit": p["commit"], "cov": len(S & dels) / len(dels), "rsr": len(S) / n,
                     "vm_cov": (len(vm & dels) / len(dels)) if vm else 0.0, "vm_rsr": len(vm) / n,
                     "ifa_td": td["ifa"], "ifa_sp": sp["ifa"], "top3_td": td["top3"], "top3_sp": sp["top3"],
                     "top5_td": td["top5"], "top5_sp": sp["top5"],
                     "effort_td": td["effort"], "effort_sp": sp["effort"],
                     "top1_sp": sp["top1"], "top1_vmsp": vsp["top1"], "top1_wsp": wsp["top1"],
                     "ifa_vmsp": vsp["ifa"], "top3_vmsp": vsp["top3"], "effort_vmsp": vsp["effort"],
                     "ifa_wsp": wsp["ifa"], "top3_wsp": wsp["top3"], "effort_wsp": wsp["effort"]})
    return rows


def summarize(rows):
    if not rows:
        return {"n": 0}
    a = lambda k: np.array([r[k] for r in rows])  # noqa: E731
    cov, rsr, vm = a("cov"), a("rsr"), a("vm_cov")
    rng = np.random.default_rng(1337)
    boot = [cov[rng.integers(0, len(cov), len(cov))].mean() for _ in range(2000)]
    p = lambda x, y: float(wilcoxon(x, y).pvalue) if np.any(x - y) else None  # noqa: E731
    return {"n": len(rows), "coverage": float(cov.mean()),
            "coverage_ci95": [float(np.percentile(boot, 2.5)), float(np.percentile(boot, 97.5))],
            "median_coverage": float(np.median(cov)), "rsr": float(rsr.mean()),
            "lift_vs_random": float(cov.mean() / rsr.mean()), "p_vs_random": p(cov, rsr),
            "var_mention_coverage": float(vm.mean()), "var_mention_rsr": float(a("vm_rsr").mean()),
            "p_vs_var_mention": p(cov, vm),
            "ifa_top_down": float(a("ifa_td").mean()), "ifa_sink_proximity": float(a("ifa_sp").mean()),
            "ifa_median_top_down": float(np.median(a("ifa_td"))), "ifa_median_sink_proximity": float(np.median(a("ifa_sp"))),
            "top3_top_down": float(a("top3_td").mean()), "top3_sink_proximity": float(a("top3_sp").mean()),
            "top5_top_down": float(a("top5_td").mean()), "top5_sink_proximity": float(a("top5_sp").mean()),
            "effort_top_down": float(a("effort_td").mean()), "effort_sink_proximity": float(a("effort_sp").mean()),
            "p_ifa": p(a("ifa_sp"), a("ifa_td")),
            "ordering_check": {
                "top1_sink_proximity": float(a("top1_sp").mean()),
                "var_mention_sink_proximity": {"ifa": float(a("ifa_vmsp").mean()),
                                               "ifa_median": float(np.median(a("ifa_vmsp"))),
                                               "top1": float(a("top1_vmsp").mean()),
                                               "top3": float(a("top3_vmsp").mean()),
                                               "effort": float(a("effort_vmsp").mean()),
                                               "p_ifa_vs_slice": p(a("ifa_sp"), a("ifa_vmsp"))},
                "whole_sink_proximity": {"ifa": float(a("ifa_wsp").mean()),
                                         "ifa_median": float(np.median(a("ifa_wsp"))),
                                         "top1": float(a("top1_wsp").mean()),
                                         "top3": float(a("top3_wsp").mean()),
                                         "effort": float(a("effort_wsp").mean()),
                                         "p_ifa_vs_slice": p(a("ifa_sp"), a("ifa_wsp"))}}}


def main():
    all_pairs, seen = [], set()
    # Sort repositories and pairs so results (incl. the seeded bootstrap CI) don't depend on CLI order.
    for repo in sorted(sys.argv[1:], key=lambda r: os.path.basename(r.rstrip("/"))):
        for p in mine(repo):
            h = hashlib.sha1((p["func_before"] + "\0" + p["func_after"]).encode("utf-8", "replace")).hexdigest()
            if h not in seen:
                seen.add(h)
                all_pairs.append(p)
    all_pairs.sort(key=lambda p: (p["repo"], p["commit"], p["file"], p["function"]))
    rows = evaluate(all_pairs)
    by_repo = collections.defaultdict(list)
    for r in rows:
        by_repo[r["repo"]].append(r)
    cves = {c for p in all_pairs for c in p["cves"]}
    commits = {(p["repo"], p["commit"]) for p in all_pairs}
    by_commit = collections.defaultdict(list)
    for r in rows:
        by_commit[(r["repo"], r["commit"])].append(r)
    cw = {k: float(np.mean([np.mean([r[k] for r in v]) for v in by_commit.values()]))
          for k in ("cov", "rsr", "vm_cov", "ifa_td", "ifa_sp", "top3_td", "top3_sp", "effort_td", "effort_sp")}
    cw["n_commits"] = len(by_commit)
    cw["lift_vs_random"] = cw["cov"] / cw["rsr"]
    out = {"since": SINCE, "repos": sorted(by_repo), "n_fix_commits": len(commits), "n_cves": len(cves),
           "n_function_pairs": len(all_pairs), "n_deletion_pairs": len(rows),
           "pooled": summarize(rows), "commit_weighted": cw, "by_repo": {k: summarize(v) for k, v in sorted(by_repo.items())}}
    with open(OUT, "w") as f:
        json.dump(out, f, indent=2)
    with open(PAIRS_OUT, "w") as f:
        for p in all_pairs:
            f.write(json.dumps({k: p[k] for k in ("repo", "commit", "cves", "file", "function")}) + "\n")
    print(json.dumps({k: v for k, v in out.items() if k != "by_repo"}, indent=2))
    for k, v in out["by_repo"].items():
        print(k, v.get("n"), round(v.get("coverage", 0), 3), round(v.get("rsr", 0), 3))


if __name__ == "__main__":
    main()
