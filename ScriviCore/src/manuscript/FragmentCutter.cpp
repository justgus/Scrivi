#include "manuscript/FragmentCutter.hpp"

#include "manuscript/ChapterDeleter.hpp"
#include "manuscript/ChapterMerger.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "manuscript/SceneDeleter.hpp"
#include "manuscript/SceneReader.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace scrivi::manuscript {

namespace {

Error invalidArg(std::string message)
{
    return Error{ErrorCode::invalidArgument, std::move(message), "", ""};
}

} // namespace

FragmentCutter::FragmentCutter(CoreServices& services)
    : services_(services) {}

Result<CutFragmentResult> FragmentCutter::cut(const AbsolutePath& projectRoot,
                                              const std::vector<FragmentSpan>& spans)
{
    using R = Result<CutFragmentResult>;

    if (spans.empty())
        return R::failure(invalidArg("no spans: nothing to cut"));

    // 1. Extract the fragment first (validates the spans + gives us the removed content). This
    //    also guarantees every downstream step works from a validated, in-order span list.
    FragmentExtractor extractor{services_};
    auto fragR = extractor.extract(projectRoot, spans);
    if (!fragR.ok())
        return R::failure(fragR.error());

    // Resolve the manuscript so we can read the head/tail bodies and know each span scene's
    // chapter membership + paths.
    ManuscriptOrderResolver resolver{services_};
    auto resolvedR = resolver.resolve(projectRoot);
    if (!resolvedR.ok())
        return R::failure(resolvedR.error());
    const auto& ordered = resolvedR.value();

    std::unordered_map<std::string, const ResolvedScene*> sceneByID;
    sceneByID.reserve(ordered.size());
    for (const auto& s : ordered)
        sceneByID.emplace(s.sceneID.value, &s);

    // The head span (first) keeps its prefix; the tail span (last) keeps its suffix.
    const FragmentSpan& headSpan = spans.front();
    const FragmentSpan& tailSpan = spans.back();
    const ResolvedScene* headScene = sceneByID.at(headSpan.sceneID.value);

    SceneReader reader{services_};

    // headPrefix = the head scene's bytes BEFORE the span start.
    auto headBodyR = reader.readContent(projectRoot, headScene->contentPath);
    if (!headBodyR.ok())
        return R::failure(headBodyR.error());
    const std::string headPrefix = headBodyR.value().substr(0, headSpan.startByte);

    // tailSuffix = the tail scene's bytes AFTER the span end (when the span is a single scene the
    // head and tail scenes are the same, so read from that one body).
    std::string tailSuffix;
    {
        const ResolvedScene* tailScene = sceneByID.at(tailSpan.sceneID.value);
        auto tailBodyR = reader.readContent(projectRoot, tailScene->contentPath);
        if (!tailBodyR.ok())
            return R::failure(tailBodyR.error());
        const std::string& tb = tailBodyR.value();
        tailSuffix = (tailSpan.endByte <= tb.size()) ? tb.substr(tailSpan.endByte) : std::string();
    }

    CutFragmentResult result;
    result.fragment         = std::move(fragR.value());
    result.survivingSceneID = headScene->sceneID;

    // 2. Fold the survivors into the head scene BEFORE deleting anything (so a later failure never
    //    loses surviving text). Write the content file directly (matching SceneMerger) — a
    //    structural fold doesn't re-stamp authorship/workspace state, and needs no appSupportRoot.
    //    The join is DIRECT concatenation, no blank-line seam (user-ruled 2026-07-27): a cut just
    //    deletes the selected characters and closes the gap — like deleting a selection in any
    //    editor — so the surviving edges simply abut, whatever whitespace the selection left.
    {
        auto& fs = *services_.fileSystem;
        auto w = fs.atomicWriteTextFile(util::join(projectRoot, headScene->contentPath),
                                        headPrefix + tailSuffix);
        if (!w.ok())
            return R::failure(w.error());
    }

    // 3. Delete every OTHER scene in the span (interior scenes + the tail scene). The head scene is
    //    the sole survivor. Track which chapters lose scenes so we can drop the emptied ones.
    SceneDeleter sceneDeleter{services_};
    std::unordered_set<std::string> touchedChapters;
    for (std::size_t i = 1; i < spans.size(); ++i) {   // skip index 0 = the head
        const auto& span = spans[i];
        const ResolvedScene* sc = sceneByID.at(span.sceneID.value);
        touchedChapters.insert(sc->chapterID.value);

        DeleteSceneRequest del;
        del.projectRootPath = projectRoot;
        del.sceneID         = span.sceneID;
        auto d = sceneDeleter.remove(del);
        if (!d.ok())
            return R::failure(d.error());
        result.removedSceneIDs.push_back(span.sceneID);
    }

    // 4. Remove any chapter that is now empty (all of its scenes were in the span). Re-resolve and
    //    see which touched chapters no longer appear in the manuscript's scene list. Never remove
    //    the head scene's own chapter (it still holds the survivor).
    if (!touchedChapters.empty()) {
        auto afterR = resolver.resolve(projectRoot);
        if (!afterR.ok())
            return R::failure(afterR.error());
        std::unordered_set<std::string> chaptersStillPresent;
        for (const auto& s : afterR.value())
            chaptersStillPresent.insert(s.chapterID.value);

        ChapterDeleter chapterDeleter{services_};
        // Delete in the span's chapter order (stable, reading order) for a deterministic result.
        std::vector<std::string> emptied;
        for (std::size_t i = 1; i < spans.size(); ++i) {
            const ResolvedScene* sc = sceneByID.at(spans[i].sceneID.value);
            const std::string& chID = sc->chapterID.value;
            if (chID == headScene->chapterID.value) continue;      // head chapter always survives
            if (chaptersStillPresent.count(chID)) continue;        // still has scenes → keep
            if (std::find(emptied.begin(), emptied.end(), chID) != emptied.end()) continue;  // once
            emptied.push_back(chID);
        }
        for (const auto& chID : emptied) {
            DeleteChapterRequest del;
            del.projectRootPath = projectRoot;
            del.chapterID       = ChapterID{chID};
            auto d = chapterDeleter.remove(del);
            if (!d.ok())
                return R::failure(d.error());
            result.removedChapterIDs.push_back(ChapterID{chID});
        }
    }

    // 5. Chapter promotion (T-0357, supersedes §4.3 "leave post-span scenes untouched"). When the
    //    span crossed into a chapter that still has SURVIVING scenes (its heading region was cut but
    //    not all its scenes), fold those survivors into the head chapter so the manuscript isn't left
    //    with a duplicate-named chapter. Fully-consumed middle chapters were already removed in step 4,
    //    so each still-present spanned chapter now has the head chapter as its immediate predecessor —
    //    ChapterMerger (I-0083's atomic relocate-then-remove primitive) merges it in reading order.
    {
        // Distinct spanned chapters other than the head, in reading order.
        std::vector<std::string> spannedChapters;
        for (std::size_t i = 1; i < spans.size(); ++i) {
            const std::string& chID = sceneByID.at(spans[i].sceneID.value)->chapterID.value;
            if (chID == headScene->chapterID.value) continue;
            if (std::find(spannedChapters.begin(), spannedChapters.end(), chID) != spannedChapters.end())
                continue;
            spannedChapters.push_back(chID);
        }
        if (!spannedChapters.empty()) {
            ChapterMerger chapterMerger{services_};
            for (const auto& chID : spannedChapters) {
                // Skip chapters already removed as empty in step 4 (nothing to promote).
                bool alreadyRemoved = false;
                for (const auto& removed : result.removedChapterIDs)
                    if (removed.value == chID) { alreadyRemoved = true; break; }
                if (alreadyRemoved) continue;
                MergeChapterRequest mg;
                mg.projectRootPath = projectRoot;
                mg.chapterID       = ChapterID{chID};   // merge INTO its predecessor (now the head)
                auto m = chapterMerger.merge(mg);
                if (!m.ok())
                    return R::failure(m.error());
                // The promoted-from chapter is removed by the merge — record it for undo.
                result.removedChapterIDs.push_back(ChapterID{chID});
            }
        }
    }

    return R::success(std::move(result));
}

} // namespace scrivi::manuscript
