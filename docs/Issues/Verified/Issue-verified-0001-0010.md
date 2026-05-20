# Verified Issues — I-0001 to I-0010

---

## I-0001: git commit fails on CI runners with no global git identity configured

**Status:** ✅ Resolved - Verified
**Platform:** Linux (CI / Ubuntu runner)
**Component:** ScriviCore/src/git/SystemGitProvider.cpp
**Severity:** High
**Sprint:** SP-002

**Description:**
The real-git integration tests (Tests 81–82) passed locally on macOS but failed on the GitHub Actions Ubuntu runner. The `git commit` call returned a non-zero exit code, causing `enableGitSnapshots()` and `createSnapshot()` to propagate a `gitError` failure.

**Expected Behavior:**
Tests 81 and 82 pass on all CI platforms when `git` is available in PATH.

**Actual Behavior:**
Tests 81 and 82 failed on Ubuntu CI with `REQUIRE( result.ok() )` expanding to `false`. The `git commit` command exited non-zero because no committer identity was available.

**Steps to Reproduce:**
1. Run the test suite on a Linux machine (or CI runner) with no global `user.name` / `user.email` in `~/.gitconfig`.
2. Tests 81 and 82 fail even though `git` is in PATH and `SystemGitProvider::available()` returns `true`.

**Impact:**
- CI Ubuntu check reported 2 failed tests out of 82, blocking merge.
- Real-git snapshot functionality silently broken on any machine without a global git identity configured.

**Date Identified:** 2026-05-20

**Root Cause Analysis:**
`git commit --author "Name <email>"` sets the *author* identity but Git still requires a *committer* identity resolvable from configuration (`user.name` / `user.email` in `.gitconfig` or the repo's local config). The GitHub Actions Ubuntu runner has no global git identity, so git exits with an error asking the user to configure their identity.

**Resolution:**

**Fix Date:** 2026-05-20
**Verification Date:** 2026-05-20

**Implementation:**

1. **Pass committer identity via `-c` flags** (`SystemGitProvider.cpp` — `commit()` method)
   - Added `-c user.name=<name>` and `-c user.email=<email>` before the `commit` subcommand.
   - The `-c` flag applies a config override for the single invocation only — no system or global config is touched.
   - Both author and committer identity are now supplied entirely from the `CommitRequest`.

**Code Changes:**
```cpp
// Before
auto r = util::runProcess("git",
    {"commit", "--allow-empty", "--author", authorStr, "-m", req.message},
    rootPath);

// After
auto configName  = "user.name=" + req.author.name;
auto configEmail = "user.email=" + req.author.email;
auto r = util::runProcess("git",
    {"-c", configName, "-c", configEmail,
     "commit", "--allow-empty", "--author", authorStr, "-m", req.message},
    rootPath);
```

**Result:**
- 82/82 tests pass locally on macOS.
- GitHub Actions Ubuntu CI run passed all 82 tests.

**Files Affected:**
- `ScriviCore/src/git/SystemGitProvider.cpp` — `commit()` method

**Verification:**
- ✅ GitHub Actions Ubuntu CI workflow passed (Tests 81 and 82)

**Related Issues:**
- None

---

*Closed: 2026-05-20*
