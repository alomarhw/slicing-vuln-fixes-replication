#!/usr/bin/env python3
"""
run_all.py
----------
Single entrypoint that reproduces every experiment and figure in the paper
("Scope Is Not Content: Structural Augmentation of Program Slices for
Vulnerability Fix Localization"), in the correct order, on the correct data.


TWO PYTHON ENVIRONMENTS are required (see requirements.txt vs
requirements-codebert.txt for why): the TF-IDF/logistic-regression scripts use
numpy>=2 / scikit-learn 1.9; the CodeBERT / torch scripts need numpy<2 /
torch==2.2.2, which are mutually incompatible in one venv. Set up both once:

    python3 -m venv .venv              && .venv/bin/pip install -r requirements.txt
    python3 -m venv .venv_codebert     && .venv_codebert/bin/pip install -r requirements-codebert.txt

Steps (5-fold CV on BigVul; seeds 42/1337 as set per-script):
  1. fetch_data.py            [main venv]     -> data/bigvul/sample.jsonl (real HF fetch)
  2. consolidated_study.py    [main venv]     -> results/study_results.json (RQ1)
                                                  figures/fig_localization.png
  3. augmented_study.py       [main venv]     -> results/augmented_results.json
                                                  (RQ2 signature discrimination; RQ3 core
                                                  4-representation detection; real per-fold
                                                  statistics: Wilcoxon, McNemar, bootstrap CIs)
  4. analyze_sink_fallback.py [main venv]     -> results/sink_fallback_analysis.json
                                                  (sink-criterion fallback-rate diagnostic)
  4b. rq1_baselines.py        [main venv]     -> results/rq1_baselines.json
                                                  (region baselines, insertion anchors, miss taxonomy)
  4c. rq1_selective_sinks.py  [main venv]     -> results/rq1_selective_sinks.json
                                                  (narrower sink sets; per-stage timing)
  4d. rq1_full_population.py  [main venv]     -> results/rq1_full_population.json
                                                  (RQ1 on all qualifying test-split pairs; needs
                                                  data/bigvul_full/test.parquet from the HF parquet export)
  4e. rq2_full_population.py  [main venv]     -> results/rq2_full_population.json
  4f. rq1_slice_direction.py  [main venv]     -> results/rq1_slice_direction.json (needs srcml+srcslice)
  4g. rq1_inspection_effort.py [main venv]    -> results/rq1_inspection_effort.json
  4h. rq3_grouped_cv.py       [main venv]     -> results/rq3_grouped_cv.json (CVE-grouped RQ3)
  4i. rq3_codebert_repeated.py embed [codebert venv] then probe [main venv]
                                              -> results/rq3_codebert_repeated.json
  4j. rq3_grouped_cv.py project -> results/rq3_grouped_cv_project.json
  4k. rq3_normalized_tokens.py  -> results/rq3_normalized_tokens.json
  4l. rq1_sink_density.py       -> results/rq1_sink_density.json
  Optional: joern_validation_rq3/ (joern-parse src; backward_slice.sc; rq3_joern_scope.py)
                                -> results/rq3_joern_scope.json
  Optional, external tools/data (not run by this script):
    joern_validation/: joern-parse src -o cpg.bin; joern --script backward_slice.sc
        --param cpgFile=cpg.bin --param outFile=joern_slices.jsonl; python compare.py
        -> results/joern_validation.json (Joern 4.0.635)
    rq1_recent_cves.py REPO... (clones of curl, FFmpeg, ImageMagick, libarchive, libtiff,
        libxml2, openjpeg, openssl, php-src, tcpdump) -> results/rq1_recent_cves.json
    slice_audit.py PATH       sink-anchored reading orders for any C code base
  5. baselines_sota.py        [codebert venv] -> results/baselines_results.json
                                                  (TextCNN/BiGRU/CodeBERT-frozen baselines)
  6. graph_model.py           [codebert venv] -> results/graph_results.json (slice-graph GNN)
  7. fusion_study.py          [codebert venv] -> results/fusion_results.json (fusion ablation)
  8. finetune_codebert.py     [codebert venv] -> results/finetuned_codebert_results.json
                                                  (fine-tuned, not frozen, CodeBERT baseline)
  9. make_figures.py          [main venv]     -> figures/fig_signature_aug.png,
                                                  fig_detection_aug.png, fig_baselines.png

Each step is a separate, independently-runnable script (kept that way
deliberately -- this is how the study was actually developed and validated
step by step). This file only sequences them and stops on the first hard
failure so a partial run is never silently reported as complete.

Usage:
  python3 run_all.py               # uses .venv / .venv_codebert next to this file if present,
                                    # else falls back to `python3` / `python3` (same interpreter)
"""

import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def _venv_python(venv_name: str) -> str:
    candidate = os.path.join(HERE, venv_name, "bin", "python3")
    return candidate if os.path.isfile(candidate) else sys.executable


MAIN_PY = _venv_python(".venv")
CODEBERT_PY = _venv_python(".venv_codebert")

STEPS = [
    ("Fetching data (BigVul via HuggingFace)", [MAIN_PY, "fetch_data.py"]),
    ("RQ1: fix-localization (consolidated_study.py)", [MAIN_PY, "consolidated_study.py"]),
    ("RQ2 + RQ3 core: srcML augmentation (augmented_study.py)", [MAIN_PY, "augmented_study.py"]),
    ("RQ1 diagnostic: sink-criterion fallback rate (analyze_sink_fallback.py)",
     [MAIN_PY, "analyze_sink_fallback.py"]),
    ("RQ1: region baselines + insertion anchors + miss taxonomy (rq1_baselines.py)",
     [MAIN_PY, "rq1_baselines.py"]),
    ("RQ1: sink-selectivity analysis + timing (rq1_selective_sinks.py)",
     [MAIN_PY, "rq1_selective_sinks.py"]),
    ("RQ1: full-population replication on the 33,050-row test split (rq1_full_population.py)",
     [MAIN_PY, "rq1_full_population.py"]),
    ("RQ2: full-population replication (rq2_full_population.py)",
     [MAIN_PY, "rq2_full_population.py"]),
    ("RQ1: slice direction -- backward vs srcSlice forward vs chop (rq1_slice_direction.py)",
     [MAIN_PY, "rq1_slice_direction.py"]),
    ("RQ1: inspection effort of reading orders (rq1_inspection_effort.py)",
     [MAIN_PY, "rq1_inspection_effort.py"]),
    ("RQ3: CVE-grouped CV sensitivity check (rq3_grouped_cv.py)",
     [MAIN_PY, "rq3_grouped_cv.py"]),
    ("RQ3: project-grouped CV sensitivity check (rq3_grouped_cv.py project)",
     [MAIN_PY, "rq3_grouped_cv.py", "project"]),
    ("RQ3: SySeVR-style normalized-token conditions (rq3_normalized_tokens.py)",
     [MAIN_PY, "rq3_normalized_tokens.py"]),
    ("RQ1: sink density vs slice/variable-mention agreement (rq1_sink_density.py)",
     [MAIN_PY, "rq1_sink_density.py"]),
    ("Figures: RQ1 region map and inspection effort (make_rq1_figures.py)",
     [MAIN_PY, "make_rq1_figures.py"]),
    ("RQ3: frozen CodeBERT embeddings for the same-protocol probe (rq3_codebert_repeated.py embed)",
     [CODEBERT_PY, "rq3_codebert_repeated.py", "embed"]),
    ("RQ3: frozen CodeBERT probe on the repeated-CV splits (rq3_codebert_repeated.py probe)",
     [MAIN_PY, "rq3_codebert_repeated.py", "probe"]),
    ("RQ3: SOTA baselines -- TextCNN/BiGRU/CodeBERT-frozen (baselines_sota.py)",
     [CODEBERT_PY, "baselines_sota.py"]),
    ("RQ3: slice-graph GNN baseline (graph_model.py)", [CODEBERT_PY, "graph_model.py"]),
    ("RQ3: CodeBERT+AugWhole fusion ablation (fusion_study.py)", [CODEBERT_PY, "fusion_study.py"]),
    ("RQ3: fine-tuned CodeBERT baseline (finetune_codebert.py)",
     [CODEBERT_PY, "finetune_codebert.py"]),
    ("Figures: RQ2/RQ3 charts (make_figures.py)", [MAIN_PY, "make_figures.py"]),
    ("Figure: method diagram (make_method_diagram.py)", [MAIN_PY, "make_method_diagram.py"]),
]


def main() -> int:
    print(f"main venv python:     {MAIN_PY}")
    print(f"codebert venv python: {CODEBERT_PY}")
    if MAIN_PY == CODEBERT_PY:
        print("WARNING: .venv / .venv_codebert not found next to this script -- falling back to "
              f"the current interpreter ({sys.executable}) for every step. The CodeBERT/torch "
              "steps will fail unless this interpreter already satisfies "
              "requirements-codebert.txt. See the module docstring for the two-venv setup.")
    for label, cmd in STEPS:
        print(f"\n==> {label}")
        result = subprocess.run(cmd, cwd=HERE)
        if result.returncode != 0:
            print(f"\n[run_all] FAILED at: {label} (exit {result.returncode})", file=sys.stderr)
            print("[run_all] Stopping -- later steps depend on this one's output.", file=sys.stderr)
            return result.returncode
    print("\n==> Done. Results are in results/*.json, figures are in figures/*.png.")
    print("    These are the exact files cited in the paper's tables and figures.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
