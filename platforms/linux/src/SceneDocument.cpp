#include "SceneDocument.hpp"

#include <QPlainTextDocumentLayout>
#include <QTextCursor>

SceneDocument::SceneDocument()
{
    // QPlainTextEdit requires its document to use QPlainTextDocumentLayout. A bare
    // QTextDocument defaults to the rich-text layout, which QPlainTextEdit refuses
    // in setDocument() (leaving the viewport blank). Install the plain layout up
    // front; the document takes ownership.
    doc_.setDocumentLayout(new QPlainTextDocumentLayout(&doc_));
}

void SceneDocument::build(const QList<Input>& inputs)
{
    segments_.clear();
    doc_.clear();

    QTextCursor cursor(&doc_);
    cursor.beginEditBlock();

    QString lastChapterID;
    int chapterOrdinal = 0;   // 1-based position of the current chapter (for "Chapter N")
    for (int i = 0; i < inputs.size(); ++i) {
        const Input& in = inputs.at(i);

        // Chapter heading at every chapter boundary (first scene of a chapter),
        // mirroring the Apple manuscript view. Presentation only — the offset map
        // below, not this text, is the scene-ownership authority.
        const bool chapterBoundary = (in.chapterID != lastChapterID);
        if (chapterBoundary) {
            ++chapterOrdinal;
            if (i > 0) {
                // Blank line between the previous scene body and the new heading.
                cursor.insertText(QStringLiteral("\n\n"));
            }
            // Custom title wins; otherwise the derived ordinal "Chapter N" (order is the
            // authority, so untitled chapters renumber implicitly on delete/insert —
            // macOS ManuscriptTextView parity). Matches chapterHeadingText().
            const QString trimmed = in.chapterTitle.trimmed();
            const QString heading = trimmed.isEmpty()
                                        ? QStringLiteral("Chapter %1").arg(chapterOrdinal)
                                        : trimmed;
            cursor.insertText(heading + QStringLiteral("\n\n"));
            lastChapterID = in.chapterID;
        } else if (i > 0) {
            // Scene separator within a chapter: a blank line.
            cursor.insertText(QStringLiteral("\n\n"));
        }

        SceneSegment seg;
        seg.sceneID             = in.sceneID;
        seg.chapterID           = in.chapterID;
        seg.title               = in.title;
        seg.chapterTitle        = in.chapterTitle;
        seg.slug                = in.slug;
        seg.metadataPath        = in.metadataPath;
        seg.contentPath         = in.contentPath;
        seg.chapterMetadataPath = in.chapterMetadataPath;
        seg.bodyStart           = cursor.position();

        cursor.insertText(in.markdown);

        seg.bodyLength = cursor.position() - seg.bodyStart;
        segments_.append(seg);
    }

    cursor.endEditBlock();
}

int SceneDocument::bodyStartForScene(const QString& sceneID) const
{
    for (const SceneSegment& seg : segments_) {
        if (seg.sceneID == sceneID) {
            return seg.bodyStart;
        }
    }
    return -1;
}

int SceneDocument::sceneIndexForScene(const QString& sceneID) const
{
    for (int i = 0; i < segments_.size(); ++i) {
        if (segments_.at(i).sceneID == sceneID) {
            return i;
        }
    }
    return -1;
}

int SceneDocument::sceneIndexForEditablePosition(int pos) const
{
    // Inclusive body range [bodyStart, bodyStart+bodyLength] so a caret at either
    // edge of a body extends that body rather than leaking into the adjacent gap.
    for (int i = 0; i < segments_.size(); ++i) {
        const SceneSegment& seg = segments_.at(i);
        if (pos >= seg.bodyStart && pos <= seg.bodyStart + seg.bodyLength) {
            return i;
        }
    }
    return -1;
}

int SceneDocument::sceneIndexForCaret(int pos) const
{
    // Last segment whose body starts at or before pos: a caret parked in a gap
    // belongs to the scene it trails.
    int found = -1;
    for (int i = 0; i < segments_.size(); ++i) {
        if (segments_.at(i).bodyStart <= pos) {
            found = i;
        } else {
            break;
        }
    }
    return found;
}

bool SceneDocument::isEditablePosition(int pos) const
{
    return sceneIndexForEditablePosition(pos) >= 0;
}

int SceneDocument::nearestEditablePosition(int pos) const
{
    if (segments_.isEmpty() || isEditablePosition(pos)) {
        return pos;
    }
    // pos is in a protected gap. Snap to whichever editable edge is closer: the end
    // of the preceding body (last segment starting at/before pos) or the start of
    // the following body.
    int prevEnd = -1;   // end of the nearest body at/before pos
    int nextStart = -1; // start of the nearest body after pos
    for (const SceneSegment& seg : segments_) {
        const int bodyEnd = seg.bodyStart + seg.bodyLength;
        if (bodyEnd <= pos) {
            prevEnd = bodyEnd;                 // keeps advancing to the closest below
        } else if (seg.bodyStart >= pos && nextStart < 0) {
            nextStart = seg.bodyStart;         // first body starting at/after pos
        }
    }
    if (prevEnd < 0) {
        return nextStart;   // before the first body → first body start
    }
    if (nextStart < 0) {
        return prevEnd;     // after the last body → last body end
    }
    return (pos - prevEnd <= nextStart - pos) ? prevEnd : nextStart;
}

bool SceneDocument::isEditableRange(int start, int end) const
{
    if (start > end) {
        std::swap(start, end);
    }
    if (start == end) {
        return isEditablePosition(start);
    }
    // The whole selection must sit inside ONE body's editable span; otherwise a
    // replace/delete could straddle (and destroy) a boundary.
    const int i = sceneIndexForEditablePosition(start);
    if (i < 0) {
        return false;
    }
    const SceneSegment& seg = segments_.at(i);
    return end <= seg.bodyStart + seg.bodyLength;
}

int SceneDocument::applyContentsChange(int pos, int charsRemoved, int charsAdded)
{
    const int delta = charsAdded - charsRemoved;
    if (delta == 0) {
        // No length change (e.g. a formatting-only change); ownership unaffected.
        return sceneIndexForCaret(pos);
    }

    // The edit is anchored at pos. For a removal the affected scene is the one
    // owning pos; for an insertion likewise. Resolve via the caret query so a pos
    // exactly at a body's trailing edge still maps to that body.
    int i = sceneIndexForEditablePosition(pos);
    if (i < 0) {
        i = sceneIndexForCaret(pos);
    }
    if (i < 0) {
        return -1;
    }

    segments_[i].bodyLength += delta;
    if (segments_[i].bodyLength < 0) {
        segments_[i].bodyLength = 0;   // defensive; edits are gated to stay in-body
    }
    for (int j = i + 1; j < segments_.size(); ++j) {
        segments_[j].bodyStart += delta;
    }
    return i;
}

int SceneDocument::insertSceneAfter(int afterIndex,
                                    const QString& sceneID,
                                    const QString& chapterID,
                                    const QString& title,
                                    const QString& chapterTitle,
                                    const QString& slug,
                                    const QString& metadataPath,
                                    const QString& contentPath,
                                    const QString& chapterMetadataPath,
                                    bool newChapter)
{
    if (afterIndex < 0 || afterIndex >= segments_.size()) {
        return -1;
    }

    const SceneSegment& prev = segments_.at(afterIndex);
    const int insertAt = prev.bodyStart + prev.bodyLength;

    // Boundary text mirrors build(): a blank line separates scenes within a chapter; a
    // chapter heading (title + blank line) precedes a new chapter's first scene. For a
    // new chapter we insert a PROVISIONAL heading here, then reflowBoundaryAt() rewrites
    // it once the segment is in the list — so the derived ordinal "Chapter N" (which
    // needs the new segment's position) is correct. Both begin with a blank line off the
    // previous body.
    QString boundary = QStringLiteral("\n\n");
    if (newChapter) {
        const QString provisional =
            chapterTitle.trimmed().isEmpty() ? QStringLiteral("Chapter")
                                             : chapterTitle.trimmed();
        boundary += provisional + QStringLiteral("\n\n");
    }

    QTextCursor cursor(&doc_);
    cursor.beginEditBlock();
    cursor.setPosition(insertAt);
    cursor.insertText(boundary);   // empty body follows — nothing more to insert
    cursor.endEditBlock();

    const int inserted = boundary.length();
    const int newBodyStart = insertAt + inserted;

    SceneSegment seg;
    seg.sceneID             = sceneID;
    seg.chapterID           = chapterID;
    seg.title               = title;
    seg.chapterTitle        = chapterTitle;
    seg.slug                = slug;
    seg.metadataPath        = metadataPath;
    seg.contentPath         = contentPath;
    seg.chapterMetadataPath = chapterMetadataPath;
    seg.bodyStart           = newBodyStart;
    seg.bodyLength          = 0;   // brand-new empty scene

    // Every segment after the insertion point shifts by the inserted length.
    for (int j = afterIndex + 1; j < segments_.size(); ++j) {
        segments_[j].bodyStart += inserted;
    }
    segments_.insert(afterIndex + 1, seg);

    // Now that the new segment is positioned, rewrite its heading to the derived
    // ordinal (only meaningful for a new untitled chapter; a no-op otherwise).
    if (newChapter) {
        reflowBoundaryAt(afterIndex + 1);
    }
    return afterIndex + 1;
}

namespace {
// The leading-boundary text that build()/insertSceneAfter place BEFORE a segment's
// body, given its position in the manuscript. `first` = this is the first segment in
// the document; `chapterStart` = this segment begins a new chapter (heading needed);
// `headingText` = the chapter title (or "Chapter" when empty). Kept in one place so
// removeScene/reflow reproduce build() exactly.
QString leadingBoundaryFor(bool first, bool chapterStart, const QString& headingText)
{
    if (first) {
        // The very first body in the document has no leading gap (build() emits none
        // before i==0), regardless of chapter — its heading, if any, is emitted with
        // no preceding blank line. Match that: heading + "\n\n", or nothing.
        return chapterStart ? headingText + QStringLiteral("\n\n") : QString();
    }
    if (chapterStart) {
        return QStringLiteral("\n\n") + headingText + QStringLiteral("\n\n");
    }
    return QStringLiteral("\n\n");   // in-chapter scene separator
}
} // namespace

void SceneDocument::reflowBoundaryAt(int index)
{
    if (index < 0 || index >= segments_.size()) {
        return;
    }
    // The document span occupied by segment `index`'s CURRENT leading boundary runs
    // from the previous body's end (or doc start for index 0) up to this bodyStart.
    const int spanStart =
        (index == 0) ? 0 : segments_.at(index - 1).bodyStart
                               + segments_.at(index - 1).bodyLength;
    const int spanEnd = segments_.at(index).bodyStart;

    const bool first = (index == 0);
    const bool chapterStart =
        first || segments_.at(index).chapterID != segments_.at(index - 1).chapterID;
    // Derived heading (custom title, else the ordinal "Chapter N") — the same authority
    // the navigator label uses, so an untitled chapter renumbers implicitly here too.
    const QString heading = chapterHeadingText(index);
    const QString desired = leadingBoundaryFor(first, chapterStart, heading);

    const int oldLen = spanEnd - spanStart;
    if (oldLen == desired.length()) {
        // Same length: still must rewrite in case the heading text changed, but no
        // offsets shift. Only rewrite when the text actually differs.
        QTextCursor probe(&doc_);
        probe.setPosition(spanStart);
        probe.setPosition(spanEnd, QTextCursor::KeepAnchor);
        if (probe.selectedText().replace(QChar(0x2029), QLatin1Char('\n')) == desired) {
            return;   // already correct
        }
    }

    QTextCursor cursor(&doc_);
    cursor.beginEditBlock();
    cursor.setPosition(spanStart);
    cursor.setPosition(spanEnd, QTextCursor::KeepAnchor);
    cursor.insertText(desired);   // replaces the selection
    cursor.endEditBlock();

    const int delta = desired.length() - oldLen;
    if (delta != 0) {
        segments_[index].bodyStart += delta;
        for (int j = index + 1; j < segments_.size(); ++j) {
            segments_[j].bodyStart += delta;
        }
    }
}

bool SceneDocument::removeScene(int index)
{
    if (index < 0 || index >= segments_.size()) {
        return false;
    }

    const SceneSegment target = segments_.at(index);   // copy (list mutates)

    // Compute the document span to delete. Default: from the previous body's end
    // through this body's end — i.e. this scene's leading boundary + its body. That
    // leaves the following scene's own leading boundary intact.
    int spanStart = 0;
    int spanEnd = target.bodyStart + target.bodyLength;
    if (index > 0) {
        const SceneSegment& prev = segments_.at(index - 1);
        spanStart = prev.bodyStart + prev.bodyLength;
    } else if (segments_.size() > 1) {
        // Removing the FIRST segment: take this body + its TRAILING boundary (up to
        // the next body's start) so the new first scene has no leading gap.
        spanStart = 0;
        spanEnd = segments_.at(1).bodyStart;
    } else {
        // Only segment: clear the whole document.
        spanStart = 0;
        spanEnd = doc_.characterCount() - 1;   // characterCount includes the final \n
    }

    QTextCursor cursor(&doc_);
    cursor.beginEditBlock();
    cursor.setPosition(spanStart);
    cursor.setPosition(spanEnd, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.endEditBlock();

    const int removed = spanEnd - spanStart;
    for (int j = index + 1; j < segments_.size(); ++j) {
        segments_[j].bodyStart -= removed;
    }
    segments_.removeAt(index);

    // The segment that shifted into slot `index` (if any) may now need a different
    // leading boundary than it had: removing the first scene strips the new-first's
    // leading gap (it must have none, or a bare heading); removing a chapter's first
    // scene promotes an in-chapter follower to chapter-first (its separator must
    // become a heading). reflowBoundaryAt recomputes the correct boundary from the
    // follower's new position and is a no-op when it's already right (the common
    // in-chapter mid-delete). The position rule alone is sufficient — no need to know
    // what the removed scene was.
    if (index < segments_.size()) {
        reflowBoundaryAt(index);
    }
    return true;
}

int SceneDocument::removeChapter(const QString& chapterID)
{
    // Collect member indices, then remove last-to-first so earlier offsets stay valid
    // and no "promote follower" reflow fires mid-chapter (the whole chapter goes).
    QList<int> members;
    for (int i = 0; i < segments_.size(); ++i) {
        if (segments_.at(i).chapterID == chapterID) {
            members.append(i);
        }
    }
    for (int k = members.size() - 1; k >= 0; --k) {
        removeScene(members.at(k));
    }
    return members.size();
}

int SceneDocument::moveScene(int fromIndex,
                             const QString& targetChapterID,
                             const QString& targetChapterTitle,
                             const QString& afterSceneID)
{
    if (fromIndex < 0 || fromIndex >= segments_.size()) {
        return -1;
    }

    // Capture the moved scene's identity + live body BEFORE we disturb the document,
    // so the re-insert reproduces it exactly at the destination.
    SceneSegment moved = segments_.at(fromIndex);   // copy
    const QString movedBody = bodyText(fromIndex);
    moved.chapterID    = targetChapterID;
    moved.chapterTitle = targetChapterTitle;

    // Resolve the destination as an INSERT index in the post-removal list. Compute it
    // from stable sceneIDs (not raw indices) so it survives the removal's re-index.
    // afterSceneID empty  → the moved scene becomes the target chapter's first scene.
    // afterSceneID present → it lands immediately after that sibling.
    // A move that would drop the scene exactly back where it started is a no-op.
    if (!afterSceneID.isEmpty() && afterSceneID == moved.sceneID) {
        return -1;   // can't land after itself
    }

    // Lift the source out. removeScene handles the old chapter's follower promotion
    // (a follower inheriting a vacated chapter-first slot regains its heading) and
    // fixes every later bodyStart. After this the list has one fewer segment.
    if (!removeScene(fromIndex)) {
        return -1;
    }

    // Where does it go now?
    int insertIndex = -1;
    if (afterSceneID.isEmpty()) {
        // First scene of the target chapter: insert before that chapter's current
        // first segment. If the target chapter has no segments left (it was the moved
        // scene's own chapter and this was its last member), fall back to the front.
        const int firstOfTarget = firstSegmentOfChapter(targetChapterID);
        insertIndex = (firstOfTarget >= 0) ? firstOfTarget : 0;
    } else {
        const int afterIdx = sceneIndexForScene(afterSceneID);
        if (afterIdx < 0) {
            return -1;   // unknown sibling — refuse rather than guess a position
        }
        insertIndex = afterIdx + 1;
    }
    insertIndex = qBound(0, insertIndex, segments_.size());

    // Insert the moved body with a PROVISIONAL leading boundary, then let reflow fix
    // it (and the follower's) from position. We first place a bare separator; if the
    // scene is the very first body reflow will strip the gap, and if it begins a
    // chapter reflow will turn it into a heading — so the exact provisional text only
    // needs to be non-empty enough to carve a boundary region. Match build()/insert:
    // "\n\n" between bodies (index 0 gets its gap stripped by reflowBoundaryAt).
    const int prevEnd =
        (insertIndex == 0) ? 0
                           : segments_.at(insertIndex - 1).bodyStart
                                 + segments_.at(insertIndex - 1).bodyLength;

    const QString provisional =
        (insertIndex == 0) ? QString() : QStringLiteral("\n\n");
    const QString inserted = provisional + movedBody;

    QTextCursor cursor(&doc_);
    cursor.beginEditBlock();
    cursor.setPosition(prevEnd);
    cursor.insertText(inserted);
    cursor.endEditBlock();

    moved.bodyStart  = prevEnd + provisional.length();
    moved.bodyLength = movedBody.length();

    // Shift every segment at/after the insert point by the inserted length, then splice
    // the moved segment in.
    for (int j = insertIndex; j < segments_.size(); ++j) {
        segments_[j].bodyStart += inserted.length();
    }
    segments_.insert(insertIndex, moved);

    // Now fix the leading boundary of the moved scene (heading if it begins a chapter,
    // separator otherwise; gap stripped if it's first) and of the segment that now
    // follows it (its predecessor changed). reflowBoundaryAt is offset-correcting and a
    // no-op when the boundary is already right.
    reflowBoundaryAt(insertIndex);
    if (insertIndex + 1 < segments_.size()) {
        reflowBoundaryAt(insertIndex + 1);
    }

    // Renumber every untitled chapter so ordinals stay correct after the move.
    reflowAllChapterHeadings();

    return sceneIndexForScene(moved.sceneID);
}

void SceneDocument::setSceneTitle(int index, const QString& title)
{
    if (index < 0 || index >= segments_.size()) {
        return;
    }
    // A scene's title is a navigator label only — no in-document heading — so nothing
    // in the document text or the offset map changes.
    segments_[index].title = title;
}

int SceneDocument::firstSegmentOfChapter(const QString& chapterID) const
{
    for (int i = 0; i < segments_.size(); ++i) {
        if (segments_.at(i).chapterID == chapterID) {
            return i;
        }
    }
    return -1;
}

void SceneDocument::reflowAllChapterHeadings()
{
    // Reflow each chapter's first segment. Walk by chapter boundary; reflowBoundaryAt
    // may shift later offsets, but we re-derive the boundary from live segment data each
    // time and iterate forward, so subsequent first-segments are found correctly.
    QString lastChapterID;
    for (int i = 0; i < segments_.size(); ++i) {
        if (segments_.at(i).chapterID != lastChapterID) {
            lastChapterID = segments_.at(i).chapterID;
            reflowBoundaryAt(i);
        }
    }
}

QString SceneDocument::chapterHeadingText(int index) const
{
    if (index < 0 || index >= segments_.size()) {
        return {};
    }
    // A non-empty custom title wins.
    const QString custom = segments_.at(index).chapterTitle.trimmed();
    if (!custom.isEmpty()) {
        return custom;
    }
    // Otherwise derive "Chapter N" from the chapter's 1-based position among the ordered
    // distinct chapters (macOS ManuscriptTextView parity). Order is authority, so a
    // delete/insert renumbers untitled chapters implicitly — no stored ordinal to fix.
    const QString chapterID = segments_.at(index).chapterID;
    int ordinal = 0;
    QString lastSeen;
    for (const SceneSegment& seg : segments_) {
        if (seg.chapterID != lastSeen) {   // new distinct chapter in manuscript order
            ++ordinal;
            lastSeen = seg.chapterID;
            if (seg.chapterID == chapterID) {
                break;
            }
        }
    }
    return QStringLiteral("Chapter %1").arg(ordinal);
}

void SceneDocument::setChapterTitle(const QString& chapterID,
                                    const QString& chapterTitle)
{
    const int first = firstSegmentOfChapter(chapterID);
    if (first < 0) {
        return;   // unknown chapter
    }
    // Every member scene carries the chapterTitle (navigator groups by it); update all.
    for (int i = 0; i < segments_.size(); ++i) {
        if (segments_.at(i).chapterID == chapterID) {
            segments_[i].chapterTitle = chapterTitle;
        }
    }
    // Rewrite the live in-document heading in place (reflow reproduces build()'s
    // heading text from the now-updated chapterTitle and fixes later offsets).
    reflowBoundaryAt(first);
}

QString SceneDocument::bodyText(int index) const
{
    if (index < 0 || index >= segments_.size()) {
        return {};
    }
    const SceneSegment& seg = segments_.at(index);
    QTextCursor cursor(const_cast<QTextDocument*>(&doc_));
    cursor.setPosition(seg.bodyStart);
    cursor.setPosition(seg.bodyStart + seg.bodyLength, QTextCursor::KeepAnchor);
    // QTextCursor::selectedText() encodes paragraph breaks as U+2029; restore real
    // newlines so the saved .md matches what the author typed.
    return cursor.selectedText().replace(QChar(0x2029), QLatin1Char('\n'));
}
