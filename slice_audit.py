"""
slice_audit.py -- sink-anchored inspection regions for C code.

Usage:
  python slice_audit.py PATH [PATH ...] [--features] [--out report.jsonl] [--show N]

For every function definition in the given .c/.h files (directories are
searched recursively), emits one JSON line with:
  file, function, start_line, n_lines,
  sinks        source lines of sink statements (calls, subscripts, derefs, ->,
               pointer arithmetic),
  slice        source lines of the intraprocedural backward slice from all sinks,
  order        suggested reading order: slice lines by distance to the nearest
               sink, then the remaining lines (the ordering evaluated in the paper),
  features     (with --features) the abstract srcML feature set phi over the slice.
Line numbers are 1-based positions in the original file. --show N prints the
first N lines of the reading order for each function with their source text.
A throughput summary is printed at the end.
"""
import argparse
import json
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import ast_slicer  # noqa: E402
from rq1_baselines import slice_and_criteria  # noqa: E402


def function_nodes(code_bytes):
    root = ast_slicer._PARSER.parse(code_bytes).root_node
    stack = [root]
    while stack:
        n = stack.pop()
        if n.type == "function_definition":
            yield n
            continue
        stack.extend(reversed(n.children))


def function_name(node, code_bytes):
    d = node.child_by_field_name("declarator")
    while d is not None and d.type not in ("function_declarator",):
        d = d.child_by_field_name("declarator")
    if d is not None:
        ident = d.child_by_field_name("declarator")
        if ident is not None:
            return code_bytes[ident.start_byte:ident.end_byte].decode("utf-8", "replace")
    return "?"


def audit_function(code, want_features):
    S, _crit, sinks = slice_and_criteria(code)
    n = len(code.splitlines())

    def dist(li):
        return min((abs(li - s) for s in sinks), default=n)
    order = sorted(S, key=lambda li: (dist(li), li)) + [i for i in range(n) if i not in S]
    rec = {"sinks": sorted(sinks), "slice": sorted(S), "order": order}
    if want_features:
        from augmented_study import srcml_features  # noqa: PLC0415
        rec["features"] = sorted({f for ln, f in srcml_features(code) if ln >= 1 and (ln - 1) in S})
    return rec


def iter_files(paths):
    for p in paths:
        if os.path.isdir(p):
            for d, _, fs in os.walk(p):
                for f in sorted(fs):
                    if f.endswith((".c", ".h")):
                        yield os.path.join(d, f)
        elif p.endswith((".c", ".h")):
            yield p


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("paths", nargs="+")
    ap.add_argument("--features", action="store_true", help="also extract srcML features over the slice")
    ap.add_argument("--out", default="-", help="JSONL output file (default: stdout)")
    ap.add_argument("--show", type=int, default=0, help="print the first N lines of each reading order")
    args = ap.parse_args(argv)

    out = sys.stdout if args.out == "-" else open(args.out, "w")
    n_files = n_funcs = n_lines = kept = 0
    t0 = time.perf_counter()
    for path in iter_files(args.paths):
        src = open(path, "rb").read()
        n_files += 1
        for node in function_nodes(src):
            code = src[node.start_byte:node.end_byte].decode("utf-8", "replace")
            base = node.start_point[0]
            rec = audit_function(code, args.features)
            n = len(code.splitlines())
            n_funcs += 1
            n_lines += n
            kept += len(rec["slice"])
            shift = lambda xs: [base + x + 1 for x in xs]  # noqa: E731
            row = {"file": path, "function": function_name(node, src), "start_line": base + 1,
                   "n_lines": n, "sinks": shift(rec["sinks"]), "slice": shift(rec["slice"]),
                   "order": shift(rec["order"])}
            if "features" in rec:
                row["features"] = rec["features"]
            if args.out != "-" or not args.show:
                out.write(json.dumps(row) + "\n")
            if args.show:
                lines = code.splitlines()
                print(f"\n{path}:{base + 1} {row['function']}  ({len(rec['slice'])}/{n} lines in slice)")
                for li in rec["order"][:args.show]:
                    mark = "S" if li in rec["sinks"] else " "
                    print(f"  {base + li + 1:6d} {mark} {lines[li].rstrip()[:100]}")
    dt = time.perf_counter() - t0
    summary = {"files": n_files, "functions": n_funcs, "lines": n_lines,
               "slice_share": round(kept / n_lines, 3) if n_lines else None,
               "seconds": round(dt, 2), "functions_per_second": round(n_funcs / dt, 1) if dt else None}
    print(json.dumps({"summary": summary}), file=sys.stderr)
    if out is not sys.stdout:
        out.close()


if __name__ == "__main__":
    main()
