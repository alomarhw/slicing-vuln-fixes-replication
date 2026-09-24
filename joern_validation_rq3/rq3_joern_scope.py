"""
joern_validation_rq3/rq3_joern_scope.py
---------------------------------------
SANER revision: repeat the RQ3 scope comparison with Joern's reference slices.

For the RQ3 detection functions that Joern's C frontend parses (backward_slice.sc
over src/d*.c), phi is computed over three scopes on the SAME functions:
Joern's backward slice (mapped to statement units as in joern_validation/compare.py),
our tree-sitter slice, and the variable-mention region. The three representations
are compared with the main RQ3 classifier (TF-IDF over feature tokens + balanced
logistic regression) under 30 x 5 repeated stratified CV on this subset, with the
Nadeau-Bengio corrected t-test and Holm correction.

Writes ../results/rq3_joern_scope.json.
"""
import os
import sys
import json
import collections

import numpy as np
from scipy import stats
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import RepeatedStratifiedKFold
from sklearn.metrics import f1_score, average_precision_score

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, ROOT)
os.chdir(ROOT)

import augmented_study as A  # noqa: E402
from joern_validation.compare import statements, to_units, span  # noqa: E402

OUT = os.path.join(ROOT, "results", "rq3_joern_scope.json")


def main():
    codes, labels = A.build_detection_set(A.load_rows())
    by_file = collections.defaultdict(list)
    for line in open(os.path.join(HERE, "joern_slices.jsonl")):
        r = json.loads(line)
        by_file[os.path.basename(r["file"])].append(r)

    idx, reps = [], {"PHI_JOERN_SLICE": [], "PHI_OUR_SLICE": [], "PHI_VARMENTION": []}
    for fname, methods in sorted(by_file.items()):
        i = int(fname[1:5])
        code = codes[i]
        n = len(code.splitlines())
        m = max(methods, key=lambda r: len(r["lines"]))
        J0 = {ln - 1 for ln in m["lines"] if 1 <= ln <= n}
        units = statements(code)
        if not J0 or not units:
            continue
        J = span(to_units(J0, units), units, n)
        if not J:
            continue
        pairs = A.srcml_pairs(code)
        if not pairs:
            continue
        feats = lambda region: " ".join(sorted({f for ln, f in pairs if ln >= 1 and (ln - 1) in region})) or "EMPTYFEAT"  # noqa: E731
        reps["PHI_JOERN_SLICE"].append(feats(J))
        reps["PHI_OUR_SLICE"].append(feats(A.slice_idx(code)))
        reps["PHI_VARMENTION"].append(feats(A.varmention_idx(code)))
        idx.append(i)

    y = np.asarray([labels[i] for i in idx])
    folds = list(RepeatedStratifiedKFold(n_splits=A.N_FOLDS, n_repeats=A.N_REPEATS,
                                         random_state=A.SEED).split(np.zeros(len(y)), y))
    n_tr, n_te = len(folds[0][0]), len(folds[0][1])
    per, out = {}, {"n_functions": len(idx), "n_positive": int(y.sum()), "representations": {}, "tests": {}}
    for name, corpus in reps.items():
        f1s, aps = [], []
        for tr, te in folds:
            vec = A.TfidfVectorizer(max_features=2000, ngram_range=(1, 1), token_pattern=r"[^ ]+", lowercase=False)
            Xtr = vec.fit_transform([corpus[k] for k in tr])
            Xte = vec.transform([corpus[k] for k in te])
            clf = LogisticRegression(max_iter=500, class_weight="balanced").fit(Xtr, y[tr])
            f1s.append(f1_score(y[te], clf.predict(Xte), zero_division=0))
            aps.append(average_precision_score(y[te], clf.predict_proba(Xte)[:, 1]))
        per[name] = f1s
        out["representations"][name] = {"f1": float(np.mean(f1s)), "f1_std": float(np.std(f1s)),
                                        "pr_auc": float(np.mean(aps))}
    comps = [("PHI_JOERN_SLICE", "PHI_VARMENTION"), ("PHI_JOERN_SLICE", "PHI_OUR_SLICE"),
             ("PHI_OUR_SLICE", "PHI_VARMENTION")]
    raw = []
    for a, b in comps:
        d = np.asarray(per[a]) - np.asarray(per[b])
        var = d.var(ddof=1)
        t = d.mean() / np.sqrt((1.0 / len(d) + n_te / n_tr) * var) if var > 0 else float("inf")
        p = float(2 * stats.t.sf(abs(t), df=len(d) - 1))
        out["tests"][f"{a}_vs_{b}"] = {"mean_diff": float(d.mean()), "nb_p": p}
        raw.append((p, f"{a}_vs_{b}"))
    raw.sort()
    for rank, (p, k) in enumerate(raw):
        out["tests"][k]["nb_p_holm"] = float(min(1.0, max(q * (len(raw) - j) for j, (q, _) in enumerate(raw[:rank + 1]))))
    json.dump(out, open(OUT, "w"), indent=2)
    print(json.dumps(out, indent=2))


if __name__ == "__main__":
    main()
