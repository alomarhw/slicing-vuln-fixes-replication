#!/usr/bin/env bash
# Reproduce this study end-to-end.
#
# Two Python environments are required: the TF-IDF/logistic-regression scripts
# need numpy>=2 (requirements.txt); the CodeBERT/torch scripts need numpy<2 +
# torch==2.2.2, which is ABI-incompatible with numpy>=2 (requirements-codebert.txt).
# This script sets up both, then hands off to run_all.py, which routes each
# step to the right one. See run_all.py's module docstring for the full step list.
set -euo pipefail

cd "$(dirname "$0")"

PY="$(command -v python3 || command -v python || true)"
if [ -z "$PY" ]; then echo "Error: Python 3 not found on PATH." >&2; exit 1; fi

echo "==> Creating main environment (.venv)"
"$PY" -m venv .venv
.venv/bin/pip install --quiet --upgrade pip
.venv/bin/pip install --quiet -r requirements.txt

echo "==> Creating CodeBERT/torch environment (.venv_codebert)"
"$PY" -m venv .venv_codebert
.venv_codebert/bin/pip install --quiet --upgrade pip
.venv_codebert/bin/pip install --quiet -r requirements-codebert.txt

echo "==> Running the full pipeline (run_all.py routes each step to the right venv)"
.venv/bin/python3 run_all.py

echo "==> Done. Results are in results/*.json, figures are in figures/*.png."
echo "    These are the exact files cited in the paper's tables and figures."
