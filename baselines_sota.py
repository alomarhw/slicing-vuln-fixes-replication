#!/usr/bin/env python3
"""
baselines_sota.py — SOTA-FAMILY baseline comparison on the SAME Big-Vul split.

PURPOSE
-------
Position our srcML-augmented-slice representation against the *families* of the
state-of-the-art function-level vulnerability detectors, run on the identical
Big-Vul detection split used by augmented_study.py (RQ3).

HONESTY NOTE (read this — it governs how the results must be cited)
-------------------------------------------------------------------
We CANNOT run the authors' original tools. Devign / ReVeal need Joern or CodeQL
to build code-property graphs; we have neither in this environment. So these are
NOT the published systems. They are controlled, REPRESENTATION-FAMILY
re-implementations on a common split:

  * TextCNN          -> token-CNN detectors (Russell/Draper family)
  * BiGRU            -> RNN token detectors  (VulDeePecker/SySeVR family, func-level)
  * CodeBERT-frozen  -> pretrained-transformer detectors (LineVul family),
                        frozen encoder + linear probe (NOT fine-tuned)

Every output row is labelled "re-impl" or "ref" so nothing is mistaken for a
re-run of the original papers. The point is *relative positioning* of the
representation, on one fixed split, not leaderboard reproduction.

DATASET / PROTOCOL  (must match augmented_study.py exactly)
-----------------------------------------------------------
We literally re-use augmented_study.build_detection_set() if importable; if the
import fails we fall back to an inlined copy with identical logic:
  positives = all vul==1 func_before (non-empty str)
  negatives = vul==0 func_before (non-empty str), sampled 1:4
  cap total 2500 ; seed 1337 ; the same random.Random(1337) shuffle sequence
  StratifiedKFold(5, shuffle=True, random_state=1337)
Metrics: mean F1, PR-AUC (average_precision_score), precision, recall.
Threshold: best-F1 swept on TRAIN scores, AND reported @0.5. Scores are
sigmoid/proba so PR-AUC is well defined.

Self-contained. Wrapped end-to-end; never crashes; CPU fallback; per-baseline
status. Caches CodeBERT tokenization+embedding per unique function.

Run as:  python3 baselines_sota.py
"""

import os
import sys
import json
import random
import traceback

import numpy as np

# --------------------------------------------------------------------------- #
# Constants — mirror augmented_study.py
# --------------------------------------------------------------------------- #
SEED = 1337
random.seed(SEED)
np.random.seed(SEED)

HERE = os.path.dirname(os.path.abspath(__file__))
DATA_PATH = os.path.join(HERE, "data", "bigvul", "sample.jsonl")
RESULTS_DIR = os.path.join(HERE, "results")
RESULTS_PATH = os.path.join(RESULTS_DIR, "baselines_results.json")

DET_NEG_RATIO = 4        # negatives : positives (1:4)        — matches aug study
DET_CAP = 2500           # total detection-set cap            — matches aug study
N_FOLDS = 5

# Modest deep-net budget — total wall-clock matters.
MAX_EPOCHS = 10
EMBED_DIM = 128
VOCAB_SIZE = 10000
BATCH_SIZE = 64
LR = 1e-3
EARLY_STOP_PATIENCE = 2
CODEBERT_MAX_LEN = 512
CODEBERT_BATCH = 16
CODEBERT_MODEL = "microsoft/codebert-base"

# Reference rows (already computed elsewhere; printed as context, NOT recomputed).
REFERENCE_ROWS = [
    ("Ours srcML-aug-slice TF-IDF",  0.397, 0.384, "ours (ref)"),
    ("Ours srcML-aug-whole TF-IDF",  0.411, 0.400, "ours (ref)"),
    ("Ours aug-slice GNN",           0.368, 0.341, "ours (ref)"),
    ("whole-function-text TF-IDF",   0.366, 0.350, "ref"),
    ("plain slice-text TF-IDF",      0.335, 0.327, "ref"),
]


# --------------------------------------------------------------------------- #
# Data loading + detection-set construction (identical to augmented_study.py)
# --------------------------------------------------------------------------- #
def _is_vuln(v):
    return v in (1, "1", True)


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


def _build_detection_set_inline(rows):
    """
    EXACT copy of augmented_study.build_detection_set so the split is identical
    even if the import fails. Do not "improve" this — fidelity is the point.
    """
    pos = [r["func_before"] for r in rows
           if _is_vuln(r.get("vul")) and isinstance(r.get("func_before"), str)
           and r["func_before"].strip()]
    neg_pool = [r["func_before"] for r in rows
                if (r.get("vul") in (0, "0", False))
                and isinstance(r.get("func_before"), str)
                and r["func_before"].strip()]

    rng = random.Random(SEED)
    rng.shuffle(pos)
    rng.shuffle(neg_pool)

    max_pos = max(1, DET_CAP // (1 + DET_NEG_RATIO))
    n_pos = min(len(pos), max_pos)
    n_neg = min(len(neg_pool), n_pos * DET_NEG_RATIO)
    pos = pos[:n_pos]
    neg = neg_pool[:n_neg]

    codes = pos + neg
    labels = [1] * len(pos) + [0] * len(neg)
    idx = list(range(len(codes)))
    rng.shuffle(idx)
    codes = [codes[i] for i in idx]
    labels = [labels[i] for i in idx]
    return codes, labels


def build_detection_set(rows):
    """
    Prefer the real function from augmented_study.py (single source of truth);
    fall back to the inlined identical copy if importing it pulls in something
    unavailable in this environment (e.g. its srcML-dependent imports).
    """
    try:
        sys.path.insert(0, HERE)
        from augmented_study import build_detection_set as real_bds  # noqa
        codes, labels = real_bds(rows)
        print("[split] using augmented_study.build_detection_set (imported)")
        return codes, labels
    except Exception as e:
        print("[split] import of augmented_study failed (%r); using inlined "
              "identical copy" % e)
        return _build_detection_set_inline(rows)


# --------------------------------------------------------------------------- #
# Tokenizer / vocab for the two from-scratch token models
# --------------------------------------------------------------------------- #
PAD, UNK = 0, 1


def code_tokens(src):
    """
    Simple code lexer: split on non-alphanumeric, but keep operator/punctuation
    runs as their own tokens. lowercase=False (case matters in C/C++ idents).
    """
    if not src:
        return []
    toks = []
    cur = []
    for ch in src:
        if ch.isalnum() or ch == "_":
            cur.append(ch)
        else:
            if cur:
                toks.append("".join(cur))
                cur = []
            if not ch.isspace():
                toks.append(ch)              # operator / punctuation as a token
    if cur:
        toks.append("".join(cur))
    return toks


def build_vocab(corpus_tokens, max_size=VOCAB_SIZE):
    """Vocab from TRAIN tokens only. 0=PAD, 1=UNK, then top (max_size-2)."""
    from collections import Counter
    cnt = Counter()
    for toks in corpus_tokens:
        cnt.update(toks)
    vocab = {"<pad>": PAD, "<unk>": UNK}
    for tok, _ in cnt.most_common(max(0, max_size - 2)):
        vocab[tok] = len(vocab)
    return vocab


def encode(toks, vocab, max_len):
    ids = [vocab.get(t, UNK) for t in toks[:max_len]]
    if len(ids) < max_len:
        ids = ids + [PAD] * (max_len - len(ids))
    return ids


# --------------------------------------------------------------------------- #
# Metric helpers
# --------------------------------------------------------------------------- #
def _sigmoid(x):
    return 1.0 / (1.0 + np.exp(-np.clip(x, -60.0, 60.0)))


def best_f1_threshold(y_true, scores):
    """Sweep candidate thresholds (from the score distribution) for max F1."""
    from sklearn.metrics import f1_score
    y_true = np.asarray(y_true)
    scores = np.asarray(scores, dtype=float)
    if scores.size == 0:
        return 0.5, 0.0
    cands = np.unique(scores)
    if cands.size > 256:
        cands = np.quantile(scores, np.linspace(0.0, 1.0, 256))
    best_t, best_f1 = 0.5, -1.0
    for t in cands:
        f1 = f1_score(y_true, (scores >= t).astype(int), zero_division=0)
        if f1 > best_f1:
            best_f1, best_t = f1, float(t)
    return best_t, best_f1


def fold_metrics(y_te, proba_te, thr):
    """F1/precision/recall at a given threshold + PR-AUC (threshold-free)."""
    from sklearn.metrics import (f1_score, precision_score, recall_score,
                                 average_precision_score)
    pred = (np.asarray(proba_te) >= thr).astype(int)
    try:
        prauc = float(average_precision_score(y_te, proba_te))
    except Exception:
        prauc = 0.0
    return {
        "f1": float(f1_score(y_te, pred, zero_division=0)),
        "precision": float(precision_score(y_te, pred, zero_division=0)),
        "recall": float(recall_score(y_te, pred, zero_division=0)),
        "pr_auc": prauc,
    }


def _mean(xs):
    return float(np.mean(xs)) if xs else 0.0


def aggregate(per_fold_maxf1, per_fold_at05):
    """Mean across folds, for both the max-F1 and the @0.5 threshold sets."""
    def agg(rows):
        if not rows:
            return {"f1": 0.0, "pr_auc": 0.0, "precision": 0.0, "recall": 0.0}
        return {k: _mean([r[k] for r in rows])
                for k in ("f1", "pr_auc", "precision", "recall")}
    return {
        "maxF1_thresh": agg(per_fold_maxf1),
        "at_0.5": agg(per_fold_at05),
        "n_folds": len(per_fold_maxf1),
    }


# --------------------------------------------------------------------------- #
# Torch device resolution (mps -> cpu fallback), lazy + defensive
# --------------------------------------------------------------------------- #
def get_torch_device():
    import torch
    try:
        if hasattr(torch.backends, "mps") and torch.backends.mps.is_available():
            # Smoke-test a tiny op on mps; fall back to cpu if it errors.
            try:
                _ = (torch.ones(2, device="mps") + 1).sum().item()
                return torch.device("mps")
            except Exception:
                return torch.device("cpu")
    except Exception:
        pass
    return torch.device("cpu")


# --------------------------------------------------------------------------- #
# BASELINE 1 & 2 — shared torch training harness for token models
# --------------------------------------------------------------------------- #
def _train_token_model(model_factory, codes, y, folds, max_len, name):
    """
    Generic CV loop for a from-scratch torch token model.
    model_factory(vocab_size) -> nn.Module producing a single logit per sample.
    Returns aggregated metrics dict (or raises -> caught by caller).
    """
    import torch
    import torch.nn as nn

    device = get_torch_device()
    print("  [%s] device=%s" % (name, device))

    # Tokenize once (cache per unique function — functions recur across rows).
    tok_cache = {}

    def toks_for(c):
        t = tok_cache.get(c)
        if t is None:
            t = code_tokens(c)
            tok_cache[c] = t
        return t

    all_tokens = [toks_for(c) for c in codes]
    y = np.asarray(y, dtype=int)

    per_maxf1, per_05 = [], []
    for fi, (tr, te) in enumerate(folds):
        try:
            vocab = build_vocab([all_tokens[i] for i in tr], VOCAB_SIZE)

            # Carve a small val set out of train for early stopping (stratified-ish).
            rng = np.random.RandomState(SEED + fi)
            tr_perm = rng.permutation(tr)
            n_val = max(1, int(0.15 * len(tr_perm)))
            val_idx = tr_perm[:n_val]
            fit_idx = tr_perm[n_val:]

            def make_tensor(idxs):
                X = np.asarray(
                    [encode(all_tokens[i], vocab, max_len) for i in idxs],
                    dtype=np.int64)
                return (torch.from_numpy(X),
                        torch.from_numpy(y[idxs].astype(np.float32)))

            Xfit, yfit = make_tensor(fit_idx)
            Xval, yval = make_tensor(val_idx)
            Xte, _ = make_tensor(te)

            n_pos = float(max(1, int(y[fit_idx].sum())))
            n_neg = float(max(1, len(fit_idx) - int(y[fit_idx].sum())))
            pos_weight = torch.tensor([n_neg / n_pos], device=device)

            model = model_factory(len(vocab)).to(device)
            opt = torch.optim.Adam(model.parameters(), lr=LR)
            loss_fn = nn.BCEWithLogitsLoss(pos_weight=pos_weight)

            from sklearn.metrics import average_precision_score
            best_val_prauc, best_state, patience = -1.0, None, 0

            def iterate(X, ylab, train):
                model.train(train)
                order = (torch.randperm(len(X)) if train
                         else torch.arange(len(X)))
                ctx = torch.enable_grad() if train else torch.no_grad()
                with ctx:
                    for s in range(0, len(X), BATCH_SIZE):
                        bidx = order[s:s + BATCH_SIZE]
                        xb = X[bidx].to(device)
                        logits = model(xb).squeeze(-1)
                        if train:
                            yb = ylab[bidx].to(device)
                            loss = loss_fn(logits, yb)
                            opt.zero_grad()
                            loss.backward()
                            opt.step()
                        else:
                            yield logits.detach().float().cpu().numpy()

            for ep in range(MAX_EPOCHS):
                for _ in iterate(Xfit, yfit, True):
                    pass
                val_logits = np.concatenate(
                    list(iterate(Xval, yval, False))) if len(Xval) else np.array([])
                val_proba = _sigmoid(val_logits)
                try:
                    vp = float(average_precision_score(
                        yval.numpy(), val_proba)) if len(val_proba) else 0.0
                except Exception:
                    vp = 0.0
                if vp > best_val_prauc:
                    best_val_prauc = vp
                    best_state = {k: v.detach().cpu().clone()
                                  for k, v in model.state_dict().items()}
                    patience = 0
                else:
                    patience += 1
                    if patience >= EARLY_STOP_PATIENCE:
                        break

            if best_state is not None:
                model.load_state_dict(best_state)

            # Train scores -> max-F1 threshold; test scores -> metrics.
            tr_logits = np.concatenate(list(iterate(Xfit, yfit, False)))
            tr_proba = _sigmoid(tr_logits)
            te_logits = np.concatenate(list(iterate(Xte, None, False)))
            te_proba = _sigmoid(te_logits)

            thr, _ = best_f1_threshold(y[fit_idx], tr_proba)
            per_maxf1.append(fold_metrics(y[te], te_proba, thr))
            per_05.append(fold_metrics(y[te], te_proba, 0.5))
            print("    fold %d: maxF1@%.3f F1=%.3f PR-AUC=%.3f | @0.5 F1=%.3f"
                  % (fi, thr, per_maxf1[-1]["f1"], per_maxf1[-1]["pr_auc"],
                     per_05[-1]["f1"]))
        except Exception as e:
            print("    fold %d FAILED: %r" % (fi, e))
            continue

    if not per_maxf1:
        raise RuntimeError("all folds failed for %s" % name)
    return aggregate(per_maxf1, per_05)


def _textcnn_factory(max_len):
    import torch
    import torch.nn as nn

    class TextCNN(nn.Module):
        def __init__(self, vocab_size):
            super().__init__()
            self.emb = nn.Embedding(vocab_size, EMBED_DIM, padding_idx=PAD)
            self.convs = nn.ModuleList([
                nn.Conv1d(EMBED_DIM, 100, k) for k in (3, 4, 5)
            ])
            self.drop = nn.Dropout(0.5)
            self.fc = nn.Linear(300, 1)

        def forward(self, x):
            e = self.emb(x).transpose(1, 2)          # (B, E, L)
            pooled = []
            for conv in self.convs:
                c = torch.relu(conv(e))              # (B, 100, L')
                p = torch.max(c, dim=2).values       # global max-pool
                pooled.append(p)
            h = self.drop(torch.cat(pooled, dim=1))  # (B, 300)
            return self.fc(h)

    return lambda vocab_size: TextCNN(vocab_size)


def _bigru_factory(max_len):
    import torch
    import torch.nn as nn

    class BiGRU(nn.Module):
        def __init__(self, vocab_size):
            super().__init__()
            self.emb = nn.Embedding(vocab_size, EMBED_DIM, padding_idx=PAD)
            self.gru = nn.GRU(EMBED_DIM, 128, batch_first=True,
                              bidirectional=True)
            self.drop = nn.Dropout(0.5)
            self.fc = nn.Linear(128 * 2 * 2, 1)      # [last ; mean] over 2 dirs

        def forward(self, x):
            e = self.emb(x)                          # (B, L, E)
            out, h = self.gru(e)                     # out: (B, L, 2*128)
            last = out[:, -1, :]                     # last timestep
            mean = out.mean(dim=1)                   # mean over timesteps
            h = self.drop(torch.cat([last, mean], dim=1))
            return self.fc(h)

    return lambda vocab_size: BiGRU(vocab_size)


def run_textcnn(codes, y, folds):
    return _train_token_model(_textcnn_factory(400), codes, y, folds,
                              max_len=400, name="TextCNN")


def run_bigru(codes, y, folds):
    return _train_token_model(_bigru_factory(400), codes, y, folds,
                              max_len=400, name="BiGRU")


# --------------------------------------------------------------------------- #
# BASELINE 3 — frozen CodeBERT embedding + linear probe (LineVul family)
# --------------------------------------------------------------------------- #
def run_codebert(codes, y, folds):
    """
    Load CodeBERT (try/except, SHORT effort). If unavailable -> status row,
    skip gracefully. If it loads: frozen forward on (mps/cpu), mean-pool
    last_hidden_state -> 768-d, cache per unique function, then per-fold
    sklearn LogisticRegression(class_weight="balanced"). NOT fine-tuned.
    """
    try:
        import torch
        from transformers import AutoTokenizer, AutoModel
    except Exception as e:
        return {"status": "unavailable - transformers/torch import failed: %r" % e}

    try:
        tokenizer = AutoTokenizer.from_pretrained(CODEBERT_MODEL)
        model = AutoModel.from_pretrained(CODEBERT_MODEL)
    except Exception as e:
        # No network / no cached weights -> graceful skip (the expected risk).
        return {"status": "unavailable - no model weights",
                "detail": repr(e)[:300]}

    try:
        device = get_torch_device()
        model.eval().to(device)
        print("  [CodeBERT] loaded %s on device=%s" % (CODEBERT_MODEL, device))

        # Embed each UNIQUE function once (cache).
        uniq = list(dict.fromkeys(codes))   # order-preserving unique
        emb_cache = {}
        with torch.no_grad():
            for s in range(0, len(uniq), CODEBERT_BATCH):
                batch = uniq[s:s + CODEBERT_BATCH]
                enc = tokenizer(batch, return_tensors="pt", truncation=True,
                                max_length=CODEBERT_MAX_LEN, padding=True)
                enc = {k: v.to(device) for k, v in enc.items()}
                out = model(**enc)
                hidden = out.last_hidden_state            # (B, L, 768)
                mask = enc["attention_mask"].unsqueeze(-1).float()
                summed = (hidden * mask).sum(dim=1)
                counts = mask.sum(dim=1).clamp(min=1.0)
                meanpool = (summed / counts).float().cpu().numpy()  # (B, 768)
                for c, vec in zip(batch, meanpool):
                    emb_cache[c] = vec

        X = np.asarray([emb_cache[c] for c in codes], dtype=np.float32)
        y = np.asarray(y, dtype=int)

        from sklearn.linear_model import LogisticRegression
        per_maxf1, per_05 = [], []
        for fi, (tr, te) in enumerate(folds):
            try:
                clf = LogisticRegression(max_iter=1000,
                                         class_weight="balanced")
                clf.fit(X[tr], y[tr])
                tr_proba = clf.predict_proba(X[tr])[:, 1]
                te_proba = clf.predict_proba(X[te])[:, 1]
                thr, _ = best_f1_threshold(y[tr], tr_proba)
                per_maxf1.append(fold_metrics(y[te], te_proba, thr))
                per_05.append(fold_metrics(y[te], te_proba, 0.5))
                print("    fold %d: maxF1@%.3f F1=%.3f PR-AUC=%.3f | @0.5 F1=%.3f"
                      % (fi, thr, per_maxf1[-1]["f1"], per_maxf1[-1]["pr_auc"],
                         per_05[-1]["f1"]))
            except Exception as e:
                print("    fold %d FAILED: %r" % (fi, e))
                continue

        if not per_maxf1:
            return {"status": "error - all folds failed"}
        res = aggregate(per_maxf1, per_05)
        res["status"] = "ok"
        res["note"] = "frozen CodeBERT mean-pool + LogReg probe (NOT fine-tuned)"
        return res
    except Exception as e:
        return {"status": "error - %r" % e,
                "trace": traceback.format_exc()[:600]}


# --------------------------------------------------------------------------- #
# Main
# --------------------------------------------------------------------------- #
def main():
    print("#" * 78)
    print("# baselines_sota.py — SOTA-FAMILY re-impl baselines on the Big-Vul split")
    print("# seed=%d  data=%s" % (SEED, DATA_PATH))
    print("# NOTE: re-implementations on a COMMON split, NOT the authors' tools.")
    print("#" * 78)

    out = {
        "seed": SEED,
        "data_path": os.path.relpath(DATA_PATH, os.path.dirname(os.path.abspath(__file__))),
        "disclaimer": ("Controlled representation-family re-implementations on a "
                       "common Big-Vul split; NOT the original Devign/ReVeal/"
                       "LineVul systems. CodeBERT is frozen + linear probe, not "
                       "fine-tuned. Use for relative positioning only."),
        "reference_rows": [
            {"representation": r[0], "f1": r[1], "pr_auc": r[2], "family": r[3]}
            for r in REFERENCE_ROWS
        ],
        "baselines": {},
    }

    rows = load_rows()
    print("loaded %d rows" % len(rows))
    if not rows:
        out["status"] = "no_data"
        _write(out)
        print("No data; wrote", RESULTS_PATH)
        return

    try:
        codes, labels = build_detection_set(rows)
    except Exception as e:
        out["status"] = "split_error: %r" % e
        _write(out)
        print("split error; wrote", RESULTS_PATH)
        return

    y = np.asarray(labels, dtype=int)
    n_pos = int(y.sum())
    n_neg = int(len(y) - n_pos)
    print("detection set: %d total | %d positive | %d negative (ratio ~1:%d)"
          % (len(y), n_pos, n_neg, DET_NEG_RATIO))
    out["dataset_counts"] = {
        "n_total": int(len(y)), "n_positive": n_pos, "n_negative": n_neg,
        "neg_ratio": DET_NEG_RATIO, "cap": DET_CAP,
    }

    if len(y) < 50 or n_pos < N_FOLDS or n_neg < N_FOLDS:
        out["status"] = "insufficient_data"
        _write(out)
        print("insufficient data; wrote", RESULTS_PATH)
        return

    # Same StratifiedKFold as augmented_study.py.
    from sklearn.model_selection import StratifiedKFold
    skf = StratifiedKFold(n_splits=N_FOLDS, shuffle=True, random_state=SEED)
    folds = list(skf.split(np.zeros(len(y)), y))

    # ---- Run each baseline, fully isolated; record status on failure. ----
    baseline_runners = [
        ("TextCNN", "token-CNN (Russell/Draper) [re-impl]", run_textcnn),
        ("BiGRU", "RNN token (VulDeePecker/SySeVR) [re-impl]", run_bigru),
        ("CodeBERT-frozen", "pretrained transformer (LineVul) [re-impl, frozen probe]",
         run_codebert),
    ]
    for name, family, runner in baseline_runners:
        print("-" * 78)
        print("BASELINE: %s — %s" % (name, family))
        try:
            res = runner(codes, y, folds)
        except Exception as e:
            res = {"status": "error - %r" % e,
                   "trace": traceback.format_exc()[:600]}
            print("  %s FAILED: %r" % (name, e))
        res["family"] = family
        out["baselines"][name] = res
        # Print a one-line summary for this baseline.
        if res.get("status", "ok") in ("ok",) or "maxF1_thresh" in res:
            mf = res.get("maxF1_thresh", {})
            print("  -> maxF1: F1=%.3f PR-AUC=%.3f P=%.3f R=%.3f (folds=%d)"
                  % (mf.get("f1", 0.0), mf.get("pr_auc", 0.0),
                     mf.get("precision", 0.0), mf.get("recall", 0.0),
                     res.get("n_folds", 0)))
        else:
            print("  -> status:", res.get("status"))

    # ---- Write JSON BEFORE any optional work. ----
    _write(out)
    print("results written to:", RESULTS_PATH)

    # ---- Final comparison table (our methods + refs + ran baselines). ----
    _print_table(out)


def _collect_table_rows(out):
    """(representation, F1, PR-AUC, family) for everything we can show."""
    table = []
    # Reference + our rows (context).
    for r in out.get("reference_rows", []):
        table.append((r["representation"], r["f1"], r["pr_auc"], r["family"]))
    # Ran baselines (use the max-F1 threshold metrics for the headline F1).
    for name, res in out.get("baselines", {}).items():
        fam = res.get("family", name)
        if "maxF1_thresh" in res:
            mf = res["maxF1_thresh"]
            table.append((name, mf.get("f1", 0.0), mf.get("pr_auc", 0.0), fam))
        else:
            table.append((name + " [UNAVAILABLE: %s]" % res.get("status", "?"),
                          None, None, fam))
    return table


def _print_table(out):
    print("=" * 78)
    print("COMPARISON TABLE  (sorted by F1; ref/ours rows interleaved)")
    print("re-impl = our re-implementation on the common split, NOT the authors' tool")
    print("=" * 78)
    rows = _collect_table_rows(out)
    # Sort: available rows by F1 desc, unavailable rows last.
    avail = [r for r in rows if r[1] is not None]
    unavail = [r for r in rows if r[1] is None]
    avail.sort(key=lambda r: r[1], reverse=True)

    hdr = "%-34s %7s %8s   %s" % ("representation", "F1", "PR-AUC", "family")
    print(hdr)
    print("-" * 78)
    for name, f1, prauc, fam in avail:
        print("%-34s %7.3f %8.3f   %s" % (name[:34], f1, prauc, fam))
    for name, f1, prauc, fam in unavail:
        print("%-34s %7s %8s   %s" % (name[:34], "  n/a", "   n/a", fam))

    # ---- Honest one-line verdict on where srcML-aug rows rank. ----
    print("-" * 78)
    ranked_names = [r[0] for r in avail]

    def rank_of(substr):
        for i, nm in enumerate(ranked_names):
            if substr in nm:
                return i + 1
        return None

    aw = rank_of("srcML-aug-whole")
    asl = rank_of("srcML-aug-slice")
    total = len(avail)
    verdict = (
        "VERDICT: among the SOTA-family baselines run here, srcML-aug-whole "
        "ranks #%s and srcML-aug-slice #%s of %d available rows. " % (
            aw if aw else "?", asl if asl else "?", total)
    )
    # Compare ours vs the deep baselines that actually ran.
    deep = {nm: f1 for (nm, f1, _, _) in avail
            if nm in ("TextCNN", "BiGRU", "CodeBERT-frozen")}
    ours_whole = next((f1 for (nm, f1, _, _) in avail
                       if "srcML-aug-whole" in nm), None)
    if deep and ours_whole is not None:
        best_deep_nm = max(deep, key=deep.get)
        if ours_whole >= deep[best_deep_nm]:
            verdict += ("srcML-aug-whole (F1=%.3f) is competitive with / above the "
                        "best ran deep baseline %s (F1=%.3f) on this split — a "
                        "cheap structural rep holding its own vs heavier models."
                        % (ours_whole, best_deep_nm, deep[best_deep_nm]))
        else:
            verdict += ("the best ran deep baseline %s (F1=%.3f) leads "
                        "srcML-aug-whole (F1=%.3f); our contribution is "
                        "efficiency/interpretability, not topping F1 here."
                        % (best_deep_nm, deep[best_deep_nm], ours_whole))
    else:
        verdict += ("deep baselines were unavailable in this run; comparison is "
                    "vs the reference TF-IDF rows only.")
    print(verdict)
    print("(All non-reference rows are re-implementations on a common split, "
          "not the original published systems.)")
    out["verdict"] = verdict
    _write(out)   # persist verdict too


def _write(out):
    try:
        os.makedirs(RESULTS_DIR, exist_ok=True)
        with open(RESULTS_PATH, "w", encoding="utf-8") as fh:
            json.dump(out, fh, indent=2, default=_json_default)
    except Exception as e:
        sys.stderr.write("WARN: could not write results JSON: %r\n" % e)


def _json_default(o):
    if isinstance(o, (set, frozenset)):
        return sorted(o)
    if isinstance(o, (np.integer,)):
        return int(o)
    if isinstance(o, (np.floating,)):
        return float(o)
    return str(o)


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        sys.stderr.write("FATAL (caught): %r\n" % e)
        sys.stderr.write(traceback.format_exc())
        try:
            _write({"status": "fatal", "error": repr(e)})
        except Exception:
            pass
