#!/bin/sh
# stamp-version.sh — [T-0622] write Scrivi's one version into the BUILT app's Info.plist (an Xcode Run Script phase).
#
#   version  = <VERSION file>.<commits on HEAD>   e.g. 1.0.296   (CFBundleShortVersionString)
#   build    = <commits on HEAD>                  e.g. 296       (CFBundleVersion; TestFlight needs it to increase)
#   commit   = <short hash>[-dirty]               e.g. a1b2c3d   (ScriviCommit; Apple's version fields take numbers only)
#
# ⚠️ Read from git at BUILD time, never checked in: a commit cannot hold its own hash. Linux computes the same numbers
# (platforms/linux/cmake/ScriviVersion.cmake), so the same commit reads the same on every platform.
# ⚠️ The phase names the built Info.plist as its INPUT: that orders it after Xcode writes the plist (it ran first, and its
# values were overwritten, until it did).
# ⚠️ An ARCHIVE (TestFlight) from a tree with uncommitted changes is refused (SP-193 Q2): a tester's build names a commit.
set -e
cd "$SRCROOT"
BASE="$(tr -d '[:space:]' < VERSION)"
COUNT="$(git rev-list --count HEAD 2>/dev/null || echo 0)"
HASH="$(git rev-parse --short=7 HEAD 2>/dev/null || echo unknown)"
DIRTY=""
if [ -n "$(git status --porcelain 2>/dev/null)" ]; then DIRTY="-dirty"; fi
if [ "$ACTION" = "install" ] && [ -n "$DIRTY" ]; then
    echo "error: refusing to archive Scrivi from a tree with uncommitted changes — commit first, so the build names a commit."
    exit 1
fi
PLIST="$TARGET_BUILD_DIR/$INFOPLIST_PATH"
# Set, or add when absent (the iOS target's generated plist has no CFBundleVersion).
put() { /usr/libexec/PlistBuddy -c "Delete :$1" "$PLIST" >/dev/null 2>&1 || true; /usr/libexec/PlistBuddy -c "Add :$1 string $2" "$PLIST"; }
put CFBundleShortVersionString "$BASE.$COUNT"
put CFBundleVersion "$COUNT"
put ScriviCommit "$HASH$DIRTY"
echo "Scrivi $BASE.$COUNT ($HASH$DIRTY)"
