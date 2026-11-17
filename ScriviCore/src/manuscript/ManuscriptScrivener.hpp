#pragma once
// EP-054 S7 (SP-220, T-0683) — a Scrivener project as a manuscript. The binder gives the structure; each document's RTF
// (ManuscriptRtf) gives its scene's text, written as Scrivi Markdown with the .docx's paragraph writer.
//
// Rulings (user 2026-11-17): Q1 Scrivener 3 (`Files/Data/<UUID>/content.rtf`) and the older layout, Mac 2 / Windows 1
// (`Files/Docs/<ID>.rtf`); anything else refused. Q2 the Draft only: Research, Trash and every item outside the Draft
// reported, not imported; documents not included in compile ARE imported, and named. Q3 + Q3a only a FOLDER is a chapter;
// every text document, with children or not, is a scene in binder order in its nearest folder's chapter; documents straight
// in the Draft make an untitled chapter per run; folders above a chapter (Parts) flattened, one note; an empty folder is an
// empty chapter. Q4 titles kept; synopsis → the scene's `outline`, keywords → `tags`, Scrivener's default statuses → Scrivi's
// (First Draft → draft, Revised Draft → revised, Final Draft / Done → final); other statuses, labels, document notes,
// snapshots, custom metadata, comments, footnotes REPORTED. Q5 the RTF reader. Q6 a folder or a zipped backup.
// ⚠️ Beyond the letter: a paragraph of only `* * *`, `***` or `#` inside a document opens a scene, as S6a's text does.
// `importScrivener` is pure (files are read through `read`); the folder and zip openers read the disk.

#include "manuscript/ManuscriptImport.hpp"
#include "scrivi/Services.hpp"

#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace scrivi::manuscript {

// A file of the project by its path inside the `.scriv` ("Files/Data/…/content.rtf"); nullopt when it is absent.
// Errors (too large, unreadable) are the reader's to return.
using ScrivenerFileReader = std::function<Result<std::optional<std::string>>(std::string_view relativePath)>;

// `scrivx` is the project file's bytes. invalidArgument "notScrivener" when it is not a Scrivener project Scrivi reads (Q1).
// Each scene's entry carries `sceneMeta`: a `scrivi.scene.v1` seed whose title, outline, tags and status a paste gives the
// new scene (Range.hpp).
[[nodiscard]] Result<ImportedManuscript> importScrivener(std::string_view scrivx, const ScrivenerFileReader& read);

// Q6: a `.scriv` folder, read through `fs`, or a zipped backup holding one. invalidArgument "notScrivener" when there is no
// project file; "tooLarge" when a file the import reads is past the cap.
[[nodiscard]] Result<ImportedManuscript> importScrivenerFolder(FileSystem& fs, const std::string& folder);
[[nodiscard]] Result<ImportedManuscript> importScrivenerZip(const std::string& zipPath);

} // namespace scrivi::manuscript
