#include "manuscript/FragmentPaster.hpp"

#include "manuscript/ChapterCreator.hpp"
#include "manuscript/ChapterDeleter.hpp"
#include "manuscript/ChapterRenamer.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "manuscript/SceneCreator.hpp"
#include "manuscript/SceneDeleter.hpp"
#include "manuscript/SceneReader.hpp"
#include "manuscript/SceneRenamer.hpp"
#include "manuscript/SceneWriter.hpp"
#include "util/PathUtils.hpp"

#include <unordered_map>

namespace scrivi::manuscript {

namespace {

Error invalidArg(std::string message)
{
    return Error{ErrorCode::invalidArgument, std::move(message), "", ""};
}

} // namespace

FragmentPaster::FragmentPaster(CoreServices& services)
    : services_(services) {}

Result<PasteFragmentResult> FragmentPaster::paste(const PasteFragmentRequest& request)
{
    using R = Result<PasteFragmentResult>;

    const Fragment& frag = request.fragment;
    if (frag.pieces.empty())
        return R::failure(invalidArg("empty fragment: nothing to paste"));

    const AbsolutePath& root = request.projectRootPath;

    // Locate the caret's scene (identity + paths + chapter) from disk order.
    ManuscriptOrderResolver resolver{services_};
    auto resolvedR = resolver.resolve(root);
    if (!resolvedR.ok())
        return R::failure(resolvedR.error());

    const ResolvedScene* targetPtr = nullptr;
    for (const auto& scene : resolvedR.value())
        if (scene.sceneID.value == request.caretSceneID.value) { targetPtr = &scene; break; }
    if (targetPtr == nullptr)
        return R::failure(invalidArg("caret scene not in manuscript: " + request.caretSceneID.value));
    const ResolvedScene target = *targetPtr;

    // Read the target body and split it at the caret.
    SceneReader reader{services_};
    auto bodyR = reader.readContent(root, target.contentPath);
    if (!bodyR.ok())
        return R::failure(bodyR.error());
    const std::string& body = bodyR.value();
    if (request.caretByteOffset > body.size())
        return R::failure(invalidArg("caret byte offset past end of scene body"));
    const std::string head = body.substr(0, request.caretByteOffset);
    const std::string tail = body.substr(request.caretByteOffset);

    SceneCreator   sceneCreator{services_};
    ChapterCreator chapterCreator{services_};
    SceneWriter    writer{services_};
    SceneRenamer   sceneRenamer{services_};
    ChapterRenamer chapterRenamer{services_};

    // Apply a captured title to a freshly-created scene (T-0357). Empty title = leave as created
    // (a scene born untitled stays untitled). metaPath is the scene.meta.json relative path.
    auto applySceneTitle = [&](const RelativePath& metaPath, const std::string& title) -> Result<void> {
        if (title.empty()) return Result<void>::success();
        RenameSceneRequest rn;
        rn.projectRootPath = root;
        rn.metadataPath    = metaPath;
        rn.newTitle        = title;
        auto r = sceneRenamer.rename(rn);
        if (!r.ok()) return Result<void>::failure(r.error());
        return Result<void>::success();
    };
    // Apply a captured chapter title to a freshly-created chapter (T-0357).
    auto applyChapterTitle = [&](const RelativePath& metaPath, const std::string& title) -> Result<void> {
        if (title.empty()) return Result<void>::success();
        RenameChapterRequest rn;
        rn.projectRootPath = root;
        rn.metadataPath    = metaPath;
        rn.newTitle        = title;
        auto r = chapterRenamer.rename(rn);
        if (!r.ok()) return Result<void>::failure(r.error());
        return Result<void>::success();
    };

    // Small helper: persist `text` into a scene given its identity + paths.
    auto saveBody = [&](const SceneID& id, const RelativePath& metaPath,
                        const RelativePath& contentPath, const std::string& text) -> Result<void> {
        SaveSceneRequest sv;
        sv.projectID         = request.projectID;
        sv.projectRootPath   = root;
        sv.appSupportRoot    = request.appSupportRoot;
        sv.sceneID           = id;
        sv.sceneMetadataPath = metaPath;
        sv.sceneContentPath  = contentPath;
        sv.markdown          = text;
        sv.author            = request.author;
        auto r = writer.save(sv);
        if (!r.ok()) return Result<void>::failure(r.error());
        return Result<void>::success();
    };

    PasteFragmentResult result;
    result.targetSceneID = target.sceneID;

    // The "running insertion point": the scene the NEXT piece appends after / the scene whose
    // body the trailing tail-suffix is appended to. Starts at the target scene.
    SceneID      runSceneID       = target.sceneID;
    RelativePath runSceneMetaPath = target.metadataPath;
    RelativePath runSceneContent  = target.contentPath;
    std::string  runSceneBody;              // the body we will write to runScene at the end
    ChapterID    runChapterID     = target.chapterID;

    // Piece 0 (opensWith None) is the leading/head partial: its text extends the head, and the
    // target scene's new body becomes head + piece0.text (the tail is dealt with at the end).
    // The join is DIRECT concatenation, not a blank-line seam: the caret sits mid-text, so a
    // flat-document insert continues the paragraph (user-ruled 2026-07-27). A blank-line seam
    // is never used in paste — each cross-boundary piece becomes its own scene, so the scene
    // split IS the boundary.
    runSceneBody = head + frag.pieces.front().text;

    // Subsequent pieces each create a scene or a chapter after the running insertion point.
    for (std::size_t i = 1; i < frag.pieces.size(); ++i) {
        const FragmentPiece& piece = frag.pieces[i];

        if (piece.opensWith == OpensWith::Chapter) {
            // Flush the running scene body before moving on, so createChapter sees a coherent tree.
            auto w = saveBody(runSceneID, runSceneMetaPath, runSceneContent, runSceneBody);
            if (!w.ok()) return R::failure(w.error());

            CreateChapterRequest chReq;
            chReq.projectRootPath = root;
            chReq.appSupportRoot  = request.appSupportRoot;
            chReq.projectID       = request.projectID;
            chReq.author          = request.author;
            chReq.afterChapterID  = runChapterID;
            auto ch = chapterCreator.create(chReq);
            if (!ch.ok()) return R::failure(ch.error());

            result.createdChapterIDs.push_back(ch.value().chapterID);
            result.createdSceneIDs.push_back(ch.value().firstSceneID);

            // Restore the captured chapter + first-scene titles (T-0357). A chapter piece carries
            // both its chapterTitle and (as any piece) its sceneTitle for the scene it opens.
            auto ct = applyChapterTitle(ch.value().chapterMetadataPath, piece.chapterTitle);
            if (!ct.ok()) return R::failure(ct.error());
            auto st = applySceneTitle(ch.value().firstSceneMetadataPath, piece.sceneTitle);
            if (!st.ok()) return R::failure(st.error());

            runChapterID     = ch.value().chapterID;
            runSceneID       = ch.value().firstSceneID;
            runSceneMetaPath = ch.value().firstSceneMetadataPath;
            runSceneContent  = ch.value().firstSceneContentPath;
            runSceneBody     = piece.text;
        } else {
            // opensWith Scene (or None, which only ever applies to piece 0): a new scene in the
            // running chapter, immediately after the running scene.
            auto w = saveBody(runSceneID, runSceneMetaPath, runSceneContent, runSceneBody);
            if (!w.ok()) return R::failure(w.error());

            CreateSceneRequest scReq;
            scReq.projectRootPath = root;
            scReq.appSupportRoot  = request.appSupportRoot;
            scReq.projectID       = request.projectID;
            scReq.chapterID       = runChapterID;
            scReq.afterSceneID    = runSceneID;
            scReq.author          = request.author;
            auto sc = sceneCreator.create(scReq);
            if (!sc.ok()) return R::failure(sc.error());

            result.createdSceneIDs.push_back(sc.value().sceneID);

            // Restore the captured scene title (T-0357).
            auto st = applySceneTitle(sc.value().metadataPath, piece.sceneTitle);
            if (!st.ok()) return R::failure(st.error());

            runSceneID       = sc.value().sceneID;
            runSceneMetaPath = sc.value().metadataPath;
            runSceneContent  = sc.value().contentPath;
            runSceneBody     = piece.text;
        }
    }

    // The target scene's tail-suffix follows the WHOLE pasted run: append it to the running
    // insertion point (after the last piece's text). Flat-document model — direct concatenation
    // (the tail continues the text where the last piece left off, no paragraph break).
    runSceneBody = runSceneBody + tail;

    // Persist the final running scene body.
    auto w = saveBody(runSceneID, runSceneMetaPath, runSceneContent, runSceneBody);
    if (!w.ok()) return R::failure(w.error());

    return R::success(std::move(result));
}

Result<UncutPasteResult> FragmentPaster::uncutPaste(const UncutPasteRequest& request)
{
    using R = Result<UncutPasteResult>;

    const Fragment& frag = request.fragment;
    if (frag.pieces.empty())
        return R::failure(invalidArg("empty fragment: nothing to un-paste"));

    const AbsolutePath& root = request.projectRootPath;

    // Resolve the manuscript to find the target scene + every created scene by ID.
    ManuscriptOrderResolver resolver{services_};
    auto resolvedR = resolver.resolve(root);
    if (!resolvedR.ok())
        return R::failure(resolvedR.error());

    std::unordered_map<std::string, const ResolvedScene*> sceneByID;
    for (const auto& s : resolvedR.value())
        sceneByID.emplace(s.sceneID.value, &s);

    auto findScene = [&](const std::string& id) -> const ResolvedScene* {
        auto it = sceneByID.find(id);
        return it == sceneByID.end() ? nullptr : it->second;
    };

    const ResolvedScene* target = findScene(request.targetSceneID.value);
    if (target == nullptr)
        return R::failure(invalidArg("target scene not in manuscript: " + request.targetSceneID.value));

    SceneReader reader{services_};

    // Restore the target's original head: paste made target = head + piece0.text, so strip the
    // trailing piece0.text (by byte length) to recover `head`.
    auto targetBodyR = reader.readContent(root, target->contentPath);
    if (!targetBodyR.ok())
        return R::failure(targetBodyR.error());
    const std::string& targetBody = targetBodyR.value();
    const std::string& firstPieceText = frag.pieces.front().text;
    if (targetBody.size() < firstPieceText.size() ||
        targetBody.compare(targetBody.size() - firstPieceText.size(),
                           firstPieceText.size(), firstPieceText) != 0)
        return R::failure(invalidArg("target scene does not end with the pasted head text — "
                                     "not a state this fragment's paste produced"));
    const std::string head = targetBody.substr(0, targetBody.size() - firstPieceText.size());

    // Restore the original tail: the paste appended the target's tail after the LAST piece's text
    // in the last created scene (or, if the fragment created no scenes, the tail followed piece0 in
    // the target itself — but then createdSceneIDs is empty and head already holds everything up to
    // piece0, with the tail right after it: strip piece0 from target gives head, and the tail sits
    // between head and the stripped piece0. That single-scene case is a same-scene paste, which is
    // not a cross-boundary structured paste; guarded by the caller. Here createdSceneIDs is non-empty).
    std::string tail;
    if (!request.createdSceneIDs.empty()) {
        const ResolvedScene* lastCreated = findScene(request.createdSceneIDs.back().value);
        if (lastCreated == nullptr)
            return R::failure(invalidArg("created scene not in manuscript: " +
                                         request.createdSceneIDs.back().value));
        auto lastBodyR = reader.readContent(root, lastCreated->contentPath);
        if (!lastBodyR.ok())
            return R::failure(lastBodyR.error());
        const std::string& lastBody = lastBodyR.value();
        const std::string& lastPieceText = frag.pieces.back().text;
        if (lastBody.size() < lastPieceText.size() ||
            lastBody.compare(0, lastPieceText.size(), lastPieceText) != 0)
            return R::failure(invalidArg("last created scene does not begin with the pasted tail "
                                         "text — not a state this fragment's paste produced"));
        tail = lastBody.substr(lastPieceText.size());
    }

    // 1. Fold the restored body back into the target BEFORE deleting anything (so a later failure
    //    never loses text). Direct write, matching FragmentCutter's fold — no authorship re-stamp.
    {
        auto& fs = *services_.fileSystem;
        auto write = fs.atomicWriteTextFile(util::join(root, target->contentPath), head + tail);
        if (!write.ok())
            return R::failure(write.error());
    }

    // 2. Delete every created scene (reading order doesn't matter for deletion by ID). A created
    //    scene inside a created chapter is removed when its chapter is deleted below, so skip those
    //    to avoid a double-delete; delete only scenes NOT owned by a created chapter.
    std::unordered_map<std::string, bool> createdChapterSet;
    for (const auto& ch : request.createdChapterIDs)
        createdChapterSet.emplace(ch.value, true);

    SceneDeleter sceneDeleter{services_};
    for (const auto& sid : request.createdSceneIDs) {
        const ResolvedScene* sc = findScene(sid.value);
        if (sc == nullptr) continue;                                  // already gone
        if (createdChapterSet.count(sc->chapterID.value)) continue;   // removed with its chapter
        DeleteSceneRequest del;
        del.projectRootPath = root;
        del.sceneID         = sid;
        auto d = sceneDeleter.remove(del);
        if (!d.ok()) return R::failure(d.error());
    }

    // 3. Delete every created chapter (removes its folder + the scenes it owned).
    ChapterDeleter chapterDeleter{services_};
    for (const auto& cid : request.createdChapterIDs) {
        DeleteChapterRequest del;
        del.projectRootPath = root;
        del.chapterID       = cid;
        auto d = chapterDeleter.remove(del);
        if (!d.ok()) return R::failure(d.error());
    }

    UncutPasteResult result;
    result.survivingSceneID = target->sceneID;
    return R::success(std::move(result));
}

} // namespace scrivi::manuscript
