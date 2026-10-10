#pragma once
// SP-174 / T-0601 (EP-048 S3a) — THE FORMAT COMMANDS AND BALANCED EDITS, in the core for both platforms. A port of Apple's
// `ManuscriptCommands.swift` and the presenter's balancing core (`tokens(in:)`, `balancedEdit`, `insertionStyle`, `rewrite`,
// `balancedCopy`), line for line, on ONE SCENE's text in UTF-16 units (a block never crosses a scene).
//
// Each command returns ONE edit (range + replacement + the selection after), which the caller applies as one history event, or
// nothing to refuse. ✅ Bold / italic are STYLE edits on the emphasis tokens, written back balanced; a paragraph is ONE of body /
// heading / list item; numbered items stay sequential; an edit that replaces a selection keeps emphasis balanced (EP-046 AC12).

#include "markdown/Emphasis.hpp"
#include "markdown/MarkdownText.hpp"

#include <map>
#include <optional>
#include <utility>
#include <vector>

namespace scrivi::markdown::editing {

// One scene's text, with each block's analysis computed once.
class Scene {
public:
    explicit Scene(Units text) : text_(std::move(text)) {}

    [[nodiscard]] const Units& text() const { return text_; }
    [[nodiscard]] int length() const { return static_cast<int>(text_.size()); }
    [[nodiscard]] char16_t at(int i) const { return text_[static_cast<std::size_t>(i)]; }
    [[nodiscard]] UnitsView slice(URange r) const {
        return UnitsView(text_).substr(static_cast<std::size_t>(r.location), static_cast<std::size_t>(r.length));
    }

    // The block (a maximal run of non-blank lines) holding `loc`, or nothing on a blank line.
    [[nodiscard]] std::optional<URange> blockAt(int loc) const;
    // A block's analysis (offsets relative to the block).
    [[nodiscard]] const Analysis16& info(URange block) const;
    // The blocks overlapping the CLOSED range [a, b].
    [[nodiscard]] std::vector<URange> blocksIn(int a, int b) const;
    // Tokens (every unit except emphasis markers, with its block's style) of a range; units outside a block carry no style.
    [[nodiscard]] std::vector<emphasis::Token> tokensIn(URange r) const;
    // The emphasis spans of the blocks overlapping [a, b].
    [[nodiscard]] std::vector<URange> spansIn(int a, int b) const;
    // The next / previous block, skipping blank lines.
    [[nodiscard]] std::optional<URange> nextBlockAfter(int p) const;
    [[nodiscard]] std::optional<URange> previousBlockBefore(int p) const;

private:
    struct Line { URange line; bool endsScene = false; };
    [[nodiscard]] Line lineContaining(int loc) const;
    [[nodiscard]] bool isBlank(URange r) const;

    Units text_;
    mutable std::map<Units, Analysis16> cache_;
};

struct Edit {
    URange range;
    Units replacement;
    URange selection;
};

enum class ParagraphFormat { body, heading1, heading2, heading3, bulletList, numberedList };

// The word the caret is IN (touching a letter or digit on either side), or nothing BETWEEN words. An escaped apostrophe or
// hyphen inside a word (`don\'t`, `well\-known`) keeps it one word.
[[nodiscard]] std::optional<URange> wordRange(const Scene& s, int caret);

// Toggle `bit` (bold or italic) on the visible text of `sel`: SET it when any visible selected character lacks it, CLEAR it
// when all have it. An edge that cannot carry it is shrunk away. `caret`: when a WORD was formatted for a caret, the caret to
// keep (the result's selection is then that caret). Nothing when it cannot be done without destroying formatting.
[[nodiscard]] std::optional<Edit> emphasisEdit(const Scene& s, URange sel, std::uint8_t bit, std::optional<int> caret = {});

// Set every paragraph the selection touches to `format`; the same command again on paragraphs that all have it returns them to
// body text. Numbered items after them are renumbered in the same edit.
[[nodiscard]] std::optional<Edit> paragraphEdit(const Scene& s, URange sel, ParagraphFormat format);

// Return with the caret in a list item: on an EMPTY item end the list; otherwise start the next item and renumber. Nothing when
// the caret is not in a list item.
[[nodiscard]] std::optional<Edit> listReturnEdit(const Scene& s, URange sel);

// The edit that keeps emphasis balanced when `range` is replaced by `replacement` (stored form), or nothing when the plain edit
// is already balanced. `caret` is where the caret goes after it. `keepsOwnFormatting`: Scrivi's own copy being pasted keeps ITS
// styles instead of taking the caret's.
struct BalancedEdit {
    URange range;
    Units replacement;
    int caret = 0;
};
[[nodiscard]] std::optional<BalancedEdit> balancedEdit(const Scene& s, URange range, UnitsView replacement,
                                                       bool keepsOwnFormatting = false);

// The style new text takes when it replaces `selected` at `a`.
[[nodiscard]] std::uint8_t insertionStyle(const Scene& s, int a, const std::vector<emphasis::Token>& selected);
[[nodiscard]] std::uint8_t styleAtCaret(const Scene& s, int caret);

// What a copy of `sel` puts on the pasteboards: the BALANCED source (Scrivi's own paste keeps the formatting) and the PRESENTED
// text (other apps).
struct Copy {
    Units source;
    Units presented;
};
[[nodiscard]] Copy balancedCopy(const Scene& s, URange sel);

// A cross-scene paste lands INSIDE the caret's span: the FIRST piece starts with that span open; the LAST re-opens it.
[[nodiscard]] std::vector<Units> balancePastePieces(const std::vector<Units>& pieces, std::uint8_t caretStyle);

} // namespace scrivi::markdown::editing
