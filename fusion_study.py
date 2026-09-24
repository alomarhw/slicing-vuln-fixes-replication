#!/usr/bin/env python3
"""fusion_study.py — does CodeBERT (x) srcML-augmented features beat CodeBERT alone?

Same Big-Vul detection split as augmented_study/baselines_sota (seed 1337, 1:4, cap 2500,
StratifiedKFold 5). CodeBERT-frozen alone = F1 0.488 / PR-AUC 0.502 reference.
Arms: CodeBERT-only, AugWhole-only, FUSION(CB+AugWhole), FUSION(CB+AugSlice).
"""
import os, sys, json
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
DATA_PATH = os.path.join(HERE, "data", "bigvul", "sample.jsonl")
RESULTS = os.path.join(HERE, "results", "fusion_results.json")
SEED = 1337
CODEBERT_MODEL = "microsoft/codebert-base"
CODEBERT_MAX_LEN = 512
CODEBERT_BATCH = 16

from augmented_study import srcml_features, build_detection_set  # single source of truth
from ast_slicer import semantic_slice_indices

import scipy.sparse as sp
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import StratifiedKFold
from sklearn.metrics import f1_score, precision_score, recall_score, average_precision_score


def best_f1_threshold(y_true, proba):
    best_t, best_f1 = 0.5, -1.0
    for t in np.unique(np.round(proba, 3)):
        f1 = f1_score(y_true, (proba >= t).astype(int), zero_division=0)
        if f1 > best_f1:
            best_f1, best_t = f1, float(t)
    return best_t


def codebert_embeddings(codes):
    import torch
    from transformers import AutoTokenizer, AutoModel
    dev = "mps" if getattr(torch.backends, "mps", None) and torch.backends.mps.is_available() else "cpu"
    tok = AutoTokenizer.from_pretrained(CODEBERT_MODEL)
    model = AutoModel.from_pretrained(CODEBERT_MODEL).eval().to(dev)
    print("[CodeBERT] device=%s" % dev)
    uniq = list(dict.fromkeys(codes))
    cache = {}
    with torch.no_grad():
        for s in range(0, len(uniq), CODEBERT_BATCH):
            batch = uniq[s:s + CODEBERT_BATCH]
            enc = tok(batch, return_tensors="pt", truncation=True, max_length=CODEBERT_MAX_LEN, padding=True)
            enc = {k: v.to(dev) for k, v in enc.items()}
            out = model(**enc)
            mask = enc["attention_mask"].unsqueeze(-1).float()
            summed = (out.last_hidden_state * mask).sum(dim=1)
            counts = mask.sum(dim=1).clamp(min=1.0)
            mp = (summed / counts).float().cpu().numpy()
            for c, v in zip(batch, mp):
                cache[c] = v
            if s % 320 == 0:
                print("  embedded %d/%d" % (s, len(uniq)))
    return np.asarray([cache[c] for c in codes], dtype=np.float32)


def aug_feature_strings(codes):
    """Return (aug_whole[str], aug_slice[str]) per code, computing srcml once per unique code."""
    cache = {}
    fail = 0
    for c in dict.fromkeys(codes):
        try:
            feats = srcml_features(c)            # list[(line, feat)] 1-based
            sidx = semantic_slice_indices(c)     # 0-based set; whole-func on failure
            whole = sorted({f for (_ln, f) in feats})
            sl = sorted({f for (ln, f) in feats if (ln - 1) in sidx})
            if not sl:
                sl = whole
            cache[c] = (" ".join(whole) or "EMPTYFEAT", " ".join(sl) or "EMPTYFEAT")
        except Exception:
            fail += 1
            cache[c] = ("EMPTYFEAT", "EMPTYFEAT")
    print("[aug] computed features for %d unique funcs (%d failures)" % (len(cache), fail))
    aw = [cache[c][0] for c in codes]
    asl = [cache[c][1] for c in codes]
    return aw, asl


def metrics(y_true, proba, thr):
    pred = (proba >= thr).astype(int)
    return (f1_score(y_true, pred, zero_division=0),
            average_precision_score(y_true, proba),
            precision_score(y_true, pred, zero_division=0),
            recall_score(y_true, pred, zero_division=0))


def main():
    rows = [json.loads(l) for l in open(DATA_PATH, encoding="utf-8", errors="replace")]
    codes, y = build_detection_set(rows)
    y = np.asarray(y, dtype=int)
    print("detection set: %d total | %d pos | %d neg" % (len(y), int(y.sum()), int((y == 0).sum())))

    emb = codebert_embeddings(codes)
    aw, asl = aug_feature_strings(codes)
    aw = np.asarray(aw, dtype=object); asl = np.asarray(asl, dtype=object)

    folds = list(StratifiedKFold(n_splits=5, shuffle=True, random_state=SEED).split(np.zeros(len(y)), y))

    def run_arm(make_train_test):
        f1s, prs, ps, rs = [], [], [], []
        for tr, te in folds:
            Xtr, Xte = make_train_test(tr, te)
            clf = LogisticRegression(max_iter=1000, class_weight="balanced")
            clf.fit(Xtr, y[tr])
            ptr = clf.predict_proba(Xtr)[:, 1]
            pte = clf.predict_proba(Xte)[:, 1]
            thr = best_f1_threshold(y[tr], ptr)
            f1, pr, p, r = metrics(y[te], pte, thr)
            f1s.append(f1); prs.append(pr); ps.append(p); rs.append(r)
        return {"f1": float(np.mean(f1s)), "pr_auc": float(np.mean(prs)),
                "precision": float(np.mean(ps)), "recall": float(np.mean(rs)),
                "f1_folds": [round(x, 3) for x in f1s], "f1_std": float(np.std(f1s))}

    def cb(tr, te):
        sc = StandardScaler().fit(emb[tr])
        return sc.transform(emb[tr]), sc.transform(emb[te])

    def augw(tr, te):
        v = TfidfVectorizer(token_pattern=r"[^ ]+", lowercase=False).fit(aw[tr].tolist())
        return v.transform(aw[tr].tolist()), v.transform(aw[te].tolist())

    def fuse(feat_arr):
        def _mk(tr, te):
            sc = StandardScaler().fit(emb[tr])
            etr = sp.csr_matrix(sc.transform(emb[tr])); ete = sp.csr_matrix(sc.transform(emb[te]))
            v = TfidfVectorizer(token_pattern=r"[^ ]+", lowercase=False).fit(feat_arr[tr].tolist())
            ftr = v.transform(feat_arr[tr].tolist()); fte = v.transform(feat_arr[te].tolist())
            return sp.hstack([etr, ftr]).tocsr(), sp.hstack([ete, fte]).tocsr()
        return _mk

    arms = {
        "CodeBERT-only": run_arm(cb),
        "AugWhole-only": run_arm(augw),
        "FUSION(CodeBERT+AugWhole)": run_arm(fuse(aw)),
        "FUSION(CodeBERT+AugSlice)": run_arm(fuse(asl)),
    }
    os.makedirs(os.path.dirname(RESULTS), exist_ok=True)
    json.dump({"arms": arms, "n": int(len(y)), "pos": int(y.sum())}, open(RESULTS, "w"), indent=2)

    print("\n" + "=" * 70)
    print("FUSION RESULTS (5-fold; reference: CodeBERT-frozen 0.488/0.502)")
    print("=" * 70)
    print("%-30s %6s %7s %8s" % ("arm", "F1", "PR-AUC", "F1_std"))
    for k in sorted(arms, key=lambda a: -arms[a]["f1"]):
        a = arms[k]
        print("%-30s %.3f  %.3f   %.3f   folds=%s" % (k, a["f1"], a["pr_auc"], a["f1_std"], a["f1_folds"]))
    cbf1 = arms["CodeBERT-only"]["f1"]
    best_fuse = max(arms["FUSION(CodeBERT+AugWhole)"]["f1"], arms["FUSION(CodeBERT+AugSlice)"]["f1"])
    print("-" * 70)
    print("HEADLINE: best fusion F1=%.3f vs CodeBERT-only F1=%.3f (delta %+.3f); vs 0.488 ref (delta %+.3f)"
          % (best_fuse, cbf1, best_fuse - cbf1, best_fuse - 0.488))
    print("DONE")


if __name__ == "__main__":
    main()
