# Scope Is Not Content: An Empirical Study of Program Slicing on Real Vulnerability Fixes

Replication package for the SANER 2027 research-track submission. Every number in the
paper's tables comes from a JSON file in `results/` written by a script in this directory.

## What the study asks

- **RQ1 (localization)**: how much of a real CVE fix does an intraprocedural backward slice
  from sinks cover, computed from `func_before` only? Compared with five region baselines,
  narrower sink sets, forward slicing (srcSlice), source-to-sink chops, and a Joern reference
  slicer; replicated on the full BigVul test split and on CVE fixes committed since 2020;
  plus inspection-effort metrics for reading orders.
- **RQ2 (fix signatures)**: do abstract srcML feature deltas discriminate fixes better than
  srcSlice's per-variable (forward-slice) profile? Replicated on the full test split.
- **RQ3 (detection)**: does abstract structural content improve lightweight detection, and
  does the slice's scope matter once content is held fixed? Repeated 30x5-fold CV with
  Nadeau-Bengio corrected tests and Holm correction; CVE-grouped sensitivity check; frozen
  CodeBERT under the same protocol.

No patches are generated anywhere.

## Environment

- Python 3.12 (tested on macOS 15, Intel Xeon W-2140B desktop; CodeBERT on the Metal (MPS) GPU backend or CPU).
- Two virtual environments, because `torch==2.2.2` needs `numpy<2` while the rest needs
  `numpy>=2`: `requirements.txt` (main) and `requirements-codebert.txt` (CodeBERT scripts).
- External tools on `PATH`: `srcml` 1.1.0 (https://www.srcml.org) and `srcslice`
  (https://github.com/srcML/srcSlice), used for the RQ2 coarse arm and the forward slices.
  The tree-sitter C grammar is installed from `requirements.txt` (tree-sitter 0.25.2,
  tree-sitter-c 0.24.2).
- Optional: Joern 4.0.635 (slicer validation) and local clones of ten C projects (recent-CVE
  replication); see below.

## Reproduce

```bash
./reproduce.sh          # creates both venvs, then runs run_all.py
```

`run_all.py` runs every step in order and stops on the first failure; its docstring lists each
script, its output, and its venv. Its first step, `fetch_data.py`, verifies the two shipped data
files by SHA-256 and re-fetches only a missing or altered one:

| File | Content | Source |
|---|---|---|
| `data/bigvul/sample.jsonl` | first 20,000 rows of the BigVul test split (main study) | HuggingFace `bstee615/bigvul`, split `test` |
| `data/bigvul_full/test.parquet` | full 33,050-row test split (full-population checks) | HuggingFace parquet export of the same split |

Every CPU step reproduces the shipped `results/*.json` exactly from a fresh clone. Most steps take
seconds to a few minutes; `augmented_study.py` (RQ2/RQ3 core) takes longest among them. Fine-tuning
CodeBERT is by far the slowest step and needs a GPU (we used the Metal backend).

## Paper table -> script -> result file

| Paper element | Script | Result file |
|---|---|---|
| RQ1 coverage and strata (baseline table, left) | `consolidated_study.py`, `rq1_baselines.py` | `study_results.json`, `rq1_baselines.json` |
| RQ1 full population (baseline table, right) | `rq1_full_population.py` | `rq1_full_population.json` |
| RQ1 sink selectivity (region-map figure, a), timing | `rq1_selective_sinks.py` | `rq1_selective_sinks.json` |
| RQ1 slice direction (region-map figure, b) | `rq1_slice_direction.py` | `rq1_slice_direction.json` |
| RQ1 Joern validation | `joern_validation/` (see below) | `joern_validation.json` |
| RQ1 inspection effort (effort figure) | `rq1_inspection_effort.py` | `rq1_inspection_effort.json` |
| RQ1 recent CVE fixes (recent-fix table; effort figure, b) | `rq1_recent_cves.py REPO...` | `rq1_recent_cves.json`, `rq1_recent_cves_pairs.jsonl` |
| RQ1 sink fallback diagnostic | `analyze_sink_fallback.py` | `sink_fallback_analysis.json` |
| RQ2 delta rates, held-out matching, CWE breakdown | `augmented_study.py` | `augmented_results.json` (`rq2_aug`) |
| RQ2 full population | `rq2_full_population.py` | `rq2_full_population.json` |
| RQ3 detection and tests | `augmented_study.py` | `augmented_results.json` (`rq3_aug`) |
| RQ3 CVE- and project-grouped CV | `rq3_grouped_cv.py`, `rq3_grouped_cv.py project` | `rq3_grouped_cv.json`, `rq3_grouped_cv_project.json` |
| RQ3 SySeVR-style normalized tokens | `rq3_normalized_tokens.py` | `rq3_normalized_tokens.json` |
| RQ3 scope with Joern slices | `joern_validation_rq3/` (see below) | `rq3_joern_scope.json` |
| RQ1 sink density | `rq1_sink_density.py` | `rq1_sink_density.json` |
| RQ3 frozen CodeBERT, same protocol | `rq3_codebert_repeated.py embed` (codebert venv), then `probe` (main venv) | `rq3_codebert_repeated.json` |
| RQ3 context baselines | `baselines_sota.py`, `graph_model.py`, `finetune_codebert.py`, `fusion_study.py` (codebert venv) | `baselines_results.json`, `graph_results.json`, `finetuned_codebert_results.json`, `fusion_results.json` |
| Fig. 1 | `make_fig1.py` | `figures/method_diagram.pdf` |
| RQ1 region-map and effort figures | `make_rq1_figures.py` | `figures/fig_region_map.pdf`, `figures/fig_effort.pdf`, `results/figure_data.json` |

## Optional steps (external tools or data)

- **Joern validation**: `cd joern_validation; joern-parse src -o cpg.bin;
  joern --script backward_slice.sc --param cpgFile=cpg.bin --param outFile=joern_slices.jsonl;
  python compare.py`. `src/` holds the 294 RQ1 functions; `backward_slice.sc` seeds the same sink
  categories and follows Joern's reaching-definition and control-dependence edges.
  `joern_validation_rq3/` repeats this for the 2,500 RQ3 detection functions (`src/d*.c`), then
  `python joern_validation_rq3/rq3_joern_scope.py` compares phi over Joern's slice, our slice,
  and the variable-mention region.
- **Recent CVE fixes**: clone curl, FFmpeg, ImageMagick, libarchive, libtiff, libxml2,
  openjpeg, openssl, php-src, and tcpdump, then `python rq1_recent_cves.py <clone dirs>`.
  Fix commits are non-merge commits since 2020-01-01 citing a CVE id; test, fuzzing, vendored
  code, and release/library-update commits are skipped (see the script's docstring).

## Tool

`slice_audit.py PATH... [--features] [--show N] [--out report.jsonl]` emits, for every function
of a C code base, its sink lines, backward slice, sink-proximity reading order, and optionally the
abstract srcML features over the slice (about 300 functions/s without features on the machine above).

## Core modules

- `ast_slicer.py`: our tree-sitter backward slicer (Algorithm 1). It is **not** srcSlice, which
  computes forward slices per variable; srcSlice is used only for the RQ2 coarse profile and the
  forward-slice comparison.
- `augmented_study.py`: srcML feature extraction (`srcml_features`), representations, RQ2/RQ3.
- `proto_dualsig.py`: srcSlice wrapper and dual-signature matching.

## Support files and history

`rp_data_runtime.py` (dataset fetching), `rp_style.py` and `sitecustomize.py` (figure style),
and `rp_entry.py` (hard-exit wrapper for a PyArrow/OpenMP shutdown deadlock) support the
pipeline. `results/*.BROKEN_*`,
`*.INVALID_*`, and `*.bak_*` files are earlier runs kept for transparency and are not cited.

## Data

BigVul (https://huggingface.co/datasets/bstee615/bigvul): `data/bigvul/sample.jsonl` holds the
first 20,000 rows of the test split used by the main study.

## License

The code in this repository is released under the MIT License (`LICENSE`). The BigVul data files
in `data/` are redistributed from the HuggingFace dataset `bstee615/bigvul` and remain under their
original terms.
