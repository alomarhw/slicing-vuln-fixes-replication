"""
rq3_normalized_tokens.py
------------------------
SANER revision: separate "abstract structural features" from "smaller
vocabulary". Adds SySeVR-style normalized-token conditions to RQ3: in the
slice (and whole-function) text, user-defined variable names become VAR1,
VAR2, ... and user-defined function names FUN1, FUN2, ... in order of first
appearance, while C keywords and library/API calls are kept, as in SySeVR's
code-gadget normalization. Same 2,500 functions, same 30 x 5 repeated splits,
same TF-IDF (word 1-2 grams, 2,000 features) + balanced logistic regression as
the main RQ3 run; compared with the main-run representations on the identical
splits with the Nadeau-Bengio corrected t-test.

Writes results/rq3_normalized_tokens.json.
"""
import os
import re
import sys
import json

import numpy as np
from scipy import stats
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import RepeatedStratifiedKFold
from sklearn.metrics import f1_score, average_precision_score

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.chdir(HERE)

import augmented_study as A  # noqa: E402
from ast_slicer import _SENSITIVE_CALLS  # noqa: E402
from proto_dualsig import C_KEYWORDS  # noqa: E402

OUT = os.path.join(HERE, "results", "rq3_normalized_tokens.json")
IDENT = re.compile(r"[A-Za-z_]\w*")
LIBRARY = set(_SENSITIVE_CALLS) | {
    "printf", "fprintf", "puts", "putchar", "getchar", "fopen", "fclose", "fwrite", "fseek",
    "ftell", "open", "close", "write", "lseek", "strcmp", "strncmp", "strchr", "strrchr",
    "strstr", "strtol", "strtoul", "atoi", "atol", "abs", "exit", "abort", "assert",
    "memchr", "isdigit", "isalpha", "isspace", "toupper", "tolower", "min", "max", "sizeof",
}


def normalize(text):
    var_map, fun_map, out, pos = {}, {}, [], 0
    for m in IDENT.finditer(text):
        name = m.group(0)
        out.append(text[pos:m.start()])
        pos = m.end()
        if name in C_KEYWORDS or name in LIBRARY or name.isupper():   # keep keywords, API, macros
            out.append(name)
            continue
        is_call = text[m.end():m.end() + 1] == "(" or text[m.end():].lstrip().startswith("(")
        table, prefix = (fun_map, "FUN") if is_call else (var_map, "VAR")
        if name not in table:
            table[name] = f"{prefix}{len(table) + 1}"
        out.append(table[name])
    out.append(text[pos:])
    return "".join(out)


def nb_test(a, b, n_te, n_tr):
    d = np.asarray(a) - np.asarray(b)
    var = d.var(ddof=1)
    t = d.mean() / np.sqrt((1.0 / len(d) + n_te / n_tr) * var) if var > 0 else float("inf")
    return {"mean_diff": float(d.mean()), "nb_p": float(2 * stats.t.sf(abs(t), df=len(d) - 1))}


def main():
    codes, labels = A.build_detection_set(A.load_rows())
    y = np.asarray(labels)
    corpora = {"SLICE_TEXT_NORM": [normalize(A.slice_text(c) or " ") for c in codes],
               "WHOLE_TEXT_NORM": [normalize(c) for c in codes]}
    folds = list(RepeatedStratifiedKFold(n_splits=A.N_FOLDS, n_repeats=A.N_REPEATS,
                                         random_state=A.SEED).split(np.zeros(len(y)), y))
    main_reps = json.load(open(os.path.join(HERE, "results", "augmented_results.json")))["rq3_aug"]["representations"]
    n_tr, n_te = len(folds[0][0]), len(folds[0][1])
    out = {"protocol": "same as augmented_study RQ3 (30x5 repeated stratified CV, TF-IDF 1-2 grams, "
                       "2000 features, balanced LR, 0.5 threshold)", "representations": {}, "tests": {}}
    per, per_ap = {}, {}
    for name, corpus in corpora.items():
        f1s, aps = [], []
        for tr, te in folds:
            vec = TfidfVectorizer(max_features=2000, ngram_range=(1, 2))
            Xtr = vec.fit_transform([corpus[i] for i in tr])
            Xte = vec.transform([corpus[i] for i in te])
            clf = LogisticRegression(max_iter=500, class_weight="balanced").fit(Xtr, y[tr])
            f1s.append(f1_score(y[te], clf.predict(Xte), zero_division=0))
            aps.append(average_precision_score(y[te], clf.predict_proba(Xte)[:, 1]))
        per[name] = f1s
        per_ap[name] = aps
        out["representations"][name] = {"f1": float(np.mean(f1s)), "f1_std": float(np.std(f1s)),
                                        "pr_auc": float(np.mean(aps))}
    for a, b in (("AUG_SLICE", "SLICE_TEXT_NORM"), ("SLICE_TEXT_NORM", "SLICE_TEXT"),
                 ("AUG_WHOLE", "WHOLE_TEXT_NORM"), ("WHOLE_TEXT_NORM", "WHOLE_TEXT")):
        fa = per.get(a, main_reps.get(a, {}).get("per_fold_f1"))
        fb = per.get(b, main_reps.get(b, {}).get("per_fold_f1"))
        out["tests"][f"{a}_vs_{b}"] = nb_test(fa, fb, n_te, n_tr)
        pa = per_ap.get(a, main_reps.get(a, {}).get("per_fold_pr_auc"))
        pb = per_ap.get(b, main_reps.get(b, {}).get("per_fold_pr_auc"))
        out["tests"][f"{a}_vs_{b}"]["pr_auc"] = nb_test(pa, pb, n_te, n_tr)
    ps = sorted((v["nb_p"], k) for k, v in out["tests"].items())
    m = len(ps)
    for rank, (p, k) in enumerate(ps):
        out["tests"][k]["nb_p_holm"] = float(min(1.0, max(p * (m - i) for i, (p, _) in enumerate(ps[:rank + 1]))))
    out["example"] = {"raw": A.slice_text(codes[0])[:300], "normalized": corpora["SLICE_TEXT_NORM"][0][:300]}
    json.dump(out, open(OUT, "w"), indent=2)
    print(json.dumps({k: v for k, v in out.items() if k != "example"}, indent=2))


if __name__ == "__main__":
    main()
