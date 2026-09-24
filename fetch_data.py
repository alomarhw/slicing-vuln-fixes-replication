"""Data setup for the replication package.

The study uses two files, both shipped in this repository and verified here by SHA-256:

  data/bigvul/sample.jsonl        first 20,000 rows of the BigVul test split (main study)
  data/bigvul_full/test.parquet   the full 33,050-row test split (full-population checks)

Both come from the HuggingFace dataset ``bstee615/bigvul`` (split ``test``). A shipped file whose
checksum matches is used as is; a missing or altered file is re-fetched. If a re-fetched file does
not match the checksum (the upstream dataset changed), a warning is printed, because results may
then differ from the paper. Writes data/fetch_manifest.json. Exits non-zero only if a file is
missing and cannot be fetched.
"""

from __future__ import annotations

import hashlib
import json
import os
import sys
import urllib.request
from pathlib import Path

HERE = Path(__file__).resolve().parent
DATA = HERE / "data"
SAMPLE = DATA / "bigvul" / "sample.jsonl"
FULL = DATA / "bigvul_full" / "test.parquet"
SAMPLE_ROWS = 20000
SHA256 = {
    SAMPLE: "413ed55516f293c1c52c945ee12be44f0c08be89fa91b9c7b8147f45ec434e17",
    FULL: "c55b9ea5553d779713cfddc133cc040359f860c75b89f4ef95513fbfe4752b39",
}
FULL_URL = "https://huggingface.co/api/datasets/bstee615/bigvul/parquet/default/test/0.parquet"
BIGVUL_CONTRACT = {
    "datasetId": "bstee615/bigvul", "datasetName": "BigVul", "sourceType": "huggingface",
    "accessUrl": "https://huggingface.co/datasets/bstee615/bigvul", "rqId": "RQ1",
    "loaderRecipe": {"strategy": "huggingface_datasets"},
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def verified(path: Path) -> bool:
    return path.exists() and sha256(path) == SHA256[path]


def fetch_sample() -> None:
    # Same loader that produced the shipped file: HuggingFace datasets, streaming, split "test".
    os.environ["RP_DATA_SAMPLE_ROWS"] = str(SAMPLE_ROWS)
    from rp_data_runtime import fetch_all  # noqa: PLC0415
    fetch_all([BIGVUL_CONTRACT], str(DATA))


def fetch_full() -> None:
    FULL.parent.mkdir(parents=True, exist_ok=True)
    urllib.request.urlretrieve(FULL_URL, FULL)


def main() -> int:
    manifest, ok = {}, True
    for path, fetch in ((SAMPLE, fetch_sample), (FULL, fetch_full)):
        rel = str(path.relative_to(HERE))
        if verified(path):
            print(f"[fetch_data] {rel}: shipped copy verified (sha256 ok)")
            manifest[rel] = {"status": "shipped", "sha256": SHA256[path]}
            continue
        print(f"[fetch_data] {rel}: missing or altered; fetching from HuggingFace (bstee615/bigvul, test)")
        try:
            fetch()
        except Exception as exc:  # network or loader failure
            print(f"[fetch_data][ERROR] {rel}: {exc}", file=sys.stderr)
            ok = False
            continue
        if not path.exists():
            print(f"[fetch_data][ERROR] {rel}: fetch produced no file", file=sys.stderr)
            ok = False
            continue
        digest = sha256(path)
        if digest != SHA256[path]:
            print(f"[fetch_data][WARNING] {rel}: fetched file differs from the paper's copy "
                  f"(sha256 {digest[:12]}...); results may differ from the paper.", file=sys.stderr)
        manifest[rel] = {"status": "fetched", "sha256": digest, "matches_paper": digest == SHA256[path]}
    DATA.mkdir(exist_ok=True)
    (DATA / "fetch_manifest.json").write_text(json.dumps(manifest, indent=2))
    return 0 if ok else 1


if __name__ == "__main__":
    code = main()
    # Hard-exit to skip interpreter/C++ atexit finalizers. Importing `datasets` pulls in PyArrow,
    # whose global ThreadPool destructor can deadlock at shutdown on macOS, hanging the run chain
    # after the data is already written. os._exit bypasses those finalizers.
    sys.stdout.flush()
    sys.stderr.flush()
    os._exit(code)
