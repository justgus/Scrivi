#!/usr/bin/env bash
# dumas_world_fixture.sh — SP-128 / T-0501 ([I-0195]) — build a LARGE test world.
#
# ⚠️ NOT a test. It asserts nothing and is not part of the smoke suite. It builds
# a throwaway project + world big enough that opening it is genuinely slow on a
# high-latency mount, which is the only condition under which [I-0195] can be
# verified.
#
# The setting is ALEXANDRE DUMAS' FRANCE — public domain (Dumas died 1870) —
# drawn from Les Trois Mousquetaires, Le Vicomte de Bragelonne (The Man in the
# Iron Mask) and Le Comte de Monte-Cristo. One France, so the relationships are
# real rather than random.
#
# ⚠️ TWO KNOBS, and the live pass needs BOTH:
#   - SCENES make the progress FRACTION MOVE (the bar counts scenes).
#   - WORLD BULK + a slow mount make each step SLOW enough to watch.
# ⚠️ Bulk alone on a local disk opens fast and shows nothing.
#
# ⚠️ It REFUSES to touch an existing project directory, and must NEVER be pointed
# at real writing work (I-0150).
#
# Usage:
#   dumas_world_fixture.sh <harness-binary> <projectDir> [worldPackagePath] [scale]
#
# Scale is a multiplier on both objects and scenes (default 1):
#   scale 1  ≈   224 objects,   384 scenes,  ~1,000 files
#   scale 3  ≈   672 objects, 1,152 scenes,  ~3,000 files
#
# ⚠️ To put the WORLD on the slow mount (which is the point), pass its path
# explicitly, e.g.:
#   dumas_world_fixture.sh "$BIN" ~/ScriviProjects/dumas.scrivi \
#       /mnt/scrivi-net/worlds/Dumas-France.scrivworld 3
set -euo pipefail

BIN="${1:?usage: dumas_world_fixture.sh <harness-binary> <projectDir> [worldPath] [scale]}"
PROJECT_DIR="${2:?usage: dumas_world_fixture.sh <harness-binary> <projectDir> [worldPath] [scale]}"
WORLD_PATH="${3:-}"
SCALE="${4:-1}"

echo "== Dumas world fixture — building a deliberately LARGE project =="
"$BIN" "$PROJECT_DIR" "$WORLD_PATH" "$SCALE"
