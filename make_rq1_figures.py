"""
make_rq1_figures.py
-------------------
Figures for RQ1 (SANER revision), drawn only from result files and recomputed
reading orders:

  figures/fig_region_map.pdf   coverage vs. region size for every region we
                               measured, with iso-lift lines: (a) baselines and
                               sink sets on all 760 deletion pairs, (b) slice
                               direction on the 338 pairs where srcSlice yields
                               a forward slice.
  figures/fig_effort.pdf       share of functions whose first deleted line is
                               among the first k lines read, for each reading
                               order: (a) BigVul, 760 pairs; (b) CVE fixes since
                               2020 (needs the cloned repositories).

The plotted values are written to results/figure_data.json.
"""
import os
import sys
import json

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

R = lambda f: json.load(open(os.path.join(HERE, "results", f)))  # noqa: E731
FIG = os.path.join(HERE, "figures")
REPOS = os.path.expanduser("~/tools/cve_repos")

# Validated categorical slots 1-3 (dataviz reference palette, all-pairs PASS, light mode).
BLUE, ORANGE, AQUA = "#2a78d6", "#eb6834", "#1baf7a"
INK, INK2, GRID = "#0b0b0b", "#52514e", "#d9d8d4"

plt.rcParams.update({
    "font.family": "sans-serif", "font.sans-serif": ["DejaVu Sans", "Arial"], "font.size": 7.5,
    "axes.labelsize": 7.5, "axes.titlesize": 8, "axes.titleweight": "bold", "xtick.labelsize": 7,
    "ytick.labelsize": 7, "legend.fontsize": 7, "axes.spines.top": False, "axes.spines.right": False,
    "axes.edgecolor": INK2, "axes.linewidth": 0.6, "xtick.color": INK2, "ytick.color": INK2,
    "axes.grid": True, "grid.color": GRID, "grid.linewidth": 0.5, "axes.axisbelow": True,
    "figure.constrained_layout.use": True, "savefig.bbox": "tight", "savefig.pad_inches": 0.02,
    # Embed TrueType (Type 42), not Type 3 fonts: IEEE PDF eXpress rejects Type 3.
    "pdf.fonttype": 42, "ps.fonttype": 42,
})


def iso_lift(ax, xmax=0.8):
    """Dashed lines of constant lift (coverage = lift x RSR); lift 1 is the random-lines expectation.
    Each is labeled in the empty lower-left part of the plot, clear of the data labels."""
    xs = np.linspace(0.05, xmax, 200)
    for lift, lab in ((1.0, "lift 1.0× (random)"), (1.5, "1.5×"), (2.0, "2.0×")):
        ys = lift * xs
        m = ys <= 1.0
        ax.plot(xs[m], ys[m], color=INK2, lw=0.6, ls=(0, (3, 2)), zorder=1)
        if lift == 1.0:   # the random line crosses empty space at the lower left
            ax.text(0.47, 0.47 - 0.03, lab, color=INK2, fontsize=6.2, ha="left", va="top", bbox=BACKING, zorder=4)
        else:             # label the steeper lines low on the left, clear of the point labels
            xl = {1.5: 0.46, 2.0: 0.30}[lift]
            ax.text(xl - 0.005, lift * xl + 0.012, lab, color=INK2, fontsize=6.2, ha="right", va="bottom",
                    bbox=BACKING, zorder=4)


def point(ax, x, y, color, marker, label, dx=0.012, dy=0.0, ha="left", lead=False):
    # Baseline regions (squares) are hollow; our slices and srcSlice-based regions are filled, so
    # fill and shape still separate the three families when printed in greyscale. Labels of
    # crowded or coinciding points sit in free space with a thin leader line (lead=True).
    if marker == "s":
        ax.scatter([x], [y], s=46, facecolor="none", edgecolor=color, marker=marker, linewidth=1.4, zorder=2)
    else:
        ax.scatter([x], [y], s=30, color=color, marker=marker, edgecolor="white", linewidth=0.7, zorder=3)
    arrow = dict(arrowstyle="-", color=INK2, lw=0.5, shrinkA=1, shrinkB=3) if lead else None
    ax.annotate(label, (x, y), xytext=(x + dx, y + dy), color=INK, fontsize=6.6, ha=ha, va="center",
                zorder=4, arrowprops=arrow, bbox=BACKING)


BACKING = dict(boxstyle="square,pad=0.08", facecolor="white", edgecolor="none", alpha=0.85)


def better_cue(ax):
    """Where a good region lies: high coverage at small size (upper left)."""
    ax.text(0.212, 0.985, "\u2196 better", color=INK2, fontsize=6.4, ha="left", va="top", style="italic",
            bbox=BACKING, zorder=4)


def region_map():
    full = R("rq1_full_population.json")["rq1_all_qualifying"]
    b = full["baselines"]
    sinks = {v["variant"]: v["all"] for v in R("rq1_selective_sinks.json")["full_population"]["variants"]}
    d = R("rq1_slice_direction.json")["full_population"]
    rsr = full["mean_rsr"]
    vm_x, vm_y = b["variable_mention"]["mean_rsr"], b["variable_mention"]["mean_coverage_deletion"]
    # (label, x, y, colour, marker, text dx, text dy, alignment, leader line)
    a_pts = [
        ("Backward slice (all sinks)", rsr, full["mean_coverage"], BLUE, "o", 0.014, 0.0, "left", False),
        ("Memory sinks", sinks["memory"]["mean_rsr"], sinks["memory"]["mean_coverage"], BLUE, "o", 0.014, 0.0, "left", False),
        ("API-call sinks", sinks["api"]["mean_rsr"], sinks["api"]["mean_coverage"], BLUE, "o", 0.014, 0.0, "left", False),
        ("Variable mention\n(same point as the slice)", vm_x, vm_y, AQUA, "s", 0.40 - vm_x, 0.87 - vm_y, "right", True),
        ("Sink window", b["sink_window"]["mean_rsr"], b["sink_window"]["mean_coverage_deletion"], AQUA, "s", 0.014, 0.0, "left", False),
        ("Random window", rsr, b["random_window"]["mean_coverage_deletion"], AQUA, "s", 0.06, 0.035, "left", True),
        ("Tail", rsr, b["tail"]["mean_coverage_deletion"], AQUA, "s", 0.06, -0.005, "left", True),
        ("Random lines", rsr, b["random_lines"]["mean_coverage_deletion"], AQUA, "s", 0.06, -0.045, "left", True),
    ]
    b_pts = [
        ("Backward from sinks", d["backward"]["mean_rsr"], d["backward"]["mean_coverage"], BLUE, "o", -0.09, 0.04, "right", True),
        ("Backward, data only", d["backward_data"]["mean_rsr"], d["backward_data"]["mean_coverage"], BLUE, "o", 0.014, -0.02, "left", False),
        ("Forward \u222a backward", d["union"]["mean_rsr"], d["union"]["mean_coverage"], ORANGE, "^", 0.0, 0.045, "center", False),
        ("Forward from inputs", d["forward"]["mean_rsr"], d["forward"]["mean_coverage"], ORANGE, "^", 0.014, 0.0, "left", False),
        ("Chop (forward \u2229 backward)", d["chop"]["mean_rsr"], d["chop"]["mean_coverage"], ORANGE, "^", 0.012, -0.05, "left", False),
    ]
    fig, axes = plt.subplots(1, 2, figsize=(7.1, 2.15), sharey=True)
    for ax, pts, title in ((axes[0], a_pts, f"(a) Which sinks, which region? All {full['n_deletion_pairs']} pairs"),
                           (axes[1], b_pts, f"(b) Which direction? {d['n_with_forward']} pairs with a forward slice")):
        iso_lift(ax)
        better_cue(ax)
        for lab, x, y, c, mk, dx, dy, ha, lead in pts:
            point(ax, x, y, c, mk, lab, dx, dy, ha, lead)
        ax.set_xlim(0.2, 0.8)
        ax.set_ylim(0.3, 1.0)
        ax.set_xlabel("Region size (share of function kept, RSR)")
        ax.set_title(title, loc="left")
    axes[0].set_ylabel("Fix lines covered (share)")
    axes[1].tick_params(labelleft=True)
    handles = [plt.Line2D([], [], marker="o", color=BLUE, ls="", markersize=6.5, markeredgecolor="white"),
               plt.Line2D([], [], marker="^", color=ORANGE, ls="", markersize=6.5, markeredgecolor="white"),
               plt.Line2D([], [], marker="s", ls="", markersize=6.5, markerfacecolor="none",
                          markeredgecolor=AQUA, markeredgewidth=1.4),
               plt.Line2D([], [], color=INK2, lw=0.8, ls=(0, (3, 2)))]
    fig.legend(handles, ["Backward slice (ours)", "srcSlice forward-based", "Baseline region",
                         "Constant lift (1.0\u00d7 = random lines of the same size)"],
               loc="outside lower center", ncol=4, frameon=False)
    fig.savefig(os.path.join(FIG, "fig_region_map.pdf"))
    fig.savefig(os.path.join(FIG, "fig_region_map.png"), dpi=300)
    return {"a": [(p[0], p[1], p[2]) for p in a_pts], "b": [(p[0], p[1], p[2]) for p in b_pts]}


def first_positions(pairs):
    from rq1_baselines import changed_lines, slice_and_criteria, variable_mention_region
    from rq1_inspection_effort import sink_proximity_order, order_first
    rows = []
    for pr in pairs:
        fbl = pr["func_before"].splitlines()
        n = len(fbl)
        dels, _ = changed_lines(fbl, pr["func_after"].splitlines())
        if not n or not dels:
            continue
        S, crit, sinks = slice_and_criteria(pr["func_before"])
        vm, _ = variable_mention_region(fbl, crit)
        first = lambda order: min(order.index(x) for x in dels)  # noqa: E731
        rows.append({"n": n, "d": len(dels), "top_down": min(dels),
                     "var_mention": first(order_first(vm, n)),
                     "sink_proximity": first(sink_proximity_order(S, sinks, n))})
    return rows


def cdf(rows, key, ks):
    return [float(np.mean([r[key] < k for r in rows])) for k in ks]


def random_cdf(rows, ks):
    out = []
    for k in ks:
        vals = []
        for r in rows:
            p_none = 1.0
            for j in range(min(k, r["n"])):
                p_none *= max(0, r["n"] - r["d"] - j) / (r["n"] - j)
            vals.append(1 - p_none)
        out.append(float(np.mean(vals)))
    return out


def effort():
    from consolidated_study import vuln_pairs, load_all_rows
    from rq1_selective_sinks import ensure_full_jsonl
    ensure_full_jsonl()  # derive test.jsonl from the shipped parquet when run on its own
    populations = [("(a) BigVul, all 760 deletion pairs",
                    first_positions(vuln_pairs(load_all_rows(os.path.join(HERE, "data", "bigvul_full", "test.jsonl")), 10 ** 9)))]
    if os.path.isdir(REPOS):
        import rq1_recent_cves as rc
        import hashlib
        pairs, seen = [], set()
        for repo in sorted(os.listdir(REPOS)):
            for p in rc.mine(os.path.join(REPOS, repo)):
                h = hashlib.sha1((p["func_before"] + "\0" + p["func_after"]).encode("utf-8", "replace")).hexdigest()
                if h not in seen:
                    seen.add(h)
                    pairs.append(p)
        rows = first_positions(pairs)
        populations.append((f"(b) CVE fixes committed since 2020, {len(rows)} pairs", rows))
    ks = list(range(0, 31))
    fig, axes = plt.subplots(1, len(populations), figsize=(7.1, 2.0), sharey=True, squeeze=False)
    data = {}
    for ax, (title, rows) in zip(axes[0], populations):
        # Line style and marker differ per order, so the curves stay distinct in greyscale print.
        series = [("Slice, sink proximity", cdf(rows, "sink_proximity", ks), BLUE, "-", "o"),
                  ("Variable mention first", cdf(rows, "var_mention", ks), AQUA, (0, (5, 2)), "s"),
                  ("Top-down (no tool)", cdf(rows, "top_down", ks), ORANGE, (0, (6, 2, 1.5, 2)), "^"),
                  ("Random order", random_cdf(rows, ks), INK2, (0, (1, 1.5)), None)]
        for lab, ys, c, ls, mk in series:
            ax.plot(ks, ys, color=c, lw=1.6 if mk else 1.1, ls=ls, label=lab, solid_capstyle="round",
                    marker=mk, markevery=5, markersize=4.2, markerfacecolor="white" if mk == "s" else c,
                    markeredgecolor=c)
        ax.set_xlim(0, 30)
        ax.set_ylim(0, 1)
        ax.set_xlabel("Lines read (k)")
        ax.set_title(title, loc="left")
        data[title] = {lab: ys for lab, ys, _, _, _ in series}
    axes[0][0].set_ylabel("Functions with a fix line\namong the first k lines read")
    h, l = axes[0][0].get_legend_handles_labels()
    fig.legend(h, l, loc="outside lower center", ncol=4, frameon=False)
    fig.savefig(os.path.join(FIG, "fig_effort.pdf"))
    fig.savefig(os.path.join(FIG, "fig_effort.png"), dpi=300)
    data["ks"] = ks
    return data


def main():
    out = {"region_map": region_map(), "effort": effort()}
    json.dump(out, open(os.path.join(HERE, "results", "figure_data.json"), "w"), indent=2)
    print("written figures/fig_region_map.pdf, figures/fig_effort.pdf, results/figure_data.json")


if __name__ == "__main__":
    main()
