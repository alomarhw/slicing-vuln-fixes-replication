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

W, H = 1740, 880

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
add(f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
    f'viewBox="0 0 {W} {H}">')
add('<defs><marker id="ah" viewBox="0 0 10 10" refX="8.5" refY="5" '
    'markerWidth="6.5" markerHeight="6.5" orient="auto-start-reverse">'
    f'<path d="M 0 1.2 L 10 5 L 0 8.8 z" fill="{C_INK}"/></marker></defs>')
add(f'<rect width="{W}" height="{H}" fill="#FFFFFF"/>')

# ---------------------------------------------------------------- lanes
band(215, 58, 530, 212, C_BAND_GT, C_BAND_GT_S,
     [("i", "Ground truth \u2014 consulted for measurement only")])
band(215, 294, 865, 164, C_BAND_SCP, C_BAND_SCP_S,
     [("i", "Scope \u2014 dependency-aware backward slice, computed from "),
      ("m", "func_before"), ("i", " alone")])
band(215, 472, 865, 190, C_BAND_CNT, C_BAND_CNT_S,
     [("i", "Content \u2014 abstract structural features read off the srcML AST "
            "over that scope")])
band(1350, 58, 370, 782, C_BAND_EVL, C_BAND_EVL_S,
     [("i", "Controlled, non-circular evaluation")])

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
rich(968, 162, [("mi", "D(F)"),
                ("i", "  \u2014  lines deleted by the human fix;"),
                ("i", "  the patch is never available at slice time")], 13.6,
     C_CAP)

# thesis callout
rrect(772, 192, 308, 78, "#FFFFFF", "#B6B6C4", r=10, sw=1.3, dash="4,3")
rich(926, 215, [("b", "Scope is not content.")], 14.6, "#25252F")
rich(926, 235, [("n", "A per-variable profile records "), ("i", "which"),
                ("n", " variables")], 12.8, C_SOFT)
rich(926, 251, [("n", "lie in S; \u03c6 records "), ("i", "how"),
                ("n", " they are used.")], 12.8, C_SOFT)

# ------------------------------------------------------------ scope lane
stage(290, 324, 205, 96, "stage 1", [("sc", "Sink Criterion")],
      [[("n", "SySeVR-style predicate on the")],
       [("n", "tree-sitter AST: calls, subscripts,")],
       [("n", "*, &, ->, arithmetic")]], C_OURS, C_OURS_S)
stage(560, 324, 205, 96, "stage 2", [("sc", "Backward Slicer")],
      [[("n", "DEF/USE sets from a full AST walk;")],
       [("n", "unbounded worklist to a fixed point")],
       [("n", "over reaching defs + control deps")]], C_OURS, C_OURS_S)
stage(830, 324, 205, 96, "output",
      [("sc", "Slice "), ("mi", "S(F, c)")],
      [[("n", "the retained line set")],
       [("mi", "RSR"), ("n", " = |S| / n")],
       [("n", "intraprocedural, patch-blind")]], C_OURS2, C_OURS2_S)

arrow(495, 372, 554, 372)
arrow(765, 372, 824, 372)
rich(524, 363, [("mi", "c")], 13.6, C_SOFT)
rich(794, 363, [("mi", "S")], 13.6, C_SOFT)

rich(290, 434, [("i", "no line-distance cutoff, no aliasing analysis — "
                      "no-sink fallback on  "),
                ("b", "4.4 %")], 11.6, C_SOFT, "start")
rich(290, 449, [("i", "of functions (5 last-statement, 8 whole-function; 13/294); "
                      "matched-sink coverage 0.78 in RSR 0.56")], 11.6, C_SOFT, "start")

# function feed
poly([(250, 252), (250, 372), (284, 372)])
poly([(250, 372), (250, 554), (284, 554)])
rich(242, 464, [("mi", "F")], 14.5, C_INK, "end")

# ---------------------------------------------------------- content lane
stage(290, 506, 205, 96, "external tool", [("sc", "srcML Parse")],
      [[("n", "fact-preserving XML markup;")],
       [("n", "full parse tree A(F) without")],
       [("n", "requiring the code to compile")]], C_TOOL, C_TOOL_S)
stage(560, 506, 205, 96, "stage 3", [("sc", "AST Visitor")],
      [[("n", "project A(F) onto the scope S;")],
       [("n", "drop variable names; keep")],
       [("n", "callee & type names")]], C_OURS, C_OURS_S)
stage(830, 506, 205, 96, "output",
      [("sc", "Features "), ("mi", "\u03c6(F, S)")],
      [[("n", "operators \u00b7 guards \u00b7 literal")],
       [("n", "& type kinds \u00b7 callee/index")],
       [("n", "as a set of feature tokens")]], C_OURS2, C_OURS2_S)

arrow(495, 554, 554, 554)
arrow(765, 554, 824, 554)
rich(524, 545, [("mi", "A(F)")], 12.6, C_SOFT)
rich(794, 545, [("mi", "\u03c6")], 14.0, C_SOFT)

rich(248, 631, [("mi", "\u03c6 :")], 14.0, C_SOFT, "start")
cx = 282
for lbl in ["idx", "ctrl:if", "op:>", "callee:print_string", "op:+",
            "op:-", "call", "lit:number"]:
    cx += chip(cx, 615, [("mb", lbl)], 12.4) + 10

# --------------------------------------------- combined representation
rrect(1115, 302, 180, 360, C_COMB, C_COMB_S, r=13, sw=2.0)
rich(1205, 334, [("b", "R(F) = ")], 17.5, "#231205")
rich(1205, 364, [("mb", "( S , \u03c6 )")], 20.0, "#231205")
add(f'<line x1="1140" y1="382" x2="1270" y2="382" stroke="{C_COMB_S}" '
    f'stroke-width="1.2"/>')
rich(1205, 404, [("i", "scope"), ("m", " (+) "),
                 ("i", "content")], 14.0, "#3D2411")
rich(1205, 426, [("n", "content variants:")], 13.2, "#3D2411")
rich(1205, 442, [("n", "tokens / normalized / \u03c6")], 12.4, "#3D2411")
rrect(1136, 456, 138, 62, "#FFF5EE", "#C4885C", r=8, sw=1.3)
rich(1205, 476, [("n", "TF\u2013IDF over")], 12.6, C_SOFT)
rich(1205, 492, [("m", "the region\u2019s")], 12.4, C_SOFT)
rich(1205, 508, [("m", "content")], 12.4, C_SOFT)
rich(1205, 536, [("n", "class-weighted")], 13.0, "#3D2411")
rich(1205, 552, [("n", "logistic regression")], 13.0, "#3D2411")
rrect(1136, 566, 138, 62, "#FFF5EE", "#C4885C", r=8, sw=1.3)
rich(1205, 587, [("i", "CPU-only, no GPU;")], 12.6, C_SOFT)
rich(1205, 603, [("i", "interpretable and")], 12.6, C_SOFT)
rich(1205, 619, [("i", "millisecond-scale")], 12.6, C_SOFT)
rich(1205, 648, [("n", "\u2014 no patch is generated \u2014")], 12.4, C_MUTE)

arrow(1035, 372, 1110, 372)
arrow(1035, 554, 1110, 554)

# ------------------------------------------------------- evaluation lane
def rq(y, h, tag, title_lines, body, chips, fill, stroke, cfill, cstroke,
       note=None):
    x, w = 1372, 326
    rrect(x, y, w, h, fill, stroke, r=11, sw=1.9)
    raw(x + 13, y + 20, tag.upper(), 10.6, "s", "n", "n", C_MUTE, "start", 1.1)
    for i, tl in enumerate(title_lines):
        rich_fit(x + w / 2, y + 42 + i * 20, tl, 15.8, w - 26)
    by = y + 42 + (len(title_lines) - 1) * 20 + 22
    for i, ln in enumerate(body):
        rich_fit(x + w / 2, by + i * 16, ln, 12.5, w - 22, C_SOFT)
    cy = by + len(body) * 16 + 6
    for j, cs in enumerate(chips):
        chip_c(x + w / 2, cy + j * 29, cs, 12.8, w - 28, cfill, cstroke)
    if note:
        for k, nl in enumerate(note):
            rich_fit(x + w / 2, cy + len(chips) * 29 + 16 + k * 15, nl, 11.8,
                     w - 22, C_MUTE)


rq(100, 218, "rq 1", [[("sc", "Fix Localization")]],
   [[("mi", "S(F, c)"), ("n", "  vs.  "),
     ("m", "func_before->func_after"), ("n", "  diff")],
    [("n", "294 CVE pairs \u00b7 the slice never sees the patch")]],
   [[("b", "coverage 78.1 %"), ("n", "   \u00b7   "), ("b", "RSR 56.9 %")],
    [("n", "95 % bootstrap CI  [74.0, 82.1]  and  [54.2, 59.5]")],
    [("n", "median coverage 100 %  \u00b7  1.38\u00d7 lift over random")]],
   C_RQ1, C_RQ1_S, "#C4DBF3", C_RQ1_S,
   note=[[("i", "\u2248 var.-mention region \u00b7 81.7 % on 760 pairs, 89 % on 369 recent fixes")]])

rq(332, 250, "rq 2", [[("sc", "Patch-Signature")], [("sc", "Discrimination")]],
   [[("mi", "\u0394"), ("n", " = f(after) \\ f(before) over a region, non-empty")],
    [("n", "vs. srcSlice\u2019s coarse per-variable profile")],
    [("n", "dual signature: containment rule, 70 / 30 database-test")]],
   [[("n", "non-empty \u0394  "), ("b", "25.6 % -> 52.8 %"), ("n", "  (316 pairs)")],
    [("n", "exact McNemar  "), ("mb", "p = 6.2e-18"),
     ("n", "  \u00b7  98 gain / 12 lose")],
    [("n", "held-out matching  MCC  "), ("b", "\u22120.02 / 0.03"),
     ("n", "  (full split 0.00 / 0.06)")]],
   C_RQ2, C_RQ2_S, "#EBD1E6", C_RQ2_S,
   note=[[("i", "\u0394 reacts to any change; held-out matching at chance")]])

rq(596, 230, "rq 3", [[("sc", "Detection")]],
   [[("n", "repeated 30\u00d75-fold CV at 1:4, same classifier & splits")],
    [("n", "Nadeau\u2013Bengio corrected "), ("mi", "t"),
     ("n", "-test, Holm-corrected, over 150 splits")]],
   [[("n", "\u03c6 slice  "), ("b", "0.407"), ("n", " > plain slice 0.349  \u00b7  "
     "Holm "), ("mi", "p"), ("n", " = 0.011")],
    [("n", "normalized slice tokens "), ("b", "0.415"), ("n", " \u2248 \u03c6 slice ("), ("mi", "p"), ("n", " = 0.57)")],
    [("n", "frozen CodeBERT, same protocol  "), ("b", "0.504")]],
   C_RQ3, C_RQ3_S, "#DCDCDC", C_RQ3_S,
   note=[[("i", "scope: \u03c6 slice \u2248 var.-mention region, also with Joern"), ],
         [("i", "slices \u00b7 holds with CVE- and project-grouped folds")]])

poly([(1295, 342), (1322, 342), (1322, 200), (1368, 200)])
poly([(1295, 480), (1322, 480), (1322, 460), (1368, 460)])
poly([(1295, 620), (1322, 620), (1322, 700), (1368, 700)])
rich(1344, 192, [("mi", "S")], 12.8, C_SOFT, "end")

# ---------------------------------------------------------- worked example
rrect(215, 690, 865, 158, "#FCFBF8", "#C0C0CB", r=13, sw=1.3)
raw(233, 712, "Worked example (Sec. III-A) \u2014 CVE-2017-13006, tcpdump",
    13.4, "s", "b", "n", "#25252F", "start")

add('<rect x="233" y="717" width="384" height="122" rx="7" fill="#FFFFFF" '
    'stroke="#DCDCE4" stroke-width="1"/>')
add('<rect x="237" y="764" width="376" height="12" fill="#FBE0E0"/>')
add('<rect x="237" y="786" width="376" height="12" fill="#FBE0E0"/>')
code = ["l2tp_q931_cc_print(ndo, dat, length)",
        "{",
        "    print_16bits_val(ndo, dat);",
        "    ND_PRINT(ndo, \"%02x\", dat[2]);",
        "    if (length > 3) {",
        "        ND_PRINT(ndo, \" \");",
        "        print_string(ndo, dat+3, length-3);",
        "    }",
        "}"]
for i, ln in enumerate(code):
    raw(242, 728 + i * 11.4, ln, 8.8, "m", "n", "n", "#20202A", "start")
raw(470, 751, "<-  insert: length<3 guard", 9.6, "s", "n", "i",
    "#699A5D", "start")
raw(470, 774, "<-  deleted: if (length > 3), line 5", 9.6, "s", "n", "i",
    "#9A3B3B", "start")
raw(470, 796, "<-  deleted: print_string, line 7", 9.6, "s", "n", "i",
    "#9A3B3B", "start")

rich(640, 733, [("mb", "S = {3, 4, 5, 6, 7}"),
                ("n", "  of 9 lines")], 13.0, C_INK, "start")
rich(640, 750, [("n", "RSR "), ("m", "\u2248 0.56"),
                ("n", "  \u00b7  coverage "), ("m", "= 1.0"),
                ("n", "  (2 of 2 deleted lines in "),
                ("mi", "S"), ("n", ")")], 12.2, C_SOFT, "start")
rich(640, 771, [("i", "real fix inserts a "), ("m", "length < 3"),
                ("i", " guard at entry;"),
                ], 12.4, C_SOFT, "start")
rich(640, 785, [("i", "old "), ("m", "length > 3"),
                ("i", " guard becomes "), ("m", "length != 0")], 12.4, C_SOFT, "start")
# Actual phi delta over the slice for this fix (augmented_study.aug_slice(after) vs (before)).
c2 = 640
raw(c2, 810, "\u0394\u03c6:", 12.4, "s", "n", "n", C_INK, "start")
c2 += 34
for lbl in ("op:>", "op:+", "op:-"):                       # removed by the fix
    c2 += chip(c2, 793, [("mb", lbl)], 11.4, "#FBE0E0", "#BC6A6A") + 4
c2 += 8
for lbl in ("op:<", "op:!=", "op:+=", "op:-="):            # added by the fix
    c2 += chip(c2, 793, [("mb", lbl)], 11.4, "#DEF0DA", "#699A5D") + 4
rich(638, 833, [("n", "red: removed, green: added; a per-variable profile sees only "),
                ("m", "dat"), ("n", "/"), ("m", "length")], 11.6, C_MUTE, "start")


# ---------------------------------------------------------------- legend
lx, ly = 30, 612
raw(lx, ly, "Legend", 12.4, "s", "b", "n", C_SOFT, "start")
add(f'<line x1="{lx}" y1="{ly + 8}" x2="{lx + 160}" y2="{ly + 8}" '
    f'stroke="#D2D2DA" stroke-width="1"/>')
items = [(C_DOC, C_DOC_S, "input / ground truth"),
         (C_TOOL, C_TOOL_S, "external tool (srcML)"),
         (C_OURS, C_OURS_S, "our pipeline stage"),
         (C_OURS2, C_OURS2_S, "our derived artefact"),
         (C_COMB, C_COMB_S, "combined representation"),
         (C_RQ3, C_RQ3_S, "measurement / evaluation")]
for i, (f, s, lbl) in enumerate(items):
    y = ly + 22 + i * 24
    rrect(lx, y, 17, 12, f, s, r=3, sw=1.2)
    raw(lx + 24, y + 11, lbl, 11.9, "s", "n", "n", C_SOFT, "start")
add(f'<line x1="{lx}" y1="{ly + 22 + 6 * 24 + 8}" x2="{lx + 160}" '
    f'y2="{ly + 22 + 6 * 24 + 8}" stroke="#D2D2DA" stroke-width="1"/>')
raw(lx, ly + 22 + 6 * 24 + 26, "solid = data path", 11.4, "s", "n", "i",
    C_MUTE, "start")
raw(lx, ly + 22 + 6 * 24 + 42, "dashed = ground truth,", 11.4, "s", "n", "i",
    C_MUTE, "start")
raw(lx, ly + 22 + 6 * 24 + 56, "measurement only", 11.4, "s", "n", "i",
    C_MUTE, "start")

add('</svg>')
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "fig1.svg")
open(OUT, "w").write("\n".join(out))
print("wrote", OUT)

# Optional: render straight to PDF/PNG if cairosvg is installed.
if __name__ == "__main__":
    try:
        import cairosvg
    except ImportError:
        print("pip install cairosvg to also emit fig1.pdf / fig1_300dpi.png")
    else:
        base = OUT[:-4]
        cairosvg.svg2pdf(url=OUT, write_to=base + ".pdf")
        cairosvg.svg2png(url=OUT, write_to=base + "_300dpi.png", scale=3.0)
        print("wrote", base + ".pdf", "and", base + "_300dpi.png")
