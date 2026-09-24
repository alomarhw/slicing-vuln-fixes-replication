"""
rp_style.py
-----------
Shared matplotlib styling for the SG-VDR experiment.
Provides apply_style() and save() with crash-guard against zero-size axes.
"""

import os
import pathlib
import warnings
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# Colorblind-safe Okabe-Ito palette + default colormap, referenced by plots.py
# (plots.py imports rp_style.PALETTE / rp_style.CMAP).
PALETTE = ["#0072B2", "#E69F00", "#009E73", "#D55E00", "#CC79A7", "#56B4E9", "#F0E442", "#000000"]
PAL = PALETTE
CMAP = "viridis"

_STYLE_APPLIED = False


def apply_style() -> None:
    global _STYLE_APPLIED
    if _STYLE_APPLIED:
        return
    plt.rcParams.update({
        "figure.dpi": 150,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.titlesize": 12,
        "axes.labelsize": 11,
        "xtick.labelsize": 9,
        "ytick.labelsize": 9,
        "legend.fontsize": 9,
        "figure.constrained_layout.use": False,   # avoid constrained-layout collapse
    })
    _STYLE_APPLIED = True


def save(path: str, fig: plt.Figure | None = None, tight: bool = True) -> None:
    """Save *fig* (or current figure) to *path*; create parent dirs; guard against 0-size axes."""
    apply_style()
    if fig is None:
        fig = plt.gcf()
    p = pathlib.Path(path)
    p.parent.mkdir(parents=True, exist_ok=True)

    # Ensure figure has positive size
    w, h = fig.get_size_inches()
    if w <= 0 or h <= 0:
        fig.set_size_inches(max(w, 8), max(h, 5))

    # Suppress constrained-layout warning, use tight_layout instead
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        try:
            if tight:
                fig.tight_layout(pad=1.5)
        except Exception:
            pass
        try:
            fig.savefig(str(p), dpi=150, bbox_inches="tight")
            print(f"[rp_style] Figure saved -> {p}")
        except Exception as exc:
            print(f"[rp_style] WARNING: Could not save {p}: {exc}")
    plt.close(fig)