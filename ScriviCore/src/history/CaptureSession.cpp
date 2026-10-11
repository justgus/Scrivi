#include "history/CaptureSession.hpp"

namespace scrivi::history {

namespace {

// Decode one UTF-8 scalar at `i`; returns its byte length (1 for a malformed byte, which then counts as one unit).
std::size_t decode(const std::string& s, std::size_t i, char32_t& cp)
{
    const auto c = static_cast<unsigned char>(s[i]);
    auto cont = [&](std::size_t k) { return static_cast<unsigned char>(s[i + k]) & 0x3F; };
    if (c < 0x80) { cp = c; return 1; }
    if ((c >> 5) == 0x6 && i + 1 < s.size()) { cp = ((c & 0x1F) << 6) | cont(1); return 2; }
    if ((c >> 4) == 0xE && i + 2 < s.size()) { cp = ((c & 0x0F) << 12) | (cont(1) << 6) | cont(2); return 3; }
    if ((c >> 3) == 0x1E && i + 3 < s.size()) {
        cp = ((c & 0x07) << 18) | (cont(1) << 12) | (cont(2) << 6) | cont(3);
        return 4;
    }
    cp = 0xFFFD;
    return 1;
}

// Foundation's `CharacterSet.whitespacesAndNewlines` (Apple's `isWhitespaceOnlyDelta`): Unicode Z* plus U+0009–U+000D, U+0085.
bool isWhitespace(char32_t c)
{
    return (c >= 0x09 && c <= 0x0D) || c == 0x20 || c == 0x85 || c == 0xA0 || c == 0x1680 || (c >= 0x2000 && c <= 0x200A)
           || c == 0x2028 || c == 0x2029 || c == 0x202F || c == 0x205F || c == 0x3000;
}

std::u32string withoutWhitespace(const std::string& s)
{
    std::u32string out;
    for (std::size_t i = 0; i < s.size();) {
        char32_t cp = 0;
        i += decode(s, i, cp);
        if (!isWhitespace(cp)) {
            out.push_back(cp);
        }
    }
    return out;
}

}   // namespace

std::int64_t CaptureSession::utf8Offset(const std::string& text, int utf16)
{
    // Apple: `sceneLocalByteOffset` — the UTF-8 length of the first `utf16` units, clamped to the text.
    int units = 0;
    std::size_t i = 0;
    while (i < text.size() && units < utf16) {
        char32_t cp = 0;
        const std::size_t n = decode(text, i, cp);
        units += cp >= 0x10000 ? 2 : 1;
        i += n;
    }
    return static_cast<std::int64_t>(i);
}

bool CaptureSession::isCommitBoundary(const std::string& text, int caretUtf16)
{
    // Apple: `isCommitBoundary` — the UTF-16 unit just before the caret is '.' '!' '?' '\n' or '\r' (✅ SP-177 Q4).
    const int target = caretUtf16 - 1;
    if (target < 0) {
        return false;
    }
    int units = 0;
    for (std::size_t i = 0; i < text.size();) {
        char32_t cp = 0;
        i += decode(text, i, cp);
        if (units == target) {
            return cp == U'.' || cp == U'!' || cp == U'?' || cp == U'\n' || cp == U'\r';
        }
        units += cp >= 0x10000 ? 2 : 1;
        if (units > target) {
            return false;   // the low half of a surrogate pair — never one of these
        }
    }
    return false;
}

bool CaptureSession::isWhitespaceOnlyDelta(const std::string& a, const std::string& b)
{
    return withoutWhitespace(a) == withoutWhitespace(b);
}

std::int64_t CaptureSession::idleDeadline() const
{
    return hasPending() && lastActivityMs_ ? *lastActivityMs_ + kIdleMs : -1;
}

void CaptureSession::seedIfNeeded(CaptureSink& sink, const std::string& sceneID, const std::string& baseline)
{
    if (seeded_.count(sceneID) != 0) {
        return;
    }
    seeded_.insert(sceneID);
    lastCommittedText_ = baseline;   // the whitespace-delta reference starts at the floor
    sink.seed(sceneID, baseline);
}

void CaptureSession::clearPending()
{
    pendingSceneID_.reset();
    pendingText_.clear();
    pendingCursorBefore_ = 0;
    pendingCursorAfter_ = 0;
}

bool CaptureSession::noteEdit(CaptureSink& sink, const std::string& sceneID, const std::string& text, int caretUtf16,
                              const std::string& textBefore, std::int64_t nowMs)
{
    caretScene_ = sceneID;   // the edit's own selection change is not a move (Apple: `lastCursorSegmentIndex`)
    seedIfNeeded(sink, sceneID, textBefore);
    bool committed = false;
    // T-0396: an idle gap ends the typing session — commit what was pending BEFORE folding this edit in.
    if (pendingSceneID_ && isIdle(nowMs)) {
        committed = flush(sink, "idle");
    }
    const std::int64_t cursor = utf8Offset(text, caretUtf16);
    if (!pendingSceneID_) {
        pendingCursorBefore_ = cursor;   // first change since the last commit — where the caret was
    }
    pendingSceneID_ = sceneID;
    pendingText_ = text;
    pendingCursorAfter_ = cursor;
    lastActivityMs_ = nowMs;
    // §4.a trigger 1: a sentence terminator or Return — not trailing whitespace, which rides into the next event.
    if (isCommitBoundary(text, caretUtf16)) {
        committed = flush(sink, "sentence") || committed;
    }
    return committed;
}

bool CaptureSession::caretMoved(CaptureSink& sink, const std::string& sceneID, bool editInFlight, bool applying)
{
    bool committed = false;
    if (!applying) {
        const bool switched = !caretScene_ || *caretScene_ != sceneID;
        if (switched) {
            committed = flush(sink, "sceneSwitch");   // leaving a scene: commit its pending edit (hard)
        } else if (!editInFlight && hasPending()) {
            committed = flush(sink, "cursorMove", "typing", /*soft=*/true);   // a whitespace-only change waits
        }
    }
    caretScene_ = sceneID;
    return committed;
}

void CaptureSession::begin(CaptureSink& sink, const std::string& kind, const std::string& bufferID)
{
    flush(sink, kind);   // close out prior typing as its own step
    nextKind_ = kind;
    if (!bufferID.empty()) {
        nextBufferID_ = bufferID;
    }
}

bool CaptureSession::flush(CaptureSink& sink, const std::string& trigger, const std::string& kind, bool soft)
{
    (void)trigger;   // the label is the caller's; the engine records the KIND
    if (!pendingSceneID_) {
        nextKind_.reset();   // nothing pending — drop any stale hint
        nextBufferID_.reset();
        return false;
    }
    if (soft && !nextKind_ && isWhitespaceOnlyDelta(lastCommittedText_, pendingText_)) {
        return false;
    }
    const std::string effectiveKind = nextKind_.value_or(kind);
    const std::string bufferID = nextBufferID_.value_or("");
    nextKind_.reset();
    nextBufferID_.reset();
    const auto r = sink.record(*pendingSceneID_, pendingText_, effectiveKind, pendingCursorBefore_, pendingCursorAfter_,
                               bufferID, "");
    noteEngineState(r.canUndo, r.canRedo);
    if (!r.noOp) {
        lastCommittedText_ = pendingText_;
    }
    clearPending();
    lastActivityMs_.reset();   // the entry is sealed — the session clock resets
    return !r.noOp;
}

bool CaptureSession::beforeSave(CaptureSink& sink, std::int64_t nowMs)
{
    // T-0396: mid-session a save records nothing; it is the boundary only when the writer has already gone idle.
    if (!isIdle(nowMs)) {
        return false;
    }
    const bool committed = flush(sink, "idle");
    lastActivityMs_.reset();
    return committed;
}

bool CaptureSession::idle(CaptureSink& sink, std::int64_t nowMs)
{
    return hasPending() && isIdle(nowMs) && flush(sink, "idle");
}

void CaptureSession::groupedEdit(CaptureSink& sink, const std::vector<GroupEdit>& edits, const std::string& kind,
                                 const std::string& groupID)
{
    if (edits.empty()) {
        return;
    }
    flush(sink, kind);
    for (const GroupEdit& e : edits) {
        seedIfNeeded(sink, e.sceneID, e.textBefore);
        const std::int64_t cursor = utf8Offset(e.textBefore, e.caretUtf16);
        const auto r = sink.record(e.sceneID, e.textAfter, kind, cursor, cursor, "", groupID);
        noteEngineState(r.canUndo, r.canRedo);
    }
    lastCommittedText_ = edits.front().textAfter;   // the caret ends in the FIRST scene of the edit
    lastActivityMs_.reset();
}

std::string CaptureSession::beforeBarrier(CaptureSink& sink, const std::string& sceneID)
{
    // The pending scene is the best attribution when the caller names none (I-0102) — taken BEFORE the flush clears it.
    const std::string attributed = !sceneID.empty() ? sceneID : pendingSceneID_.value_or("");
    flush(sink, "flush");
    return attributed;
}

void CaptureSession::noteApplied(const std::string& sceneID, const std::string& text)
{
    lastCommittedText_ = text;
    caretScene_ = sceneID;
}

}   // namespace scrivi::history
