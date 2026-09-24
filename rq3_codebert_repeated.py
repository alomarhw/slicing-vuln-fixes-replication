"""
rq3_codebert_repeated.py
------------------------
SANER revision: put the frozen-CodeBERT probe under the SAME protocol as the
TF-IDF representations in RQ3 (same 2,500 functions, same 30 x 5 repeated
stratified splits, same balanced logistic regression with a 0.5 threshold), so
its comparison with the srcML representations is paired and can be tested with
the Nadeau-Bengio corrected t-test.

  step 1 (codebert venv):  python rq3_codebert_repeated.py embed
      -> results/rq3_codebert_embeddings.npy  (mean-pooled microsoft/codebert-base,
         max_len 512, as in fusion_study.py)
  step 2 (main venv):      python rq3_codebert_repeated.py probe
      -> results/rq3_codebert_repeated.json
"""
import os
import sys
import json

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

EMB = os.path.join(HERE, "results", "rq3_codebert_embeddings.npy")
OUT = os.path.join(HERE, "results", "rq3_codebert_repeated.json")


def embed():
    import augmented_study as A
    from fusion_study import codebert_embeddings
    codes, labels = A.build_detection_set(A.load_rows())
    X = codebert_embeddings(codes)
    np.save(EMB, X)
    np.save(EMB.replace(".npy", "_labels.npy"), np.asarray(labels, dtype=int))
    print("saved", X.shape)


def probe():
    import augmented_study as A
    from sklearn.linear_model import LogisticRegression
    from sklearn.model_selection import RepeatedStratifiedKFold
    from sklearn.metrics import f1_score, average_precision_score, precision_score, recall_score
    X = np.load(EMB)
    y = np.load(EMB.replace(".npy", "_labels.npy"))
    _, labels = A.build_detection_set(A.load_rows())
    assert list(y) == list(labels), "embeddings do not match the RQ3 detection set"
    folds = list(RepeatedStratifiedKFold(n_splits=A.N_FOLDS, n_repeats=A.N_REPEATS,
                                         random_state=A.SEED).split(np.zeros(len(y)), y))
    f1s, aps, precs, recs = [], [], [], []
    for tr, te in folds:
        clf = LogisticRegression(max_iter=1000, class_weight="balanced").fit(X[tr], y[tr])
        pred = clf.predict(X[te])
        f1s.append(f1_score(y[te], pred, zero_division=0))
        precs.append(precision_score(y[te], pred, zero_division=0))
        recs.append(recall_score(y[te], pred, zero_division=0))
        aps.append(average_precision_score(y[te], clf.predict_proba(X[te])[:, 1]))
    reps = json.load(open(os.path.join(HERE, "results", "augmented_results.json")))["rq3_aug"]["representations"]
    n_tr, n_te = len(folds[0][0]), len(folds[0][1])
    tests = {}
    for name in ("AUG_WHOLE", "AUG_SLICE", "WHOLE_TEXT"):
        other = np.asarray(reps[name]["per_fold_f1"])
        assert len(other) == len(f1s)
        d = np.asarray(f1s) - other
        # Nadeau-Bengio corrected resampled t-test (same formula as augmented_study)
        from scipy import stats
        var = d.var(ddof=1)
        t = d.mean() / np.sqrt((1.0 / len(d) + n_te / n_tr) * var) if var > 0 else float("inf")
        p = float(2 * stats.t.sf(abs(t), df=len(d) - 1))
        tests[f"codebert_frozen_vs_{name}"] = {"mean_diff": float(d.mean()), "nb_t": float(t), "nb_p": p}
    out = {"protocol": "30x5 repeated stratified CV, seed %d, balanced LR, 0.5 threshold" % A.SEED,
           "n_splits": len(folds), "f1": float(np.mean(f1s)), "f1_std": float(np.std(f1s)),
           "pr_auc": float(np.mean(aps)), "precision": float(np.mean(precs)),
           "recall": float(np.mean(recs)), "per_fold_f1": f1s, "tests": tests}
    json.dump(out, open(OUT, "w"), indent=2)
    print(json.dumps({k: v for k, v in out.items() if k != "per_fold_f1"}, indent=2))


if __name__ == "__main__":
    {"embed": embed, "probe": probe}[sys.argv[1]]()
