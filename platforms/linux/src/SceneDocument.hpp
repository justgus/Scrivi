#pragma once

#include <QList>
#include <QString>
#include <QTextDocument>

// SceneDocument — assembles every scene body of an open project into ONE
// continuous QTextDocument, and keeps the side offset map that is the authority
// for "which scene owns this character position" (the sceneStartMap /
// sceneStorageOffsetMap seed from Apple's ManuscriptTextView).
//
// SP-061 (EP-022, T-0235) uses this read-only: build once from the ordered
// scenes[] + activeScene payload of scrivi_open_project, plus a scrivi_open_scene
// loop for the remaining bodies. SP-062 makes the same document editable — so the
// OFFSET MAP, not the rendered boundary text, is the source of truth. The chapter
// heading + blank separator lines are presentation; `segments` records each
// scene's real start position and body length so edits can still be routed to the
// owning scene once the viewport is editable.
//
// No ScriviCore logic lives here — the caller (EditorShell) fetches bodies via
// ScriviBridge and hands them in.
struct SceneSegment
{
    QString sceneID;
    QString chapterID;
    QString title;
    QString chapterTitle;
    QString slug;
    QString metadataPath;
    QString contentPath;

    // Character offset (into the assembled document) where this scene's BODY
    // text begins — after any chapter heading + separator lines. This is the
    // sceneStartMap value: navigator click scrolls here, and SP-062 maps a caret
    // position back to the owning scene through these starts + lengths.
    int bodyStart = 0;
    // Length in characters of this scene's body as inserted.
    int bodyLength = 0;
};

class SceneDocument
{
public:
    // Installs a QPlainTextDocumentLayout on the owned document. QPlainTextEdit
    // rejects any document that lacks one ("setDocument: Document set does not
    // support QPlainTextDocumentLayout"), so this must be set before the document
    // is handed to the viewport.
    SceneDocument();

    // A scene body paired with the ordered scenes[] entry it belongs to. The
    // caller assembles these in manuscript order (openProject scene order).
    struct Input
    {
        QString sceneID;
        QString chapterID;
        QString title;
        QString chapterTitle;
        QString slug;
        QString metadataPath;
        QString contentPath;
        QString markdown;   // the scene body (may be empty)
    };

    // Build a fresh continuous document from ordered inputs. A chapter heading is
    // emitted at each chapter boundary (first scene of a chapter); scenes within a
    // chapter are separated by a blank line. Populates segments() with the real
    // body start/length for each scene. Read-only presentation for SP-061; the
    // offsets survive into the editable SP-062 viewport.
    void build(const QList<Input>& inputs);

    // The assembled document (owned here; hand document() to the QPlainTextEdit).
    QTextDocument* document() { return &doc_; }

    // Ordered per-scene segments with their body offsets (the sceneStartMap).
    const QList<SceneSegment>& segments() const { return segments_; }

    // Body-start offset for a sceneID, or -1 if unknown. Used by the navigator to
    // scroll the viewport to a scene.
    int bodyStartForScene(const QString& sceneID) const;

    // Segment index for a sceneID, or -1 if unknown. Used to promote a
    // navigator-clicked scene to active (T-0244).
    int sceneIndexForScene(const QString& sceneID) const;

    // --- SP-062 editable-viewport support (T-0238) ------------------------
    //
    // The offset map is the edit-routing authority. These queries + the mutator
    // below let EditorShell (a) decide whether a position is inside an editable
    // body vs. a protected boundary (chapter heading / separator), and (b) keep the
    // map in sync with edits WITHOUT rebuilding the document per keystroke.

    // Index of the segment whose body contains `pos`, treating the body as the
    // inclusive range [bodyStart, bodyStart+bodyLength] so edits at either edge
    // extend that body (not the neighbouring gap). Returns -1 if `pos` falls in a
    // boundary region (before the first body, or in a heading/separator gap).
    int sceneIndexForEditablePosition(int pos) const;

    // Index of the segment whose body "owns" `pos` for caret/active-scene purposes:
    // the last segment whose bodyStart <= pos. Unlike the editable query this never
    // returns -1 for an in-document position past the first body — a caret sitting
    // in a gap belongs to the preceding scene. Returns -1 only if `pos` precedes
    // the first body. Used to map the caret to the active scene.
    int sceneIndexForCaret(int pos) const;

    // True if `pos` is a legal insertion/deletion point — inside some body's
    // inclusive editable range. Boundary regions return false (protected).
    bool isEditablePosition(int pos) const;

    // Snap `pos` to the nearest editable body position (T-0246): if `pos` is already
    // editable it's returned unchanged; if it's in a heading/separator gap it snaps
    // to whichever is closer — the end of the preceding body or the start of the
    // following body. So the caret never rests inside protected boundary text.
    // Returns `pos` unchanged if there are no segments.
    int nearestEditablePosition(int pos) const;

    // True if the half-open range [start, end) lies entirely within one body's
    // editable span (so a selection replace/delete stays inside one scene and never
    // eats a boundary). Empty ranges defer to isEditablePosition(start).
    bool isEditableRange(int start, int end) const;

    // Apply a document contentsChange to the map: the body containing `pos` grows or
    // shrinks by (charsAdded - charsRemoved), and every later body's bodyStart
    // shifts by the same delta. Returns the index of the affected (now-dirty) scene,
    // or -1 if `pos` did not resolve to a body (should not happen once edits are
    // gated by isEditablePosition). Does NOT touch document text — the document
    // already changed; this only reconciles the side map.
    int applyContentsChange(int pos, int charsRemoved, int charsAdded);

    // Current body text of segment `index` (extracted live from the document via its
    // recorded offsets). Empty string if index is out of range. Used by the per-scene
    // save path (T-0239).
    QString bodyText(int index) const;

    // --- SP-062 in-editor structure creation (T-0240 / T-0241) ------------
    //
    // Surgically splice a freshly-created scene into the live document + map after
    // segment `afterIndex`, WITHOUT rebuilding. Inserts the appropriate boundary
    // text (a blank scene-separator when the new scene shares afterIndex's chapter,
    // or a chapter heading when `chapterTitle` names a new chapter) followed by an
    // empty body, records a new SceneSegment for it, and shifts every later segment's
    // bodyStart by the inserted length. Returns the index of the new segment (so the
    // caller can drop the caret into its empty body and refresh the navigator), or
    // -1 on a bad afterIndex.
    //
    // `newChapter` distinguishes T-0240 (⌘↩, same chapter → separator) from T-0241
    // (⌘⇧↩, new chapter → heading). The scene identity fields come from the
    // create_scene / create_chapter result. The document edits here run inside a
    // caller-managed "programmatic" window so they don't churn dirty flags.
    int insertSceneAfter(int afterIndex,
                         const QString& sceneID,
                         const QString& chapterID,
                         const QString& title,
                         const QString& chapterTitle,
                         const QString& slug,
                         const QString& metadataPath,
                         const QString& contentPath,
                         bool newChapter);

private:
    QTextDocument doc_;
    QList<SceneSegment> segments_;
};
