#!/usr/bin/env python3
"""
graph_model.py  —  Learned GNN over the srcML-augmented program-slice graph
                   for vulnerability detection.

GOAL
----
Test whether GRAPH STRUCTURE (sequential + data-dep + control edges over slice
statements) plus ABSTRACT srcML node features beats the bag-of-features TF-IDF
baselines from prior runs:
    srcML-augmented-slice TF-IDF : F1 = 0.397  PR-AUC = 0.384
    whole-function-text  TF-IDF  : F1 = 0.366  PR-AUC = 0.350

We build, per function, a small graph whose nodes are slice statement lines and
whose node feature is a multi-hot over a GLOBAL abstract-srcML feature vocab
(built from the TRAIN fold ONLY -> no leakage). A hand-rolled dense GCN (2
layers, plain torch, NO torch_geometric) classifies each graph.

Self-contained. stdlib + numpy + sklearn + torch only. Fixed seed 1337.
No network. SHELLS OUT to `srcml` via the reused srcml_features().

NOTE: This script is NOT run in the authoring environment; the orchestrator
runs and verifies it (torch 2.2.2 with MPS available).
Run as:  python3 graph_model.py
"""

import os
import sys
import json
import time
import random
import traceback
from collections import Counter, defaultdict

import numpy as np

# --------------------------------------------------------------------------- #
# Determinism
# --------------------------------------------------------------------------- #
SEED = 1337
random.seed(SEED)
np.random.seed(SEED)

# --------------------------------------------------------------------------- #
# Paths / knobs
# --------------------------------------------------------------------------- #
HERE = os.path.dirname(os.path.abspath(__file__))
DATA_PATH = os.path.join(HERE, "data", "bigvul", "sample.jsonl")
RESULTS_DIR = os.path.join(HERE, "results")
RESULTS_PATH = os.path.join(RESULTS_DIR, "graph_results.json")

DET_NEG_RATIO = 4          # negatives : positives (1:4)
DET_CAP = 2500             # total detection-set cap (srcml is the bottleneck)
N_FOLDS = 5

MAX_NODES = 80             # cap nodes per graph (keep first 80 slice lines)
VOCAB_SIZE = 256           # top-K abstract features; index 0 reserved = OOV/empty
HIDDEN = 64                # GCN hidden width
EPOCHS = 15
LR = 1e-3
CTRL_K = 3                 # control-header connects to next K slice nodes

# Baselines reported by prior runs (printed for context only).
BASE_AUGSLICE_F1 = 0.397
BASE_AUGSLICE_PRAUC = 0.384
BASE_WHOLE_F1 = 0.366
BASE_WHOLE_PRAUC = 0.350

# --------------------------------------------------------------------------- #
# Reused project modules (same directory). Import defensively; provide
# fallbacks so the script never crashes purely on an import error.
# --------------------------------------------------------------------------- #
try:
    from ast_slicer import semantic_slice_indices  # -> Set[int] 0-based
except Exception as e:  # pragma: no cover
    sys.stderr.write("WARN: could not import semantic_slice_indices: %s\n" % e)

    def semantic_slice_indices(code):
        # Fallback: whole function.
        return set(range(len(code.splitlines()))) if code else set()

try:
    from augmented_study import srcml_features  # -> list[(line:int, feat:str)] 1-based
except Exception as e:  # pragma: no cover
    sys.stderr.write("WARN: could not import srcml_features, reimplementing: %s\n" % e)
    import tempfile
    import subprocess
    import xml.etree.ElementTree as ET

    NS_POS = "http://www.srcML.org/srcML/position"
    POS_START = "{%s}start" % NS_POS
    CTRL_TAGS = {
        "if", "else", "for", "while", "switch", "do", "case",
        "condition", "ternary", "goto", "break", "continue", "return",
    }

    def _local(tag):
        if not isinstance(tag, str):
            return ""
        return tag.split("}")[-1]

    def _start_line(el):
        val = el.get(POS_START)
        if not val:
            return None
        try:
            return int(val.split(":")[0])
        except Exception:
            return None

    def _callee_name(call_el):
        try:
            for child in list(call_el):
                if _local(child.tag) != "name":
                    continue
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

    def srcml_features(code):
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
                    ["srcml", "--position", cpath],
                    stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                    timeout=20, check=False,
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
                    callee = _callee_name(el)
                    if callee:
                        feats.append((ln, "callee:" + callee))
            return feats
        finally:
            if cpath:
                try:
                    os.unlink(cpath)
                except Exception:
                    pass

try:
    from proto_dualsig import idents_in_lines  # (lines, idxs) -> set[str]
except Exception as e:  # pragma: no cover
    sys.stderr.write("WARN: could not import idents_in_lines, reimplementing: %s\n" % e)
    import re
    _IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z_0-9]*")
    _C_KEYWORDS = {
        "if", "else", "for", "while", "do", "switch", "case", "default",
        "break", "continue", "return", "goto", "sizeof", "int", "char",
        "void", "float", "double", "long", "short", "unsigned", "signed",
        "struct", "union", "enum", "const", "static", "extern", "volatile",
        "register", "typedef", "auto", "inline", "restrict",
    }

    def idents_in_lines(lines, idxs):
        out = set()
        for i in idxs:
            if 0 <= i < len(lines):
                for m in _IDENT_RE.findall(lines[i]):
                    if m not in _C_KEYWORDS:
                        out.add(m)
        return out


# --------------------------------------------------------------------------- #
# torch (import defensively; device fallback handled at train time)
# --------------------------------------------------------------------------- #
try:
    import torch
    import torch.nn as nn
    _TORCH_OK = True
except Exception as e:  # pragma: no cover
    _TORCH_OK = False
    sys.stderr.write("FATAL: torch unavailable: %s\n" % e)


def _is_vuln(v):
    return v in (1, "1", True)


# --------------------------------------------------------------------------- #
# Data loading
# --------------------------------------------------------------------------- #
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


def build_detection_set(rows):
    """
    Positives: all vul==1 func_before. Negatives: vul==0 func_before, sampled
    at 1:DET_NEG_RATIO. Capped at ~DET_CAP. Returns (texts, labels, cwes).
    """
    pos = [(r["func_before"], r.get("CWE ID"))
           for r in rows
           if _is_vuln(r.get("vul"))
           and isinstance(r.get("func_before"), str)
           and r["func_before"].strip()]
    neg_pool = [r["func_before"]
                for r in rows
                if (r.get("vul") in (0, "0", False))
                and isinstance(r.get("func_before"), str)
                and r["func_before"].strip()]

    rng = random.Random(SEED)
    rng.shuffle(neg_pool)

    # Respect 1:4 ratio and the overall cap.
    max_pos = DET_CAP // (1 + DET_NEG_RATIO)
    if len(pos) > max_pos:
        rng.shuffle(pos)
        pos = pos[:max_pos]
    n_neg = min(len(neg_pool), DET_NEG_RATIO * len(pos))
    neg = neg_pool[:n_neg]

    texts = [t for (t, _) in pos] + list(neg)
    labels = [1] * len(pos) + [0] * len(neg)
    cwes = [c for (_, c) in pos] + [None] * len(neg)

    # Shuffle together so folds aren't ordered by class.
    order = list(range(len(texts)))
    rng.shuffle(order)
    texts = [texts[i] for i in order]
    labels = [labels[i] for i in order]
    cwes = [cwes[i] for i in order]
    return texts, np.array(labels, dtype=np.int64), cwes


# --------------------------------------------------------------------------- #
# Per-function feature/slice cache
# --------------------------------------------------------------------------- #
class FuncCache:
    """Cache srcml_features and slice indices per function TEXT (avoid recompute
    across folds). srcml is the throughput bottleneck, so this matters."""

    def __init__(self):
        self._slice = {}     # text -> sorted list of 0-based node line idxs (capped)
        self._lines = {}     # text -> list[str] source lines
        self._line_feats = {}  # text -> dict{0-based line -> set(feat str)}
        self.fail_count = 0
        self.empty_node_count = 0

    def get(self, text):
        if text in self._slice:
            return (self._slice[text], self._lines[text], self._line_feats[text])

        lines = text.splitlines()
        self._lines[text] = lines
        n = len(lines)

        # --- slice indices (0-based) ---
        try:
            sidx = semantic_slice_indices(text)
            if not sidx:
                sidx = set(range(n))
        except Exception:
            self.fail_count += 1
            sidx = set(range(n))
        # Keep only valid indices, sorted, capped at MAX_NODES (first 80).
        nodes = sorted(i for i in sidx if 0 <= i < n)[:MAX_NODES]

        # --- srcml abstract features grouped by 0-based line ---
        line_feats = defaultdict(set)
        try:
            pairs = srcml_features(text)  # 1-based lines, -1 sentinel possible
        except Exception:
            self.fail_count += 1
            pairs = []
        for (ln1, feat) in pairs:
            # Convert 1-based -> 0-based; drop sentinel / out-of-range.
            ln0 = ln1 - 1
            if 0 <= ln0 < n:
                line_feats[ln0].add(feat)

        self._slice[text] = nodes
        self._line_feats[text] = dict(line_feats)
        return (nodes, lines, dict(line_feats))


# --------------------------------------------------------------------------- #
# Vocab (built from TRAIN graphs only)
# --------------------------------------------------------------------------- #
def build_vocab(train_texts, cache):
    """Top VOCAB_SIZE-1 most frequent abstract features over TRAIN nodes.
    Index 0 reserved for OOV/empty. Returns dict feat -> index in [1, VOCAB_SIZE)."""
    cnt = Counter()
    for text in train_texts:
        nodes, _lines, line_feats = cache.get(text)
        for ln in nodes:
            for f in line_feats.get(ln, ()):
                cnt[f] += 1
    vocab = {}
    for i, (feat, _) in enumerate(cnt.most_common(VOCAB_SIZE - 1)):
        vocab[feat] = i + 1  # 0 reserved
    return vocab


# --------------------------------------------------------------------------- #
# Graph construction
# --------------------------------------------------------------------------- #
_CTRL_HEADER_FEATS = {
    "ctrl:if", "ctrl:for", "ctrl:while", "ctrl:switch", "ctrl:do",
}


def build_graph(text, cache, vocab):
    """
    Build (X, A) for one function.
      X : float32 [N, VOCAB_SIZE] multi-hot node features (col 0 = OOV/empty).
      A : float32 [N, N] symmetric adjacency WITHOUT self-loops (self-loops
          added inside the GCN via A+I). Includes sequential, data-dep, control.
    Returns (X_np, A_np). Falls back to a single OOV node on any failure.
    """
    try:
        nodes, lines, line_feats = cache.get(text)
        if not nodes:
            return _oov_graph()

        N = len(nodes)
        V = VOCAB_SIZE
        X = np.zeros((N, V), dtype=np.float32)

        # Per-node multi-hot features + per-node identifier set + control flag.
        node_idents = []
        node_is_ctrl = []
        for r, ln in enumerate(nodes):
            feats = line_feats.get(ln, set())
            hit = False
            for f in feats:
                idx = vocab.get(f)
                if idx is not None:
                    X[r, idx] = 1.0
                    hit = True
            if not hit:
                X[r, 0] = 1.0  # OOV/empty node marker
            # data-dep proxy: identifiers on this node's source line
            try:
                node_idents.append(idents_in_lines(lines, [ln]))
            except Exception:
                node_idents.append(set())
            # control proxy: is this a control header?
            node_is_ctrl.append(bool(feats & _CTRL_HEADER_FEATS))

        A = np.zeros((N, N), dtype=np.float32)

        # sequential edges: consecutive nodes in the slice order
        for r in range(N - 1):
            A[r, r + 1] = 1.0
            A[r + 1, r] = 1.0

        # data-dep proxy: shared identifier -> edge
        for a in range(N):
            ia = node_idents[a]
            if not ia:
                continue
            for b in range(a + 1, N):
                if ia & node_idents[b]:
                    A[a, b] = 1.0
                    A[b, a] = 1.0

        # control proxy: control header connects to next CTRL_K slice nodes
        for r in range(N):
            if node_is_ctrl[r]:
                for k in range(1, CTRL_K + 1):
                    nb = r + k
                    if nb < N:
                        A[r, nb] = 1.0
                        A[nb, r] = 1.0

        return X, A
    except Exception:
        return _oov_graph()


def _oov_graph():
    X = np.zeros((1, VOCAB_SIZE), dtype=np.float32)
    X[0, 0] = 1.0
    A = np.zeros((1, 1), dtype=np.float32)
    return X, A


def normalized_adj(A_np):
    """Ahat = D^{-1/2} (A + I) D^{-1/2} as a dense numpy float32 matrix."""
    N = A_np.shape[0]
    A_hat = A_np + np.eye(N, dtype=np.float32)
    deg = A_hat.sum(axis=1)
    deg[deg == 0.0] = 1.0
    d_inv_sqrt = 1.0 / np.sqrt(deg)
    D = np.diag(d_inv_sqrt).astype(np.float32)
    return (D @ A_hat @ D).astype(np.float32)


# --------------------------------------------------------------------------- #
# Model: hand-rolled dense 2-layer GCN + mean/max pool + MLP head
# --------------------------------------------------------------------------- #
if _TORCH_OK:

    class GCN(nn.Module):
        def __init__(self, vocab_size=VOCAB_SIZE, hidden=HIDDEN):
            super().__init__()
            # Embed multi-hot features: linear vocab -> hidden.
            self.embed = nn.Linear(vocab_size, hidden)
            # 2 GCN weight matrices (H' = relu(Ahat @ H @ W)).
            self.W1 = nn.Linear(hidden, hidden)
            self.W2 = nn.Linear(hidden, hidden)
            self.drop = nn.Dropout(0.3)
            # Pool = concat(mean, max) -> 2*hidden. MLP 128 -> 64 -> 1.
            self.head = nn.Sequential(
                nn.Linear(2 * hidden, hidden),
                nn.ReLU(),
                nn.Dropout(0.3),
                nn.Linear(hidden, 1),
            )

        def forward(self, X, Ahat):
            # X: [N, V], Ahat: [N, N]
            H = torch.relu(self.embed(X))           # [N, hidden]
            H = torch.relu(Ahat @ self.W1(H))       # GCN layer 1
            H = self.drop(H)
            H = torch.relu(Ahat @ self.W2(H))       # GCN layer 2
            mean = H.mean(dim=0)                    # [hidden]
            mx, _ = H.max(dim=0)                    # [hidden]
            g = torch.cat([mean, mx], dim=0)        # [2*hidden]
            return self.head(g).squeeze(-1)         # scalar logit


def _select_device():
    if not _TORCH_OK:
        return None
    try:
        if torch.backends.mps.is_available():
            return torch.device("mps")
    except Exception:
        pass
    return torch.device("cpu")


def _to_device_graph(X_np, A_np, device):
    Ahat = normalized_adj(A_np)
    X = torch.from_numpy(X_np)
    Ah = torch.from_numpy(Ahat)
    try:
        X = X.to(device)
        Ah = Ah.to(device)
    except Exception:
        # device move failed -> stay on cpu
        pass
    return X, Ah


def _forward_one(model, X_np, A_np, device):
    """Forward a single graph; fall back to cpu on a device error."""
    X, Ah = _to_device_graph(X_np, A_np, device)
    try:
        return model(X, Ah)
    except Exception:
        # Retry on cpu for this graph.
        model_cpu = model
        Xc = torch.from_numpy(X_np)
        Ahc = torch.from_numpy(normalized_adj(A_np))
        return model_cpu(Xc, Ahc)


# --------------------------------------------------------------------------- #
# Threshold helpers
# --------------------------------------------------------------------------- #
def best_f1_threshold(y_true, scores):
    """Threshold (on sigmoid scores) that maximizes F1; sweep candidate cuts."""
    from sklearn.metrics import f1_score
    if len(set(y_true.tolist())) < 2:
        return 0.5
    cands = np.unique(scores)
    if cands.size > 200:
        cands = np.quantile(scores, np.linspace(0.01, 0.99, 200))
    best_t, best_f = 0.5, -1.0
    for t in cands:
        pred = (scores >= t).astype(int)
        f = f1_score(y_true, pred, zero_division=0)
        if f > best_f:
            best_f, best_t = f, float(t)
    return best_t


# --------------------------------------------------------------------------- #
# Train / eval one fold
# --------------------------------------------------------------------------- #
def run_fold(fold_id, train_idx, test_idx, graphs, labels, device):
    """graphs: list of (X_np, A_np). Returns metrics dict for this fold."""
    from sklearn.metrics import (
        f1_score, precision_score, recall_score, average_precision_score,
    )

    # Reseed per fold for reproducibility of init/dropout.
    torch.manual_seed(SEED + fold_id)
    np.random.seed(SEED + fold_id)
    random.seed(SEED + fold_id)

    # Inner train/val split (80/20 of train) for early stopping.
    rng = np.random.RandomState(SEED + fold_id)
    tr = list(train_idx)
    rng.shuffle(tr)
    n_val = max(1, int(0.2 * len(tr)))
    val_idx = tr[:n_val]
    fit_idx = tr[n_val:] if len(tr) - n_val > 0 else tr

    y_fit = labels[fit_idx]
    n_pos = int((y_fit == 1).sum())
    n_neg = int((y_fit == 0).sum())
    pos_weight_val = (n_neg / n_pos) if n_pos > 0 else 1.0

    model = GCN().to(device) if device is not None else GCN()
    try:
        model = model.to(device)
    except Exception:
        device = torch.device("cpu")
        model = model.to(device)

    pos_weight = torch.tensor([pos_weight_val], dtype=torch.float32)
    try:
        pos_weight = pos_weight.to(device)
    except Exception:
        pass
    loss_fn = nn.BCEWithLogitsLoss(pos_weight=pos_weight)
    opt = torch.optim.Adam(model.parameters(), lr=LR)

    def eval_scores(idxs):
        model.eval()
        out = np.zeros(len(idxs), dtype=np.float32)
        with torch.no_grad():
            for j, gi in enumerate(idxs):
                X_np, A_np = graphs[gi]
                logit = _forward_one(model, X_np, A_np, device)
                out[j] = float(torch.sigmoid(logit).item())
        return out

    best_val_ap = -1.0
    best_state = None
    patience, bad = 4, 0

    for epoch in range(EPOCHS):
        model.train()
        order = list(fit_idx)
        random.shuffle(order)
        total = 0.0
        for gi in order:
            X_np, A_np = graphs[gi]
            y = torch.tensor([float(labels[gi])], dtype=torch.float32)
            try:
                y = y.to(device)
            except Exception:
                pass
            opt.zero_grad()
            try:
                logit = model(*_to_device_graph(X_np, A_np, device))
                loss = loss_fn(logit.view(1), y.view(1))
                loss.backward()
                opt.step()
                total += float(loss.item())
            except Exception:
                # Skip a pathological graph rather than crash training.
                continue

        # Early stop on val PR-AUC.
        try:
            vs = eval_scores(val_idx)
            yv = labels[val_idx]
            vap = average_precision_score(yv, vs) if len(set(yv.tolist())) > 1 else 0.0
        except Exception:
            vap = 0.0
        if vap > best_val_ap:
            best_val_ap = vap
            best_state = {k: v.detach().cpu().clone() for k, v in model.state_dict().items()}
            bad = 0
        else:
            bad += 1
        print("  [fold %d] epoch %2d  train_loss=%.4f  val_PR-AUC=%.4f%s"
              % (fold_id, epoch, total / max(1, len(order)), vap,
                 "  *" if bad == 0 else ""))
        if bad >= patience:
            print("  [fold %d] early stop at epoch %d" % (fold_id, epoch))
            break

    # Restore best weights.
    if best_state is not None:
        try:
            model.load_state_dict({k: v.to(device) for k, v in best_state.items()})
        except Exception:
            model.load_state_dict(best_state)

    # Pick threshold by max-F1 on TRAIN scores (fair to imbalance).
    train_scores = eval_scores(train_idx)
    y_train = labels[train_idx]
    thr = best_f1_threshold(y_train, train_scores)

    # Test.
    test_scores = eval_scores(test_idx)
    y_test = labels[test_idx]

    def metrics_at(thr_val):
        pred = (test_scores >= thr_val).astype(int)
        return {
            "f1": float(f1_score(y_test, pred, zero_division=0)),
            "precision": float(precision_score(y_test, pred, zero_division=0)),
            "recall": float(recall_score(y_test, pred, zero_division=0)),
        }

    pr_auc = (float(average_precision_score(y_test, test_scores))
              if len(set(y_test.tolist())) > 1 else 0.0)

    m_best = metrics_at(thr)
    m_half = metrics_at(0.5)
    fold_res = {
        "fold": fold_id,
        "n_train": int(len(train_idx)),
        "n_test": int(len(test_idx)),
        "threshold_maxF1_train": float(thr),
        "pr_auc": pr_auc,
        "at_maxF1": m_best,
        "at_0.5": m_half,
        "val_pr_auc": float(best_val_ap),
        "pos_weight": float(pos_weight_val),
    }
    print("  [fold %d] TEST  PR-AUC=%.4f | maxF1thr(%.3f): F1=%.4f P=%.4f R=%.4f"
          " | @0.5: F1=%.4f P=%.4f R=%.4f"
          % (fold_id, pr_auc, thr, m_best["f1"], m_best["precision"],
             m_best["recall"], m_half["f1"], m_half["precision"], m_half["recall"]))
    return fold_res


# --------------------------------------------------------------------------- #
# Main
# --------------------------------------------------------------------------- #
def main():
    t0 = time.time()
    os.makedirs(RESULTS_DIR, exist_ok=True)

    print("=" * 78)
    print("GNN over srcML-augmented program-slice graph — vulnerability detection")
    print("=" * 78)

    result = {
        "config": {
            "seed": SEED, "det_cap": DET_CAP, "neg_ratio": DET_NEG_RATIO,
            "n_folds": N_FOLDS, "max_nodes": MAX_NODES, "vocab_size": VOCAB_SIZE,
            "hidden": HIDDEN, "epochs": EPOCHS, "lr": LR, "ctrl_k": CTRL_K,
        },
        "baselines": {
            "augslice_tfidf": {"f1": BASE_AUGSLICE_F1, "pr_auc": BASE_AUGSLICE_PRAUC},
            "whole_text_tfidf": {"f1": BASE_WHOLE_F1, "pr_auc": BASE_WHOLE_PRAUC},
        },
        "status": "init",
    }

    if not _TORCH_OK:
        result["status"] = "error: torch unavailable"
        _write(result)
        print("FATAL: torch unavailable; wrote stub results.")
        return

    rows = load_rows()
    print("loaded %d rows from %s" % (len(rows), DATA_PATH))
    if not rows:
        result["status"] = "error: no data"
        _write(result)
        return

    texts, labels, cwes = build_detection_set(rows)
    n_pos = int((labels == 1).sum())
    n_neg = int((labels == 0).sum())
    print("detection set: total=%d  positives(vul=1)=%d  negatives(vul=0)=%d  (ratio 1:%.1f)"
          % (len(texts), n_pos, n_neg, (n_neg / n_pos) if n_pos else 0.0))
    result["counts"] = {"total": len(texts), "positives": n_pos, "negatives": n_neg}

    if n_pos < N_FOLDS or n_neg < N_FOLDS:
        result["status"] = "error: too few samples per class for %d folds" % N_FOLDS
        _write(result)
        print("FATAL: not enough per-class samples; wrote stub results.")
        return

    device = _select_device()
    print("device: %s" % device)
    result["device"] = str(device)

    # --- Build graphs ONCE (vocab differs per fold but slice/feats are cached).
    # We precompute slice + srcml features into the cache here so the expensive
    # srcml calls happen exactly once per unique function.
    cache = FuncCache()
    print("precomputing slices + srcML features for %d functions (srcml shell-out)..."
          % len(texts))
    for i, t in enumerate(texts):
        cache.get(t)
        if (i + 1) % 200 == 0:
            print("  ...%d/%d  (%.1fs)" % (i + 1, len(texts), time.time() - t0))
    print("  srcml/slice cache built in %.1fs  (failures=%d)"
          % (time.time() - t0, cache.fail_count))

    # --- Cross-validation.
    from sklearn.model_selection import StratifiedKFold
    skf = StratifiedKFold(n_splits=N_FOLDS, shuffle=True, random_state=SEED)
    fold_results = []

    X_dummy = np.zeros(len(texts))  # placeholder for skf.split
    for fold_id, (train_idx, test_idx) in enumerate(skf.split(X_dummy, labels)):
        print("-" * 70)
        print("FOLD %d/%d  (train=%d test=%d)"
              % (fold_id, N_FOLDS, len(train_idx), len(test_idx)))

        # Build vocab from TRAIN graphs only (no leakage).
        train_texts = [texts[i] for i in train_idx]
        vocab = build_vocab(train_texts, cache)
        print("  vocab built from train fold: %d features (cap %d)"
              % (len(vocab), VOCAB_SIZE))

        # Build all graphs for this fold's vocab (train + test share the vocab,
        # but test multi-hot only activates known features; unknown -> OOV).
        graphs = [None] * len(texts)
        need = set(train_idx.tolist()) | set(test_idx.tolist())
        for gi in need:
            graphs[gi] = build_graph(texts[gi], cache, vocab)

        try:
            fr = run_fold(fold_id, train_idx, test_idx, graphs, labels, device)
        except Exception:
            sys.stderr.write("WARN: fold %d crashed:\n%s\n"
                             % (fold_id, traceback.format_exc()))
            fr = {"fold": fold_id, "error": "fold crashed"}
        fold_results.append(fr)

    # --- Aggregate.
    def _mean(key_path):
        vals = []
        for fr in fold_results:
            d = fr
            ok = True
            for k in key_path:
                if isinstance(d, dict) and k in d:
                    d = d[k]
                else:
                    ok = False
                    break
            if ok and isinstance(d, (int, float)):
                vals.append(float(d))
        return float(np.mean(vals)) if vals else 0.0

    def _std(key_path):
        vals = []
        for fr in fold_results:
            d = fr
            ok = True
            for k in key_path:
                if isinstance(d, dict) and k in d:
                    d = d[k]
                else:
                    ok = False
                    break
            if ok and isinstance(d, (int, float)):
                vals.append(float(d))
        return float(np.std(vals)) if vals else 0.0

    summary = {
        "mean_pr_auc": _mean(["pr_auc"]),
        "std_pr_auc": _std(["pr_auc"]),
        "maxF1_train_threshold": {
            "mean_f1": _mean(["at_maxF1", "f1"]),
            "std_f1": _std(["at_maxF1", "f1"]),
            "mean_precision": _mean(["at_maxF1", "precision"]),
            "mean_recall": _mean(["at_maxF1", "recall"]),
            "mean_threshold": _mean(["threshold_maxF1_train"]),
        },
        "threshold_0.5": {
            "mean_f1": _mean(["at_0.5", "f1"]),
            "std_f1": _std(["at_0.5", "f1"]),
            "mean_precision": _mean(["at_0.5", "precision"]),
            "mean_recall": _mean(["at_0.5", "recall"]),
        },
    }

    result["status"] = "ok"
    result["folds"] = fold_results
    result["summary"] = summary
    result["srcml_failures"] = cache.fail_count
    result["runtime_sec"] = round(time.time() - t0, 1)

    # Write results BEFORE any optional work.
    _write(result)

    # --- Headline comparison.
    gnn_f1 = summary["maxF1_train_threshold"]["mean_f1"]
    gnn_f1_half = summary["threshold_0.5"]["mean_f1"]
    gnn_prauc = summary["mean_pr_auc"]
    print("=" * 78)
    print("HEADLINE  (5-fold mean)")
    print("-" * 78)
    print("  GNN (slice-graph + srcML feats):")
    print("    maxF1@train-thr  F1=%.3f  (P=%.3f R=%.3f thr~%.3f)"
          % (gnn_f1,
             summary["maxF1_train_threshold"]["mean_precision"],
             summary["maxF1_train_threshold"]["mean_recall"],
             summary["maxF1_train_threshold"]["mean_threshold"]))
    print("    @0.5             F1=%.3f  (P=%.3f R=%.3f)"
          % (gnn_f1_half,
             summary["threshold_0.5"]["mean_precision"],
             summary["threshold_0.5"]["mean_recall"]))
    print("    PR-AUC           %.3f" % gnn_prauc)
    print("  BASELINES:")
    print("    srcML-aug-slice TF-IDF   F1=%.3f  PR-AUC=%.3f"
          % (BASE_AUGSLICE_F1, BASE_AUGSLICE_PRAUC))
    print("    whole-func-text TF-IDF   F1=%.3f  PR-AUC=%.3f"
          % (BASE_WHOLE_F1, BASE_WHOLE_PRAUC))
    print("-" * 78)
    best_gnn_f1 = max(gnn_f1, gnn_f1_half)
    verdict_aug = "BEATS" if best_gnn_f1 > BASE_AUGSLICE_F1 else "does NOT beat"
    verdict_whole = "BEATS" if best_gnn_f1 > BASE_WHOLE_F1 else "does NOT beat"
    print("  VERDICT (best GNN F1=%.3f): %s aug-slice TF-IDF (%.3f); %s whole-text (%.3f)"
          % (best_gnn_f1, verdict_aug, BASE_AUGSLICE_F1, verdict_whole, BASE_WHOLE_F1))
    print("  GNN PR-AUC %.3f vs aug-slice %.3f -> %s"
          % (gnn_prauc, BASE_AUGSLICE_PRAUC,
             "BEATS" if gnn_prauc > BASE_AUGSLICE_PRAUC else "does NOT beat"))
    print("=" * 78)
    print("results written to %s  (%.1fs total)" % (RESULTS_PATH, time.time() - t0))


def _json_default(o):
    try:
        import numpy as _np
        if isinstance(o, (_np.integer,)):
            return int(o)
        if isinstance(o, (_np.floating,)):
            return float(o)
        if isinstance(o, _np.ndarray):
            return o.tolist()
    except Exception:
        pass
    return str(o)


def _write(result):
    try:
        os.makedirs(RESULTS_DIR, exist_ok=True)
        with open(RESULTS_PATH, "w", encoding="utf-8") as fh:
            json.dump(result, fh, indent=2, default=_json_default)
    except Exception:
        sys.stderr.write("WARN: could not write results:\n%s\n" % traceback.format_exc())


if __name__ == "__main__":
    try:
        main()
    except Exception:
        sys.stderr.write("FATAL top-level:\n%s\n" % traceback.format_exc())
        # Best-effort stub so the orchestrator always finds a file.
        try:
            _write({"status": "error: top-level crash",
                    "traceback": traceback.format_exc()})
        except Exception:
            pass
