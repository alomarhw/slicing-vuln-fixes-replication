# Data Dictionary

Describes every field used by the current pipeline: the raw BigVul input
(`data/bigvul/sample.jsonl`) and the derived fields written to `results/*.json`
by `consolidated_study.py` (RQ1), `augmented_study.py` (RQ2, RQ3 core),
`baselines_sota.py` / `graph_model.py` / `fusion_study.py` / `finetune_codebert.py`
(RQ3 baselines), and `analyze_sink_fallback.py` (RQ1 diagnostic). This
supersedes an earlier version of this file written for an abandoned pipeline
(SG-VDR / Defects4J / Juliet) that is no longer part of this study.

## Raw input: `data/bigvul/sample.jsonl` (one JSON object per line)

| Field | Type | Description |
|---|---|---|
| `func_before` | string | Full source of the function as it existed before the fixing commit. The only field the slicer/detector ever sees at slice/prediction time. |
| `func_after` | string | Full source of the function after the fixing commit. Consulted **only** for measurement (RQ1 coverage/RSR ground truth); never available to the slicer, the detector, or the signature discriminator. |
| `vul` | integer (binary) | 1 if `func_before` is the vulnerable version of a real CVE-fixing pair; 0 if the function is an unrelated, non-vulnerable BigVul row (used as RQ3 detection negatives). |
| `CWE ID` | string | CWE category of the vulnerability (e.g. `CWE-119`), where known. Used for the RQ2 by-CWE stratified breakdown; unlabeled rows are grouped as `unlabeled`. |
| `project` | string | Open-source project the function was taken from (e.g. `Chrome`, `Linux`). |
| `commit_id` | string | Git commit hash of the fixing commit in the source project. |
| `commit_message` | string | Full commit message of the fixing commit. Not used by any experiment; kept for traceability. |
| `codeLink` / `CVE Page` | string (URL) | Links to the source commit and CVE record. Not used by any experiment; kept for traceability. |
| `CVE ID` | string | CVE identifier, where known. |
| `lang` | string | Source language (`C` for this study; C++ rows are also present but not distinguished). |

## RQ1 — `results/study_results.json` (`RQ1` object)

| Field | Type | Description |
|---|---|---|
| `pairs_used` | integer | Number of CVE pairs (294) with a non-empty diff and a non-empty slice; the denominator for every RQ1 statistic. |
| `slice.mean_coverage`, `slice.median_coverage` | float in [0,1] | Fraction of the real diff's deleted lines that fall inside the backward slice, per pair, then averaged/medianed. |
| `slice.mean_region_size_ratio`, `slice.median_region_size_ratio` | float in [0,1] | Slice line count / function line count, per pair, then averaged/medianed (RSR). |
| `slice.coverage_ci95`, `slice.region_size_ratio_ci95` | [float, float] | 95% bootstrap CI (2,000 resamples, seed 1337) on the mean coverage / RSR. |
| `by_fix_size` | object keyed `"1 line"` / `"2-5 lines"` / `">5 lines"` | Per-stratum `n`, `mean_coverage`, `mean_region_size_ratio`, `coverage_ci95`, grouped by number of deleted lines in the real fix. |
| `by_function_length_quartile` | object | `quartile_boundaries_lines` (Q1/Q2/Q3 cutoffs in `func_before` line count) and per-quartile `strata` with the same fields as `by_fix_size`. |

## RQ1 diagnostic — `results/sink_fallback_analysis.json`

| Field | Type | Description |
|---|---|---|
| `n_sink_criterion_matched` / `n_sink_criterion_fallback` | integer | Pairs where a real sink pattern matched vs. where the conservative last-statement fallback fired. |
| `sink_criterion_fallback_rate_pct` | float | `100 * fallback / (matched + fallback)` — the paper's headline 1.7%. |
| `matched` / `fallback` | object | `{mean_coverage, mean_region_size_ratio}` computed separately for each group, showing the fallback's weaker localization. |

## RQ2 — `results/augmented_results.json` (`rq2_aug` object)

| Field | Type | Description |
|---|---|---|
| `test_a.coarse` / `test_a.aug` | object | `n`, `nonempty` (CVEs whose fix produces a non-empty feature delta), `nonempty_pct` — the headline discrimination-rate row (25.6% coarse → 52.8% augmented). |
| `test_a_by_cwe.by_cwe` | list of objects | Per-CWE `{cwe, n_coarse, n_aug, coarse_nonempty_pct, aug_nonempty_pct}`, restricted to CWEs with ≥5 usable pairs in both arms. |
| `test_b.coarse` / `test_b.aug` | object | `ruleA_best` / `ruleB_best` (swept-tau-optimal P/R/F1/`func_after_fp`) and `ruleB_fixed_tau_0.5` (same metrics at a single pre-specified τ=0.5) for the held-out 70/30 generalization test. |
| `statistics.test_a_mcnemar` | object | `p_value`, `n_discordant`, `b01`/`b10` — exact McNemar's test on paired non-empty-delta outcomes (coarse vs. augmented, same CVE subset). |
| `statistics.test_b_coarse_bootstrap_ci` / `test_b_aug_bootstrap_ci` | object | `ci95`, `mean`, `n` — 1,000-resample bootstrap 95% CI on dual-rule F1, per arm. |
| `robustness.fixed_tau_0.5` | object | Dual-rule F1 for both arms at τ=0.5 (not swept), to bound test-set-selection optimism. |
| `robustness.intersection_restricted` | object | Augmented arm's dual-rule F1 re-computed on exactly the 316-pair subset the coarse arm can compute, with its 95% CI — a matched, apples-to-apples robustness check. |

## RQ3 core — `results/augmented_results.json` (`rq3_aug` object)

| Field | Type | Description |
|---|---|---|
| `representations.{WHOLE_TEXT,SLICE_TEXT,AUG_WHOLE,AUG_SLICE}` | object | `f1`, `pr_auc`, `f1_std`, `pr_auc_std`, `per_fold_f1`, `per_fold_pr_auc` — 5-fold stratified CV metrics for each of the four lightweight representations. |
| `statistics.{pair}` | object | `diff` (mean per-fold difference), `p` (paired Wilcoxon signed-rank p-value), `cliffs_delta` — for the 4 pairwise comparisons among the lightweight representations. |

## RQ3 baselines — other `results/*.json`

| File | Key fields |
|---|---|
| `baselines_results.json` | `baselines.{TextCNN,BiGRU,CodeBERT-frozen}.maxF1_thresh.{f1,precision,recall,pr_auc}` — controlled re-implementations on the identical 5-fold split. |
| `graph_model.json` → `graph_results.json` | `summary.maxF1_train_threshold.mean_f1`, `summary.mean_pr_auc` — slice-graph GNN baseline. |
| `fusion_results.json` | `arms` — frozen-CodeBERT-embedding + srcML-augmented-feature fusion ablation; `n`, `pos` — dataset size / positive count. |
| `finetuned_codebert_results.json` | `per_fold_maxF1` / `per_fold_at05` (per-fold `{f1,precision,recall,pr_auc}`), `maxF1_thresh` / `maxF1_thresh_std` (mean ± std across 5 folds), `status` (`"ok"` or `"all_folds_failed"`). Full fine-tune, not a frozen probe — see `note` field for the exact training config. |

## Provenance — `results/data_provenance.json`

| Field | Type | Description |
|---|---|---|
| `usedSynthetic` | boolean | Must be `false` for every reported result in the paper; `true` would mean a fabricated-data fallback fired (it never does in the shipped results). |
| `datasets` | list | Which real dataset(s) backed each experiment (BigVul only; see README's Data section for why Devign/CodeXGLUE and Defects4J were not used). |
