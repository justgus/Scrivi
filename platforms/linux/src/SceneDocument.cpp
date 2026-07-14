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
    for (int i = 0; i < inputs.size(); ++i) {
        const Input& in = inputs.at(i);

        // Chapter heading at every chapter boundary (first scene of a chapter),
        // mirroring the Apple manuscript view. Presentation only — the offset map
        // below, not this text, is the scene-ownership authority.
        const bool chapterBoundary = (in.chapterID != lastChapterID);
        if (chapterBoundary) {
            if (i > 0) {
                // Blank line between the previous scene body and the new heading.
                cursor.insertText(QStringLiteral("\n\n"));
            }
            const QString heading = in.chapterTitle.isEmpty()
                                        ? QStringLiteral("Chapter")
                                        : in.chapterTitle;
            cursor.insertText(heading + QStringLiteral("\n\n"));
            lastChapterID = in.chapterID;
        } else if (i > 0) {
            // Scene separator within a chapter: a blank line.
            cursor.insertText(QStringLiteral("\n\n"));
        }

        SceneSegment seg;
        seg.sceneID      = in.sceneID;
        seg.chapterID    = in.chapterID;
        seg.title        = in.title;
        seg.chapterTitle = in.chapterTitle;
        seg.slug         = in.slug;
        seg.metadataPath = in.metadataPath;
        seg.contentPath  = in.contentPath;
        seg.bodyStart    = cursor.position();

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
                                    bool newChapter)
{
    if (afterIndex < 0 || afterIndex >= segments_.size()) {
        return -1;
    }

    const SceneSegment& prev = segments_.at(afterIndex);
    const int insertAt = prev.bodyStart + prev.bodyLength;

    // Boundary text mirrors build(): a blank line separates scenes within a
    // chapter; a chapter heading (title + blank line) precedes a new chapter's
    // first scene. Both begin with a blank line off the previous body.
    QString boundary = QStringLiteral("\n\n");
    if (newChapter) {
        const QString heading = chapterTitle.isEmpty() ? QStringLiteral("Chapter")
                                                       : chapterTitle;
        boundary += heading + QStringLiteral("\n\n");
    }

    QTextCursor cursor(&doc_);
    cursor.beginEditBlock();
    cursor.setPosition(insertAt);
    cursor.insertText(boundary);   // empty body follows — nothing more to insert
    cursor.endEditBlock();

    const int inserted = boundary.length();
    const int newBodyStart = insertAt + inserted;

    SceneSegment seg;
    seg.sceneID      = sceneID;
    seg.chapterID    = chapterID;
    seg.title        = title;
    seg.chapterTitle = chapterTitle;
    seg.slug         = slug;
    seg.metadataPath = metadataPath;
    seg.contentPath  = contentPath;
    seg.bodyStart    = newBodyStart;
    seg.bodyLength   = 0;   // brand-new empty scene

    // Every segment after the insertion point shifts by the inserted length.
    for (int j = afterIndex + 1; j < segments_.size(); ++j) {
        segments_[j].bodyStart += inserted;
    }
    segments_.insert(afterIndex + 1, seg);
    return afterIndex + 1;
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
