#!/usr/bin/env python3
"""
finetune_codebert.py
---------------------
Fine-tunes microsoft/codebert-base (full model, not a frozen linear probe) for
vulnerability detection, on the exact same data / 5-fold split as the frozen
CodeBERT baseline in baselines_sota.py, so the two are directly comparable.
Added in response to a Reviewer critique: the paper's only pretrained-encoder
comparison was a frozen linear probe; this adds the fine-tuned upper bound
(or shows there isn't much of a gap, either way an honest data point).

Run in the isolated .venv_codebert (numpy<2 / torch==2.2.2 / transformers==4.46.2
-- the main venv's numpy>=2 breaks torch's ABI). See ../.venv_codebert.

Writes: results/finetuned_codebert_results.json (checkpointed after every fold),
        with per-fold f1/pr_auc arrays (not just the mean) so real statistics
        (Wilcoxon, CIs) can be computed downstream.
"""

import json
import os
import subprocess
import sys
import time
import traceback

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from baselines_sota import (  # noqa: E402
    load_rows, build_detection_set, best_f1_threshold, fold_metrics,
    get_torch_device, SEED, N_FOLDS, CODEBERT_MODEL, CODEBERT_MAX_LEN,
    DATA_PATH,
)

RESULTS_PATH = os.path.join("results", "finetuned_codebert_results.json")
EPOCHS = 1
BATCH_SIZE = 4
LR = 2e-5
MAX_LEN = 256  # CODEBERT_MAX_LEN (512) triggers MPS OOM (O(n^2) attention memory)


def _write(obj):
    os.makedirs("results", exist_ok=True)
    with open(RESULTS_PATH, "w") as f:
        json.dump(obj, f, indent=2)


def run_fold(fold_idx, codes, y, tr_idx, te_idx, device):
    import torch
    import torch.nn as nn
    from torch.utils.data import Dataset, DataLoader
    from transformers import AutoTokenizer, AutoModelForSequenceClassification

    torch.manual_seed(SEED + fold_idx)

    tokenizer = AutoTokenizer.from_pretrained(CODEBERT_MODEL)
    model = AutoModelForSequenceClassification.from_pretrained(CODEBERT_MODEL, num_labels=2)
    model.to(device)

    class CodeDS(Dataset):
        def __init__(self, idxs):
            self.idxs = idxs

        def __len__(self):
            return len(self.idxs)

        def __getitem__(self, i):
            j = self.idxs[i]
            return codes[j], int(y[j])

    def collate(batch):
        texts, labels = zip(*batch)
        enc = tokenizer(list(texts), return_tensors="pt", truncation=True,
                         max_length=MAX_LEN, padding=True)
        enc["labels"] = torch.tensor(labels, dtype=torch.long)
        return enc

    tr_loader = DataLoader(CodeDS(list(tr_idx)), batch_size=BATCH_SIZE, shuffle=True,
                            collate_fn=collate)
    te_loader = DataLoader(CodeDS(list(te_idx)), batch_size=BATCH_SIZE * 2, shuffle=False,
                            collate_fn=collate)

    # Class-weighted loss for the 1:4 imbalance (mirrors class_weight="balanced").
    y_tr = y[tr_idx]
    n_pos = max(1, int((y_tr == 1).sum()))
    n_neg = max(1, int((y_tr == 0).sum()))
    w_pos = len(y_tr) / (2.0 * n_pos)
    w_neg = len(y_tr) / (2.0 * n_neg)
    class_weights = torch.tensor([w_neg, w_pos], dtype=torch.float32, device=device)
    loss_fn = nn.CrossEntropyLoss(weight=class_weights)

    optimizer = torch.optim.AdamW(model.parameters(), lr=LR)

    model.train()
    t0 = time.time()
    for epoch in range(EPOCHS):
        total_loss, n_batches = 0.0, 0
        for step, batch in enumerate(tr_loader):
            batch = {k: v.to(device) for k, v in batch.items()}
            labels = batch.pop("labels")
            optimizer.zero_grad()
            out = model(**batch)
            loss = loss_fn(out.logits, labels)
            loss.backward()
            optimizer.step()
            total_loss += loss.item()
            n_batches += 1
            del out, loss, batch, labels
            if step % 20 == 0:
                try:
                    if device.type == "mps":
                        torch.mps.empty_cache()
                except Exception:
                    pass
            if step % 25 == 0:
                elapsed = time.time() - t0
                print(f"  [fold {fold_idx}] epoch {epoch+1}/{EPOCHS} step {step}/{len(tr_loader)} "
                      f"loss={total_loss / max(1, n_batches):.4f} elapsed={elapsed:.0f}s", flush=True)
        print(f"  [fold {fold_idx}] epoch {epoch+1}/{EPOCHS} done, mean_loss="
              f"{total_loss / max(1, n_batches):.4f}", flush=True)

    # ---- Inference: get train-fold probs (for threshold selection) + test-fold probs.
    model.eval()

    def predict_proba(idxs):
        loader = DataLoader(CodeDS(list(idxs)), batch_size=BATCH_SIZE * 2, shuffle=False,
                             collate_fn=collate)
        probs = []
        with torch.no_grad():
            for bi, batch in enumerate(loader):
                batch = {k: v.to(device) for k, v in batch.items()}
                batch.pop("labels", None)
                out = model(**batch)
                p = torch.softmax(out.logits, dim=-1)[:, 1].cpu().numpy()
                probs.append(p)
                del out, batch
                if bi % 20 == 0:
                    try:
                        if device.type == "mps":
                            torch.mps.empty_cache()
                    except Exception:
                        pass
        return np.concatenate(probs) if probs else np.array([])

    tr_proba = predict_proba(tr_idx)
    te_proba = predict_proba(te_idx)
    thr, _ = best_f1_threshold(y[tr_idx], tr_proba)
    m_maxf1 = fold_metrics(y[te_idx], te_proba, thr)
    m_at05 = fold_metrics(y[te_idx], te_proba, 0.5)
    runtime = time.time() - t0

    # Free MPS/CPU memory before the next fold.
    del model, tokenizer
    import gc
    gc.collect()
    try:
        if device.type == "mps":
            torch.mps.empty_cache()
    except Exception:
        pass

    return {"maxF1_thresh": m_maxf1, "at_0.5": m_at05, "threshold": thr,
            "runtime_sec": runtime, "n_train": len(tr_idx), "n_test": len(te_idx)}


def _get_folds():
    rows = load_rows()
    print("loaded", len(rows), "rows")
    codes, labels = build_detection_set(rows)
    y = np.asarray(labels, dtype=int)
    print(f"detection set: {len(y)} total | {int(y.sum())} positive | {len(y) - int(y.sum())} negative")
    from sklearn.model_selection import StratifiedKFold
    skf = StratifiedKFold(n_splits=N_FOLDS, shuffle=True, random_state=SEED)
    folds = list(skf.split(np.zeros(len(y)), y))
    return codes, y, folds


def run_single_fold_and_exit(fold_idx: int, out_json: str):
    """Run exactly one fold in this process and write its result to out_json,
    then exit. Called as a fresh subprocess per fold (see main()) because the
    PyTorch MPS allocator on this hardware does not fully release memory
    between folds within one process -- confirmed empirically: fold 0
    succeeds, every subsequent fold in the same process OOMs at an identical
    ~10.5GB "other allocations" ceiling regardless of in-process cleanup
    (del + gc.collect() + torch.mps.empty_cache()). A fresh OS process per
    fold is the reliable fix."""
    codes, y, folds = _get_folds()
    tr, te = folds[fold_idx]
    device = get_torch_device()
    print("device:", device, flush=True)
    res = run_fold(fold_idx, codes, y, tr, te, device)
    with open(out_json, "w") as f:
        json.dump(res, f)
    print(f"  -> fold {fold_idx} F1={res['maxF1_thresh']['f1']:.3f} "
          f"PR-AUC={res['maxF1_thresh']['pr_auc']:.3f} runtime={res['runtime_sec']:.0f}s",
          flush=True)


def main():
    if "--fold" in sys.argv:
        fi = int(sys.argv[sys.argv.index("--fold") + 1])
        out_json = sys.argv[sys.argv.index("--out-json") + 1]
        run_single_fold_and_exit(fi, out_json)
        return

    print("#" * 78)
    print(f"# finetune_codebert.py — seed={SEED} data={DATA_PATH} epochs={EPOCHS}")
    print("# Each fold runs as a separate subprocess (see run_single_fold_and_exit "
          "docstring for why).")
    print("#" * 78, flush=True)

    device_label = str(get_torch_device())
    out = {
        "seed": SEED, "data_path": os.path.relpath(DATA_PATH, os.path.dirname(os.path.abspath(__file__))), "model": CODEBERT_MODEL,
        "epochs": EPOCHS, "batch_size": BATCH_SIZE, "lr": LR, "max_len": MAX_LEN,
        "note": "Full fine-tune (all weights updated), NOT a frozen linear probe. "
                "Same StratifiedKFold(5, seed=1337) split as the frozen CodeBERT "
                "baseline in baselines_sota.py, for a paired, fair comparison. Each "
                "fold ran as an isolated subprocess (MPS allocator memory is not "
                "fully released across folds within one process on this hardware).",
        "device": device_label,
        "per_fold_maxF1": [], "per_fold_at05": [], "status": "running",
    }
    _write(out)

    for fi in range(N_FOLDS):
        print("-" * 78, flush=True)
        print(f"FOLD {fi+1}/{N_FOLDS} (subprocess)", flush=True)
        fold_json = os.path.join("results", f"_finetune_fold_{fi}.json")
        if os.path.exists(fold_json):
            os.remove(fold_json)
        proc = subprocess.run(
            [sys.executable, os.path.abspath(__file__), "--fold", str(fi),
             "--out-json", fold_json],
        )
        if proc.returncode != 0 or not os.path.exists(fold_json):
            print(f"  fold {fi} FAILED: subprocess exit code {proc.returncode}", flush=True)
            out.setdefault("fold_errors", []).append(
                {"fold": fi, "error": f"subprocess exit code {proc.returncode}"})
            _write(out)
            continue
        with open(fold_json) as f:
            res = json.load(f)
        os.remove(fold_json)
        out["per_fold_maxF1"].append(res["maxF1_thresh"])
        out["per_fold_at05"].append(res["at_0.5"])
        _write(out)  # checkpoint after every fold

    def _mean(key_outer, key_inner):
        vals = [r[key_inner] for r in out[key_outer]]
        return float(np.mean(vals)) if vals else 0.0

    def _std(key_outer, key_inner):
        vals = [r[key_inner] for r in out[key_outer]]
        return float(np.std(vals)) if vals else 0.0

    out["maxF1_thresh"] = {
        k: _mean("per_fold_maxF1", k) for k in ("f1", "precision", "recall", "pr_auc")
    }
    out["maxF1_thresh_std"] = {
        k: _std("per_fold_maxF1", k) for k in ("f1", "precision", "recall", "pr_auc")
    }
    out["at_0.5"] = {k: _mean("per_fold_at05", k) for k in ("f1", "precision", "recall", "pr_auc")}
    out["n_folds"] = len(out["per_fold_maxF1"])
    out["status"] = "ok" if out["per_fold_maxF1"] else "all_folds_failed"
    _write(out)
    print("=" * 78)
    print(f"DONE. F1={out['maxF1_thresh']['f1']:.3f} (+/-{out['maxF1_thresh_std']['f1']:.3f}) "
          f"PR-AUC={out['maxF1_thresh']['pr_auc']:.3f}")
    print("results written to", RESULTS_PATH)


if __name__ == "__main__":
    main()
