#!/usr/bin/env bash
# Runs the macOS interop tests (ScriviInteropTests) WITHOUT quitting a running Scrivi.
#
# ⚠️ WHY THIS EXISTS. The tests are HOSTED in Scrivi.app (TEST_HOST), and LaunchServices
# refuses to launch a second app with the SAME BUNDLE IDENTIFIER — measured 2026-10-03:
# "Could not launch ScriviInteropTests", even from a separate DerivedData path. So the
# test host is built under its own ID (com.caposoft.scrivi.testhost, via the
# SCRIVI_TEST_HOST_ID_SUFFIX build setting, empty by default) into its own DerivedData.
#
# ✅ Side benefit: a different bundle ID is a different sandbox container, so the test
# host never sees the writer's session state (the I-0150 class).
#
# ⚠️ The test host declares Scrivi's document types, so it is UNREGISTERED from
# LaunchServices afterwards — otherwise Finder could pick it to open a .scrivi package.
set -uo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DERIVED="$REPO_ROOT/build-xctest"
LSREGISTER=/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister

xcodebuild -workspace "$REPO_ROOT/Scrivi.xcworkspace" -scheme ScriviApp \
    -destination 'platform=macOS' -derivedDataPath "$DERIVED" \
    SCRIVI_TEST_HOST_ID_SUFFIX=.testhost "$@" test
status=$?

"$LSREGISTER" -u "$DERIVED/Build/Products/Debug/Scrivi.app" 2>/dev/null || true
exit $status
