#!/usr/bin/env python3
"""Publication-quality redesign of Fig. 1 for
'Scope Is Not Content: Structural Augmentation of Program Slices
 for Vulnerability Fix Localization'.

Visual grammar follows the SRCVUL-style architecture figure: shaded process
lanes with italic captions, small-caps stage boxes, pastel fills keyed by
role, solid arrows for the data path, dashed arrows for ground-truth /
measurement-only flows, and cylinders for corpora.

Text is laid out with real font metrics (PIL) so that small caps, mixed
math/prose runs and auto-sized chips are exact rather than estimated.
"""
import os

from PIL import ImageFont

W, H = 1740, 690

SERIF = "DejaVu Serif, Liberation Serif, Times New Roman, Times, serif"
MONO = "DejaVu Sans Mono, Courier New, monospace"

# Font faces. The figure's geometry is computed from real font metrics, so the
# renderer must use the same faces listed here -- the FIRST resolvable name in
# each list must also be the font cairosvg/fontconfig actually selects for the
# corresponding CSS family string above, or PIL's width metrics will silently
# diverge from the render and text will overlap. Times New Roman's GSUB
# ligature tables (fi/fl/ff) trip up cairosvg's simplified text shaping and
# insert visible gaps ("f ix" for "fix"), so DejaVu Serif/Mono (bundled with
# matplotlib, no such ligatures) are used instead and listed first.
_FACES = {
    ("s", "n", "n"): ["DejaVuSerif.ttf", "LiberationSerif-Regular.ttf"],
    ("s", "b", "n"): ["DejaVuSerif-Bold.ttf", "LiberationSerif-Bold.ttf"],
    ("s", "n", "i"): ["DejaVuSerif-Italic.ttf", "LiberationSerif-Italic.ttf"],
    ("s", "b", "i"): ["DejaVuSerif-BoldItalic.ttf", "LiberationSerif-BoldItalic.ttf"],
    ("m", "n", "n"): ["DejaVuSansMono.ttf"],
    ("m", "b", "n"): ["DejaVuSansMono-Bold.ttf"],
    ("m", "n", "i"): ["DejaVuSansMono-Oblique.ttf"],
}

_SEARCH_DIRS = [
    "/usr/share/fonts/truetype/liberation",
    "/usr/share/fonts/truetype/dejavu",
    "/usr/share/fonts/liberation",
    "/usr/share/fonts/dejavu",
    "/usr/local/share/fonts",
    "/Library/Fonts",
    "/System/Library/Fonts/Supplemental",
    os.path.expanduser("~/Library/Fonts"),
    os.path.expanduser("~/.fonts"),
    "C:\\Windows\\Fonts",
]
try:  # matplotlib ships DejaVu, and often Liberation too
    import matplotlib
    _SEARCH_DIRS.append(os.path.join(os.path.dirname(matplotlib.__file__),
                                     "mpl-data", "fonts", "ttf"))
except Exception:
    pass


def _resolve(key):
    for name in _FACES[key]:
        for d in _SEARCH_DIRS:
            cand = os.path.join(d, name)
            if os.path.exists(cand):
                return cand
        for root in ("/usr/share/fonts", "/usr/local/share/fonts"):
            for dirpath, _, files in os.walk(root) if os.path.isdir(root) else []:
                if name in files:
                    return os.path.join(dirpath, name)
    raise SystemExit(
        "Missing font face %s. Install the Liberation and DejaVu font "
        "families, or add their directory to _SEARCH_DIRS at the top of this "
        "script." % _FACES[key][0])


FP = {k: _resolve(k) for k in _FACES}
_cache = {}


def _font(key, size):
    k = (key, round(size * 8))
    if k not in _cache:
        _cache[k] = ImageFont.truetype(FP[key], round(size * 8))
    return _cache[k]


def wid(s, size, fam="s", weight="n", style="n", track=0.0):
    key = (fam, weight, style)
    if key not in FP:
        key = (fam, weight, "n")
    return _font(key, size).getlength(s) / 8.0 + track * len(s)


# ---------------------------------------------------------------- palette
C_BAND_GT, C_BAND_GT_S = "#F1E9F8", "#D3BFE6"
C_BAND_SCP, C_BAND_SCP_S = "#DDE4F2", "#B4C3E0"
C_BAND_CNT, C_BAND_CNT_S = "#E5EED8", "#BCCFA2"
C_BAND_EVL, C_BAND_EVL_S = "#FBE6E6", "#EDBFBF"

C_DOC, C_DOC_S = "#FFFFFF", "#575757"
C_CYL, C_CYL_S = "#F2F2F2", "#575757"
C_TOOL, C_TOOL_S = "#5B9BD5", "#2C5C8C"
C_OURS, C_OURS_S = "#F7D98C", "#B98F22"
C_OURS2, C_OURS2_S = "#CFE0AE", "#71903B"
C_COMB, C_COMB_S = "#EDA97C", "#B56630"
C_DIFF, C_DIFF_S = "#CFE0AE", "#67893A"
C_RQ1, C_RQ1_S = "#DCE8F7", "#3B6EA5"
C_RQ2, C_RQ2_S = "#F4E2F0", "#8F5187"
C_RQ3, C_RQ3_S = "#EAEAEA", "#5A5A5A"
C_CHIP, C_CHIP_S = "#FFF7DC", "#C6A33A"
C_INK, C_SOFT, C_MUTE = "#161616", "#464646", "#6C6C6C"
C_CAP = "#3A3A4E"

out = []
add = out.append


def esc(t):
    return t.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def raw(x, y, s, size, fam="s", weight="n", style="n", fill=C_INK,
        anchor="start", track=0.0, opacity=1.0):
    fam_css = SERIF if fam == "s" else MONO
    wt = "bold" if weight == "b" else "normal"
    st = "italic" if style == "i" else "normal"
    ls = f' letter-spacing="{track}"' if track else ""
    op = f' opacity="{opacity}"' if opacity != 1.0 else ""
    add(f'<text x="{x:.1f}" y="{y:.1f}" font-family="{fam_css}" '
        f'font-size="{size:.2f}" font-weight="{wt}" font-style="{st}" '
        f'fill="{fill}" text-anchor="{anchor}"{ls}{op} '
        f'xml:space="preserve">{esc(s)}</text>')


# --------------------------------------------------- rich (multi-run) text
def _expand(segs, base):
    """segs: list of (kind, text). Returns list of runs
    (text, size, fam, weight, style, track)."""
    runs = []
    for kind, t in segs:
        if kind == "sc":                      # true small caps
            cur, is_low = "", None
            for ch in t + "\0":
                low = ("a" <= ch <= "z")
                if is_low is None:
                    is_low = low
                if ch == "\0" or low != is_low:
                    if cur:
                        runs.append((cur.upper() if is_low else cur,
                                     base * 0.79 if is_low else base,
                                     "s", "b", "n", 0.5))
                    cur, is_low = ch, low
                else:
                    cur += ch
        elif kind == "b":
            runs.append((t, base, "s", "b", "n", 0.0))
        elif kind == "n":
            runs.append((t, base, "s", "n", "n", 0.0))
        elif kind == "i":
            runs.append((t, base, "s", "n", "i", 0.0))
        elif kind == "mi":
            runs.append((t, base, "s", "b", "i", 0.0))
        elif kind == "m":
            runs.append((t, base * 0.90, "m", "n", "n", 0.0))
        elif kind == "mb":
            runs.append((t, base * 0.90, "m", "b", "n", 0.0))
    return runs


def _normalize(runs):
    """Move boundary spaces to the start of the following run (trailing
    whitespace is trimmed by SVG renderers) and pad family switches so that
    prose and monospace runs do not collide."""
    runs = [list(r) for r in runs]
    for i in range(len(runs) - 1):
        t = runs[i][0]
        stripped = t.rstrip(" ")
        if stripped != t:
            runs[i][0] = stripped
            runs[i + 1][0] = t[len(stripped):] + runs[i + 1][0]
    return [tuple(r) for r in runs]


def _pads(runs):
    """Extra advance inserted before each run at a family boundary."""
    pads = [0.0] * len(runs)
    for i in range(1, len(runs)):
        a, b = runs[i - 1], runs[i]
        if b[2] != a[2]:
            pads[i] = 3.2
        elif b[4] != a[4]:
            pads[i] = 2.2
        elif b[3] != a[3]:
            pads[i] = 1.4
    return pads


def rich(cx, y, segs, base=15.0, fill=C_INK, anchor="middle"):
    runs = _normalize(_expand(segs, base))
    pads = _pads(runs)
    ws = [wid(t, s, f, w, st, tr) for (t, s, f, w, st, tr) in runs]
    total = sum(ws) + sum(pads)
    x = cx - total / 2 if anchor == "middle" else (
        cx - total if anchor == "end" else cx)
    for (t, s, f, w, st, tr), ww, pd in zip(runs, ws, pads):
        x += pd
        raw(x, y, t, s, f, w, st, fill, "start", tr)
        x += ww
    return total


def rich_w(segs, base):
    runs = _normalize(_expand(segs, base))
    return (sum(wid(t, s, f, w, st, tr) for (t, s, f, w, st, tr) in runs)
            + sum(_pads(runs)))


def rich_fit(cx, y, segs, base, maxw, fill=C_INK):
    while base > 7 and rich_w(segs, base) > maxw:
        base -= 0.3
    return rich(cx, y, segs, base, fill)


# --------------------------------------------------------------- primitives
def rrect(x, y, w, h, fill, stroke, r=8, sw=1.7, dash=None, opacity=1.0):
    d = f' stroke-dasharray="{dash}"' if dash else ""
    op = f' opacity="{opacity}"' if opacity != 1.0 else ""
    add(f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" '
        f'rx="{r}" ry="{r}" fill="{fill}" stroke="{stroke}" '
        f'stroke-width="{sw}"{d}{op}/>')


def band(x, y, w, h, fill, stroke, cap_segs=None, cap_size=15.5):
    rrect(x, y, w, h, fill, stroke, r=15, sw=1.2, opacity=0.96)
    if cap_segs:
        rich(x + w / 2, y + 25, cap_segs, cap_size, C_CAP)


def doc_shape(x, y, w, h, top, bottom):
    p = (f"M {x} {y} L {x+w} {y} L {x+w} {y+h-11} "
         f"Q {x+0.78*w:.1f} {y+h+3} {x+0.55*w:.1f} {y+h-9} "
         f"Q {x+0.30*w:.1f} {y+h-21} {x} {y+h-9} Z")
    add(f'<path d="{p}" fill="{C_DOC}" stroke="{C_DOC_S}" stroke-width="1.7"/>')
    rich(x + w / 2, y + h / 2 - 1, [("mb", top)], 15.5)
    rich(x + w / 2, y + h / 2 + 17, [("sc", bottom)], 13.0, C_SOFT)


def cylinder(x, y, w, h, lines):
    ry = 12
    add(f'<path d="M {x} {y+ry} L {x} {y+h-ry} A {w/2} {ry} 0 0 0 {x+w} '
        f'{y+h-ry} L {x+w} {y+ry} Z" fill="{C_CYL}" stroke="{C_CYL_S}" '
        f'stroke-width="1.7"/>')
    add(f'<ellipse cx="{x+w/2}" cy="{y+ry}" rx="{w/2}" ry="{ry}" '
        f'fill="#FAFAFA" stroke="{C_CYL_S}" stroke-width="1.7"/>')
    y0 = y + h / 2 - (len(lines) - 1) * 9 + 8
    for i, segs in enumerate(lines):
        rich(x + w / 2, y0 + i * 18, segs, 14.0)


def poly(points, dashed=False, stroke=C_INK, sw=1.8, head=True, dash="6,4.5"):
    pts = " ".join(f"{px:.1f},{py:.1f}" for px, py in points)
    d = f' stroke-dasharray="{dash}"' if dashed else ""
    m = ' marker-end="url(#ah)"' if head else ""
    add(f'<polyline points="{pts}" fill="none" stroke="{stroke}" '
        f'stroke-width="{sw}"{d}{m} stroke-linejoin="round"/>')


def arrow(x1, y1, x2, y2, **kw):
    poly([(x1, y1), (x2, y2)], **kw)


def chip(x, y, segs, base=12.4, fill=C_CHIP, stroke=C_CHIP_S, h=26, pad=10):
    w = rich_w(segs, base) + 2 * pad
    rrect(x, y, w, h, fill, stroke, r=6, sw=1.25)
    rich(x + w / 2, y + h / 2 + base * 0.35, segs, base)
    return w


def chip_c(cx, y, segs, base=12.6, maxw=320, fill=C_CHIP, stroke=C_CHIP_S,
           h=26):
    while base > 8 and rich_w(segs, base) + 20 > maxw:
        base -= 0.3
    w = rich_w(segs, base) + 20
    rrect(cx - w / 2, y, w, h, fill, stroke, r=6, sw=1.25)
    rich(cx, y + h / 2 + base * 0.35, segs, base)


def stage(x, y, w, h, tag, title_segs, sub_lines, fill, stroke):
    rrect(x, y, w, h, fill, stroke, r=9, sw=1.9)
    raw(x + 11, y + 17, tag.upper(), 10.2, "s", "n", "n", C_MUTE, "start", 0.9)
    rich_fit(x + w / 2, y + 37, title_segs, 16.0, w - 22)
    for i, segs in enumerate(sub_lines):
        rich_fit(x + w / 2, y + 57 + i * 16, segs, 12.6, w - 18, C_SOFT)


# ================================================================== canvas
# Crop to the drawn content (lanes span x 22..1720, y 58..662) so no blank margin sits between the
# figure and its caption in the paper.
CX, CY, CW, CH = 12, 50, 1718, 622
add(f'<svg xmlns="http://www.w3.org/2000/svg" width="{CW}" height="{CH}" '
    f'viewBox="{CX} {CY} {CW} {CH}">')
add('<defs><marker id="ah" viewBox="0 0 10 10" refX="8.5" refY="5" '
    'markerWidth="6.5" markerHeight="6.5" orient="auto-start-reverse">'
    f'<path d="M 0 1.2 L 10 5 L 0 8.8 z" fill="{C_INK}"/></marker></defs>')
add(f'<rect width="{W}" height="{H}" fill="#FFFFFF"/>')

# ---------------------------------------------------------------- lanes
band(215, 58, 530, 212, C_BAND_GT, C_BAND_GT_S,
     [("i", "Ground truth \u2014 consulted for measurement only")])
band(215, 294, 865, 164, C_BAND_SCP, C_BAND_SCP_S,
     [("i", "Scope \u2014 backward slice from "), ("m", "func_before"), ("i", " alone")])
band(215, 472, 865, 190, C_BAND_CNT, C_BAND_CNT_S,
     [("i", "Content \u2014 srcML features over that scope")])
band(1350, 58, 370, 604, C_BAND_EVL, C_BAND_EVL_S,
     [("i", "Evaluation")])

# --------------------------------------------------------------- corpus
cylinder(28, 130, 152, 94, [[("sc", "BigVul")],
                            [("n", "CVE-fixing")],
                            [("n", "commits")]])
rich(104, 252, [("n", "400 CVE pairs -> RQ1, RQ2")], 12.6, C_MUTE)
rich(104, 269, [("n", "2,500 functions (1:4) -> RQ3")], 12.6, C_MUTE)

# --------------------------------------------------- ground-truth lane
doc_shape(243, 98, 168, 60, "func_after", "patched")
doc_shape(243, 192, 168, 60, "func_before", "vulnerable")
rrect(478, 150, 98, 46, C_DIFF, C_DIFF_S, r=9, sw=1.9)
rich(527, 179, [("sc", "diff")], 17.0)

arrow(180, 158, 238, 128, dashed=True)
arrow(180, 198, 238, 224, dashed=True)
poly([(411, 126), (446, 126), (446, 162), (473, 162)], dashed=True)
poly([(411, 220), (446, 220), (446, 184), (473, 184)], dashed=True)

poly([(576, 173), (1366, 173)], dashed=True)
rich(968, 162, [("mi", "D(F)"), ("i", "  deleted lines")], 13.6, C_CAP)

# ------------------------------------------------------------ scope lane
stage(290, 336, 205, 80, "stage 1", [("sc", "Sink Criterion")],
      [[("n", "SySeVR-style predicate")]], C_OURS, C_OURS_S)
stage(560, 336, 205, 80, "stage 2", [("sc", "Backward Slicer")],
      [[("n", "worklist: data + control")]], C_OURS, C_OURS_S)
stage(830, 336, 205, 80, "output",
      [("sc", "Slice "), ("mi", "S(F, c)")],
      [[("mi", "RSR"), ("n", " = |S| / n")]], C_OURS2, C_OURS2_S)

arrow(495, 376, 554, 376)
arrow(765, 376, 824, 376)
rich(524, 367, [("mi", "c")], 13.6, C_SOFT)
rich(794, 367, [("mi", "S")], 13.6, C_SOFT)

# function feed
poly([(250, 252), (250, 376), (284, 376)])
poly([(250, 376), (250, 544), (284, 544)])
rich(242, 464, [("mi", "F")], 14.5, C_INK, "end")

# ---------------------------------------------------------- content lane
stage(290, 504, 205, 80, "external tool", [("sc", "srcML Parse")],
      [[("n", "XML markup, no compile")]], C_TOOL, C_TOOL_S)
stage(560, 504, 205, 80, "stage 3", [("sc", "AST Visitor")],
      [[("n", "abstract elements in S")]], C_OURS, C_OURS_S)
stage(830, 504, 205, 80, "output",
      [("sc", "Features "), ("mi", "φ(F, S)")],
      [[("n", "set of feature tokens")]], C_OURS2, C_OURS2_S)

arrow(495, 544, 554, 544)
arrow(765, 544, 824, 544)
rich(524, 535, [("mi", "A(F)")], 12.6, C_SOFT)
rich(794, 535, [("mi", "φ")], 14.0, C_SOFT)

rich(248, 623, [("mi", "φ :")], 14.0, C_SOFT, "start")
cx = 282
for lbl in ["op:>", "ctrl:if", "idx", "callee:memcpy", "lit:number", "type:size_t", "mod:*"]:
    cx += chip(cx, 607, [("mb", lbl)], 12.4) + 10

# --------------------------------------------- measured representation
rrect(1115, 302, 180, 360, C_COMB, C_COMB_S, r=13, sw=2.0)
rich(1205, 340, [("b", "R(F) = ")], 17.5, "#231205")
rich(1205, 370, [("mb", "( S , φ )")], 20.0, "#231205")
add(f'<line x1="1140" y1="390" x2="1270" y2="390" stroke="{C_COMB_S}" '
    f'stroke-width="1.2"/>')
rich(1205, 416, [("i", "scope"), ("n", "  ×  "), ("i", "content")], 14.0, "#3D2411")
rich(1205, 450, [("n", "content variants")], 13.2, "#3D2411")
for i, lbl in enumerate(["raw tokens", "normalized tokens", "φ features"]):
    rrect(1140, 462 + i * 34, 130, 26, "#FFF5EE", "#C4885C", r=7, sw=1.2)
    rich(1205, 480 + i * 34, [("n", lbl)], 12.6, C_SOFT)
rich(1205, 590, [("n", "TF–IDF +")], 13.0, "#3D2411")
rich(1205, 607, [("n", "logistic regression")], 13.0, "#3D2411")

arrow(1035, 376, 1110, 376)
arrow(1035, 544, 1110, 544)

# ------------------------------------------------------- evaluation lane
def rq(y, h, tag, title_lines, question, chips, fill, stroke, cfill, cstroke):
    x, w = 1372, 326
    rrect(x, y, w, h, fill, stroke, r=11, sw=1.9)
    raw(x + 13, y + 20, tag.upper(), 10.6, "s", "n", "n", C_MUTE, "start", 1.1)
    for i, tl in enumerate(title_lines):
        rich_fit(x + w / 2, y + 42 + i * 20, tl, 15.8, w - 26)
    by = y + 42 + (len(title_lines) - 1) * 20 + 22
    rich_fit(x + w / 2, by, question, 12.8, w - 22, C_SOFT)
    for j, cs in enumerate(chips):
        chip_c(x + w / 2, by + 12 + j * 29, cs, 12.6, w - 28, cfill, cstroke)


rq(96, 180, "rq 1", [[("sc", "Fix Localization")]],
   [("i", "Does the slice contain the fix?")],
   [[("mi", "Coverage"), ("n", " = |S ∩ D(F)| / |D(F)|")],
    [("mi", "RSR"), ("n", " = |S| / n")],
    [("n", "vs. size-matched regions")]],
   C_RQ1, C_RQ1_S, "#C4DBF3", C_RQ1_S)

rq(290, 188, "rq 2", [[("sc", "Fix Signatures")]],
   [("i", "Do feature deltas identify fixes?")],
   [[("mi", "Δ"), ("n", " = f(after) \\ f(before), non-empty")],
    [("n", "φ vs. srcSlice profile")],
    [("n", "held-out matching: BA, MCC")]],
   C_RQ2, C_RQ2_S, "#EBD1E6", C_RQ2_S)

rq(492, 158, "rq 3", [[("sc", "Detection")]],
   [("i", "Does content or scope drive detection?")],
   [[("n", "scope × content, repeated CV")],
    [("n", "vs. frozen CodeBERT")]],
   C_RQ3, C_RQ3_S, "#DCDCDC", C_RQ3_S)

poly([(1295, 330), (1322, 330), (1322, 222), (1368, 222)])
poly([(1295, 430), (1322, 430), (1322, 384), (1368, 384)])
poly([(1295, 620), (1322, 620), (1322, 571), (1368, 571)])
rich(1346, 214, [("mi", "S")], 12.8, C_SOFT)

# ---------------------------------------------------------------- legend
lx, ly = 22, 318
raw(lx, ly, "Legend", 12.4, "s", "b", "n", C_SOFT, "start")
add(f'<line x1="{lx}" y1="{ly + 8}" x2="{lx + 160}" y2="{ly + 8}" '
    f'stroke="#D2D2DA" stroke-width="1"/>')
items = [(C_DOC, C_DOC_S, "input / ground truth"),
         (C_TOOL, C_TOOL_S, "external tool (srcML)"),
         (C_OURS, C_OURS_S, "our pipeline stage"),
         (C_OURS2, C_OURS2_S, "our derived artefact"),
         (C_COMB, C_COMB_S, "measured representation"),
         (C_RQ3, C_RQ3_S, "measurement / evaluation")]
for i, (f, s, lbl) in enumerate(items):
    y = ly + 22 + i * 24
    rrect(lx, y, 17, 12, f, s, r=3, sw=1.2)
    raw(lx + 24, y + 11, lbl, 11.9, "s", "n", "n", C_SOFT, "start")
add(f'<line x1="{lx}" y1="{ly + 22 + 6 * 24 + 8}" x2="{lx + 160}" '
    f'y2="{ly + 22 + 6 * 24 + 8}" stroke="#D2D2DA" stroke-width="1"/>')
raw(lx, ly + 22 + 6 * 24 + 26, "solid = data path", 11.4, "s", "n", "i",
    C_MUTE, "start")
raw(lx, ly + 22 + 6 * 24 + 42, "dashed = measurement", 11.4, "s", "n", "i",
    C_MUTE, "start")
raw(lx, ly + 22 + 6 * 24 + 56, "only (func_after)", 11.4, "s", "n", "i",
    C_MUTE, "start")

add('</svg>')
# In the replication package (a figures/ directory next to this script) the diagram is written to
# figures/method_diagram.*, the file the paper includes; elsewhere it is written as fig1.* here.
_HERE = os.path.dirname(os.path.abspath(__file__))
if os.path.isdir(os.path.join(_HERE, "figures")):
    OUT = os.path.join(_HERE, "figures", "method_diagram.svg")
else:
    OUT = os.path.join(_HERE, "fig1.svg")
open(OUT, "w").write("\n".join(out))
print("wrote", OUT)

# Optional: render straight to PDF/PNG if cairosvg is installed.
if __name__ == "__main__":
    try:
        import cairosvg
    except ImportError:
        print("pip install cairosvg (needs the cairo library) to also emit the PDF and PNG")
    else:
        base = OUT[:-4]
        cairosvg.svg2pdf(url=OUT, write_to=base + ".pdf")
        png = base + (".png" if base.endswith("method_diagram") else "_300dpi.png")
        cairosvg.svg2png(url=OUT, write_to=png, scale=3.0)
        print("wrote", base + ".pdf", "and", png)
