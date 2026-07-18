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
    // The CHAPTER's own sidecar metadata path (open_project's `chapterMetadataPath`).
    // Needed to rename the chapter (EP-023 / SP-066, T-0254). Every scene segment in a
    // chapter carries the same value.
    QString chapterMetadataPath;

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
        QString chapterMetadataPath;   // chapter sidecar path (for rename, T-0254)
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

    // Directional snap for KEYBOARD navigation across a boundary gap. If `pos` is already
    // editable it's returned unchanged. Otherwise, when `movingForward` is true the caret
    // snaps to the START of the FOLLOWING body (so Down/Right steps into the next scene);
    // when false it snaps to the END of the PRECEDING body (so Up/Left steps into the prior
    // scene). This is what lets the arrows cross a chapter heading / scene separator instead
    // of getting stuck at the boundary (the "nearest" rule always snapped back the way you
    // came). Falls back to the available edge when there is no body in the chosen direction.
    int editablePositionInDirection(int pos, bool movingForward) const;

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
                         const QString& chapterMetadataPath,
                         bool newChapter);

    // --- EP-023 in-navigator structure editing (T-0252) -------------------
    //
    // Surgically remove segment `index` from the live document + map, WITHOUT
    // rebuilding. Deletes the scene's body plus exactly one adjoining boundary
    // (heading/separator) so the remaining scenes stay well-formed, then shifts every
    // later segment's bodyStart back by the removed length and drops the segment.
    //
    // Boundary rule (reverse of build()): a scene is joined to the manuscript by the
    // text between the previous body's end and its own body's end. Removing that span
    // takes the leading separator/heading with it. Special cases:
    //   • Removing the FIRST segment instead removes from doc start through the start
    //     of the next body (the trailing boundary), so the new first scene needs no
    //     leading gap. If it's also the only segment, the whole document is cleared.
    //   • If the removed scene begins a chapter and a following scene inherits that
    //     chapter, the follower is re-promoted to the chapter's first scene: its
    //     leading separator becomes a heading. The caller passes the surviving
    //     chapter's title so the heading text is right; the map/segments are updated
    //     and the affected span rewritten. (For a whole-chapter delete the caller uses
    //     removeChapter, which removes every member scene, so this case only arises on
    //     a single-scene delete that happened to be a chapter's first.)
    //
    // Returns true if a segment was removed. The document edits run inside a
    // caller-managed "programmatic" window so they don't churn dirty flags.
    bool removeScene(int index);

    // Remove every segment belonging to `chapterID` (the chapter + all its scenes),
    // reusing removeScene per member from last to first so offsets stay valid. Returns
    // the number of scenes removed (0 if the chapter is unknown).
    int removeChapter(const QString& chapterID);

    // --- EP-023 scene reorder (T-0259 / SP-067) ---------------------------
    //
    // Move segment `fromIndex` within its chapter or across a chapter boundary,
    // surgically re-splicing the live document + offset map WITHOUT a rebuild (the
    // inverse peer of removeScene + insertSceneAfter, but preserving the moved body).
    //
    // The destination mirrors the scrivi_reorder_scene contract: `targetChapterID` is
    // the chapter the scene joins (== its current chapter for a within-chapter move),
    // and `afterSceneID` names the sibling it lands right after — empty means it
    // becomes the target chapter's FIRST scene. The moved scene's `chapterID`/
    // `chapterTitle` are updated to the target chapter's; its body text and identity
    // (sceneID, metadataPath, contentPath, slug, title) are preserved.
    //
    // Boundary handling (reuses build()'s leading-boundary rule via reflow):
    //   • Lifting the scene out demotes/promotes its old-chapter follower exactly as
    //     removeScene does (a follower that inherits a vacated chapter-first slot gains
    //     its heading; a mid-chapter follower is untouched).
    //   • Re-inserting it gives it a heading when it becomes the target chapter's first
    //     scene, or a plain separator otherwise.
    // Then reflowAllChapterHeadings() renumbers every untitled chapter so ordinals stay
    // correct after the move. Runs inside a caller-managed "programmatic" window (no
    // dirty churn). Returns the moved scene's NEW segment index, or -1 on a bad move
    // (unknown fromIndex, unknown afterSceneID, or a no-op that would place the scene
    // exactly where it already is).
    int moveScene(int fromIndex,
                  const QString& targetChapterID,
                  const QString& targetChapterTitle,
                  const QString& afterSceneID);

    // --- EP-023 rename (T-0255) -------------------------------------------
    //
    // Update segment `index`'s scene title in the map (the navigator label authority).
    // Scenes have no in-document heading, so this touches no document text. Out-of-range
    // index is a no-op.
    void setSceneTitle(int index, const QString& title);

    // Update the `chapterTitle` on EVERY segment in `chapterID`, then rewrite that
    // chapter's live in-document heading in place via reflowBoundaryAt (its first
    // segment). Reused by both chapter rename (T-0255) and the I-0062 live new-chapter
    // heading fix (T-0256). No-op if the chapter is unknown. Runs inside a
    // caller-managed "programmatic" window so it doesn't churn dirty flags.
    void setChapterTitle(const QString& chapterID, const QString& chapterTitle);

    // Index of the first segment of `chapterID`, or -1 if unknown. Used to target the
    // heading for a chapter-level update.
    int firstSegmentOfChapter(const QString& chapterID) const;

    // Rewrite every chapter's in-document heading to its current derived text
    // (chapterHeadingText) via reflowBoundaryAt. Call after any structural change that
    // can shift chapter ordinals (delete/insert/reorder) so untitled chapters renumber
    // in the document, matching the navigator — with no disk rewrite. Runs inside a
    // caller-managed "programmatic" window (no dirty churn).
    void reflowAllChapterHeadings();

    // The DISPLAY heading for segment `index`'s chapter: the trimmed custom chapterTitle
    // if non-empty, else the derived ordinal "Chapter N" where N is the chapter's 1-based
    // position among the ordered distinct chapters (matching the macOS ManuscriptTextView
    // rule). Deriving N from ORDER — not a stored string — means deleting or inserting a
    // chapter renumbers every subsequent untitled chapter for free, with no disk rewrite.
    // The ordinal authority for both the in-document heading and the navigator label.
    // Empty string if index is out of range.
    QString chapterHeadingText(int index) const;

private:
    // Recompute the leading-boundary text (heading/separator) for segment `index`
    // from scratch — used when a delete promotes a following scene to a chapter's
    // first scene (its separator must become a heading) or demotes one. Rewrites the
    // document span before the body and fixes all later bodyStarts. Internal helper
    // for removeScene.
    void reflowBoundaryAt(int index);

    QTextDocument doc_;
    QList<SceneSegment> segments_;
};
