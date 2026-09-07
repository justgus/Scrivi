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

} // namespace scrivi::util
