"""ResearchPilot deterministic data fetcher.

Generated from Architect/Engineer data contracts. This script fetches real public data into data/
and writes data/fetch_manifest.json. It exits 0 even when a source fails so main.py can produce a
clear strict-real-data error instead of hiding the issue behind a shell failure.
"""

from __future__ import annotations

import json
import os
import sys

from rp_data_runtime import fetch_all


CONTRACTS = json.loads('[{"accessUrl": "https://huggingface.co/datasets/bstee615/bigvul", "dataRequirements": {"confidence": 0.0, "labelField": {"canonical": "label", "type": "unknown"}, "modality": "unknown", "rejectModalities": [], "requiredColumns": [], "source": "inferred", "taskType": "unknown"}, "datasetId": "bstee615/bigvul", "datasetName": "BigVul", "experimentId": "RQ1-E1", "issues": [], "loaderRecipe": {"codeHint": "Use datasets.load_dataset(\'bstee615/bigvul\', streaming=True when possible); materialize a small deterministic sample/split into data/ for the default run.", "preprocessHint": "Handle DatasetDict splits; flatten nested records only if needed by the metrics.", "pythonPackages": ["datasets"], "strategy": "huggingface_datasets"}, "provenance": {"checksum": "", "citation": "", "license": "", "preprocessing": [], "recordCount": null, "schema": [], "version": ""}, "rqId": "RQ1", "source": "public_benchmark", "sourceType": "huggingface", "status": "verified", "verification": "registry_id_resolved"}, {"accessUrl": "https://huggingface.co/datasets/google/code_x_glue_cc_defect_detection", "dataRequirements": {"confidence": 0.0, "labelField": {"canonical": "label", "type": "unknown"}, "modality": "unknown", "rejectModalities": [], "requiredColumns": [], "source": "inferred", "taskType": "unknown"}, "datasetId": "google/code_x_glue_cc_defect_detection", "datasetName": "Juliet Test Suite", "experimentId": "RQ1-E2", "issues": [], "loaderRecipe": {"codeHint": "Use datasets.load_dataset(\'google/code_x_glue_cc_defect_detection\', streaming=True when possible); materialize a small deterministic sample/split into data/ for the default run.", "preprocessHint": "Handle DatasetDict splits; flatten nested records only if needed by the metrics.", "pythonPackages": ["datasets"], "strategy": "huggingface_datasets"}, "provenance": {"checksum": "", "citation": "", "license": "", "preprocessing": [], "recordCount": null, "schema": [], "version": ""}, "rqId": "RQ1", "source": "public_benchmark", "sourceType": "huggingface", "status": "verified", "verification": "registry_id_resolved"}]')


def main() -> int:
    try:
        fetch_all(CONTRACTS, "data")
        return 0
    except Exception as exc:  # fetch_all already isolates per-source failures into the manifest
        print(f"[fetch_data.py][FATAL] {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    code = main()
    # Hard-exit to skip interpreter/C++ atexit finalizers. Importing `datasets`/`pandas` pulls in
    # PyArrow, whose global ThreadPool destructor can deadlock at shutdown on macOS (and torch's
    # OpenMP runtime compounds it) — leaving `python fetch_data.py` hung forever AFTER the data is
    # already fetched and the manifest written, which would stall the `fetch_data.py && main.py`
    # run chain. os._exit bypasses those finalizers; all real work + flushing is already done.
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(code)
