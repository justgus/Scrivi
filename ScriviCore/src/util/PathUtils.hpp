#pragma once

#include "scrivi/Types.hpp"
#include "scrivi/Result.hpp"

#include <string_view>

namespace scrivi::util {

AbsolutePath join(const AbsolutePath& base, std::string_view relative);

std::string extension(const AbsolutePath& path);

AbsolutePath replaceExtension(const AbsolutePath& path, std::string_view newExt);

std::string filename(const AbsolutePath& path);

AbsolutePath parent(const AbsolutePath& path);

Result<AbsolutePath> makeAbsolute(const RelativePath& rel, const AbsolutePath& base);

// ---------------------------------------------------------------------------
// Path validation (I-0191)
// ---------------------------------------------------------------------------
//
// `AbsolutePath` is a bare `std::string` alias (Types.hpp): the NAME asserts an
// invariant that the TYPE cannot enforce. Nothing in the core checked it, so a
// junk or empty root reached `create_directories` and was resolved against the
// process CWD -- which for a dev build is the repository root. That is how three
// directories named from raw control bytes appeared in the repo on 2026-08-17.
//
// These are the central enforcement point. Do NOT re-implement either rule at a
// call site: a restated rule is the defect that keeps recurring in this codebase.

// True if `s` contains any C0 control byte (< 0x20) or DEL (0x7F).
//
// ScriviCore must NEVER create a path containing unprintable characters. Such a
// name is never a legitimate request: it is the signature of a dangling or
// uninitialised `const char*` that crossed the C ABI, and creating it silently
// converts a memory-safety bug into filesystem litter.
bool containsControlCharacter(std::string_view s);

// Validates a root path supplied from outside the core (an `appSupportRoot`, a
// project root). Fails when the path is empty, relative, or holds a control
// character. Callers MUST check this BEFORE creating anything, so a rejected
// root never leaves a partial tree behind.
Result<void> validateRootPath(const AbsolutePath& path, std::string_view label);

// ---------------------------------------------------------------------------
// Filesystem artifacts that are not project content (I-0221)
// ---------------------------------------------------------------------------
//
// True when `filename` names a file the OS created beside real content -- never
// something Scrivi wrote and never something a scan should read.
//
// ⚠️ THE CASE THAT MADE THIS NECESSARY. On any volume without native extended
// attributes (FAT32, exFAT, most SMB shares), macOS stores xattrs in an
// "AppleDouble" sidecar named `._<original>`. So `001-scene.meta.json` acquires
// a binary companion `._001-scene.meta.json` -- which ENDS IN `.meta.json` and
// therefore passed every suffix test in the manuscript scan. `parseSceneMeta`
// was handed an AppleDouble blob (magic 0x00051607) and the whole project open
// failed with a JSON error naming no file.
//
// ⚠️ IT IS NOT A ONE-TIME IMPORT ARTIFACT, which is why ignoring it is the only
// available fix. Scrivi is sandboxed, so macOS stamps `com.apple.quarantine` on
// every file it writes to a user-selected location; on a volume with no xattr
// support that stamp MATERIALISES A NEW SIDECAR. Measured 2026-09-17: a plain
// write creates none, setting any xattr creates one instantly, and files Scrivi
// had just written carried `com.apple.quarantine: 0082;...;Scrivi;`.
// ⚠️ So the app REGENERATED the files that stopped it opening: clean them by
// hand and the next save re-created them. They cannot be prevented from a
// sandboxed app -- they can only be ignored on read.
//
// ⚠️ DO NOT RE-IMPLEMENT THIS TEST AT A CALL SITE. Three scans restated "what
// counts as a scene file" independently and only one of them was fatal, which is
// precisely why the defect was so hard to see. Call this instead.
bool isIgnorableFilesystemArtifact(std::string_view filename);

} // namespace scrivi::util
