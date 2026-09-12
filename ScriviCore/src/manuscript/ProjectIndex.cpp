#include "manuscript/ProjectIndex.hpp"

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
