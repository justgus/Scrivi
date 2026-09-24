#include "manuscript/ProjectIndex.hpp"

#include "manuscript/ChapterIndex.hpp"
#include "manuscript/SceneIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <unordered_set>

namespace scrivi::manuscript {

bool storyTimeIsExplicitlySet(const SceneStoryTime& st) {
    // Design §9. ⚠️ "Set" is a question about the WRITER'S INTENT, not about whether the
    // struct holds non-zero numbers: `durationMs` defaults to 3,600,000 and `offsetMs` is
    // DERIVED for every scene on the default chain, so testing the values would report
    // every scene as set and make the call dense again.
    return st.offsetSource != "default"
        || st.durationSource != "default"
        || !st.bandID.empty();
}

Result<ProjectIndex> ProjectIndex::build(const AbsolutePath& projectRoot,
                                         CoreServices&       services) {
    // ⚠️ ONE traversal feeds all three indexes (AC3). `resolve()` already walks
    // chapters→scenes in filesystem-authoritative order and parses each sidecar, so
    // everything the three indexes need is in this single pass.
    ManuscriptOrderResolver resolver{services};
    auto scenesR = resolver.resolve(projectRoot);
    if (!scenesR.ok()) {
        // ⚠️ Leaves the index INVALID and EMPTY -- never half-built. A caller that gets a
        // failure here must traverse, not consult a partial index.
        return Result<ProjectIndex>::failure(scenesR.error());
    }

    ProjectIndex index;
    const auto&  resolved = scenesR.value();
    index.scenes_.reserve(resolved.size());
    index.bySceneID_.reserve(resolved.size());
    index.storyTimes_.reserve(resolved.size());

    for (std::size_t i = 0; i < resolved.size(); ++i) {
        const auto& rs = resolved[i];

        SceneLocation loc;
        loc.sceneID             = rs.sceneID;
        loc.chapterID           = rs.chapterID;
        loc.title               = rs.title;
        loc.chapterTitle        = rs.chapterTitle;
        loc.slug                = rs.slug;
        loc.status              = rs.status;
        loc.metadataPath        = rs.metadataPath;
        loc.contentPath         = rs.contentPath;
        loc.chapterMetadataPath = rs.chapterMetadataPath;
        loc.ordinal             = i;

        // ⚠️ A duplicate sceneID across two sidecars is possible on disk (a copied scene
        // file, a resolved merge conflict). ✅ FIRST WINS, matching manuscript order, and
        // the duplicate stays reachable through scenesInOrder() -- dropping it silently
        // would make the index disagree with the traversal it was built from.
        index.bySceneID_.try_emplace(rs.sceneID.value, i);
        index.storyTimes_.try_emplace(rs.sceneID.value, rs.storyTime);
        index.scenes_.push_back(std::move(loc));
    }

    index.valid_ = true;
    return Result<ProjectIndex>::success(std::move(index));
}

std::optional<SceneLocation> ProjectIndex::findScene(const SceneID& sceneID) const {
    if (!valid_) { return std::nullopt; }
    const auto it = bySceneID_.find(sceneID.value);
    if (it == bySceneID_.end()) { return std::nullopt; }
    return scenes_[it->second];
}

std::optional<SceneStoryTime> ProjectIndex::storyTime(const SceneID& sceneID) const {
    if (!valid_) { return std::nullopt; }
    const auto it = storyTimes_.find(sceneID.value);
    if (it == storyTimes_.end()) { return std::nullopt; }
    return it->second;
}

bool ProjectIndex::rebuildChapters(const AbsolutePath&           projectRoot,
                                   CoreServices&                 services,
                                   const std::vector<ChapterID>& changed) {
    if (!valid_) { return false; }        // nothing to update incrementally
    if (changed.empty())  { return true; }  // nothing changed; index already correct

    auto& fs = *services.fileSystem;

    // ⚠️ CHAPTER ORDER IS RE-OBSERVED, NOT ASSUMED. A Class B op can rename a chapter
    // folder (reorder), add one, or remove one. ✅ This is the cheap half of the walk:
    // one directory scan plus one sidecar read per chapter.
    auto chaptersR = listChaptersByOrder(fs, projectRoot);
    if (!chaptersR.ok()) { return false; }   // ⛔ caller drops the whole index

    std::unordered_set<std::string> mustReread;
    mustReread.reserve(changed.size());
    for (const auto& id : changed) { mustReread.insert(id.value); }

    // ✅ The scenes this index already holds, grouped by chapter, so an UNAFFECTED
    // chapter costs nothing to carry forward.
    std::unordered_map<std::string, std::vector<SceneLocation>> cachedByChapter;
    cachedByChapter.reserve(chaptersR.value().size());
    for (const auto& loc : scenes_) {
        cachedByChapter[loc.chapterID.value].push_back(loc);
    }

    // ⚠️ BUILT INTO LOCALS. ⛔ Nothing below touches this index's own state until the
    // whole rebuild has SUCCEEDED -- a half-updated index is the failure mode that
    // makes an index worse than no index at all.
    std::vector<SceneLocation>                      nextScenes;
    std::unordered_map<std::string, SceneStoryTime> nextStoryTimes;
    nextScenes.reserve(scenes_.size());
    nextStoryTimes.reserve(storyTimes_.size());

    for (const auto& ch : chaptersR.value()) {
        const bool isNew     = cachedByChapter.find(ch.chapterID.value) == cachedByChapter.end();
        const bool reread    = isNew || mustReread.count(ch.chapterID.value) > 0;

        if (!reread) {
            // ✅ CARRIED OVER. ⚠️ Its scenes keep their existing relative order; only
            // `ordinal` is restamped below, because a chapter ahead of it may have
            // changed size.
            for (auto& loc : cachedByChapter[ch.chapterID.value]) {
                // ⚠️ A carried-over scene MUST already have a story time entry -- it came
                // from a completed build. ⛔ If it does not, this index is already
                // inconsistent and the caller must drop it rather than paper over it.
                const auto st = storyTimes_.find(loc.sceneID.value);
                if (st == storyTimes_.end()) { return false; }
                nextStoryTimes.emplace(loc.sceneID.value, st->second);
                nextScenes.push_back(std::move(loc));
            }
            continue;
        }

        // ⚠️ RE-READ: this chapter changed (or is new to the index), so its scenes come
        // from disk exactly as `build()` would read them.
        auto chTextR = fs.readTextFile(util::join(projectRoot, ch.chapterMetadataRelPath));
        if (!chTextR.ok()) { return false; }
        auto chParsed = schemas::parseChapterMeta(chTextR.value());
        if (!chParsed.ok()) { return false; }

        auto scenesR = listScenesByOrder(fs, projectRoot, ch.chapterMetadataRelPath);
        if (!scenesR.ok()) { return false; }

        const std::string chDir = ch.chapterMetadataRelPath.substr(
            0, ch.chapterMetadataRelPath.find_last_of('/'));

        for (const auto& se : scenesR.value()) {
            auto sTextR = fs.readTextFile(util::join(projectRoot, se.metadataRelPath));
            if (!sTextR.ok()) { return false; }
            auto sParsed = schemas::parseSceneMeta(sTextR.value());
            if (!sParsed.ok()) { return false; }

            SceneLocation loc;
            loc.sceneID             = sParsed.value().sceneID;
            loc.chapterID           = chParsed.value().chapterID;
            loc.title               = sParsed.value().title;
            loc.chapterTitle        = chParsed.value().title;
            loc.slug                = sParsed.value().slug;
            loc.status              = sParsed.value().status;
            loc.metadataPath        = se.metadataRelPath;
            // §8.1: sidecar contentPath is a bare filename -- resolve against the dir.
            loc.contentPath         = chDir + "/" + sParsed.value().contentPath;
            loc.chapterMetadataPath = ch.chapterMetadataRelPath;

            // ⚠️ FIRST WINS on a duplicate sceneID, matching `build()` exactly -- the two
            // must not disagree about a manuscript that has one.
            nextStoryTimes.try_emplace(loc.sceneID.value, sParsed.value().storyTime);
            nextScenes.push_back(std::move(loc));
        }
    }

    // ⚠️ ORDINALS ARE RESTAMPED ACROSS THE WHOLE VECTOR. Free, and REQUIRED: splicing a
    // chapter shifts every later scene's position.
    std::unordered_map<std::string, std::size_t> nextBySceneID;
    nextBySceneID.reserve(nextScenes.size());
    for (std::size_t i = 0; i < nextScenes.size(); ++i) {
        nextScenes[i].ordinal = i;
        nextBySceneID.try_emplace(nextScenes[i].sceneID.value, i);
    }

    // ✅ COMMIT -- one assignment each, after everything above succeeded.
    scenes_     = std::move(nextScenes);
    bySceneID_  = std::move(nextBySceneID);
    storyTimes_ = std::move(nextStoryTimes);
    return true;
}

bool ProjectIndex::patchStoryTime(const SceneID& sceneID, const SceneStoryTime& st) {
    // ⚠️ An invalid index is not patchable -- it describes nothing, so there is no
    // entry to correct. ✅ The caller drops it and the next query rebuilds.
    if (!valid_) { return false; }

    // ⚠️ `bySceneID_` is the membership question, NOT `storyTimes_`. A scene on the
    // default chain is IN the manuscript but may have no `storyTimes_` entry yet, and
    // setting a story time for the first time is exactly the case that must work.
    if (bySceneID_.find(sceneID.value) == bySceneID_.end()) { return false; }

    // ✅ insert-or-assign: the first explicit set creates the entry, a later one
    // replaces it. ⚠️ `try_emplace` would silently keep the OLD value -- the same
    // shape as the `count:0` staleness AC5b exists to catch.
    storyTimes_.insert_or_assign(sceneID.value, st);
    return true;
}

std::vector<std::pair<SceneID, SceneStoryTime>> ProjectIndex::explicitStoryTimes() const {
    std::vector<std::pair<SceneID, SceneStoryTime>> out;
    if (!valid_) { return out; }

    // ⚠️ Manuscript order, not hash order -- the timeline draws in reading order and an
    // arbitrary iteration order would make the result non-deterministic between runs.
    for (const auto& scene : scenes_) {
        const auto it = storyTimes_.find(scene.sceneID.value);
        if (it == storyTimes_.end()) { continue; }
        if (storyTimeIsExplicitlySet(it->second)) {
            out.emplace_back(scene.sceneID, it->second);
        }
    }
    return out;
}

} // namespace scrivi::manuscript
