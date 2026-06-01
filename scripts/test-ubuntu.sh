#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "==> Building Docker image..."
docker build -f "$REPO_ROOT/devops/docker/linux/Dockerfile" -t scrivi-core-linux "$REPO_ROOT"

echo "==> Running configure + build + test in container..."
docker run --rm -v "$REPO_ROOT:/work" scrivi-core-linux

echo "==> All steps passed."
