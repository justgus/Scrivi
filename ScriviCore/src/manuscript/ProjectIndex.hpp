#pragma once

#include "manuscript/ManuscriptOrderResolver.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace scrivi::manuscript {

// EP-039 AC1-AC3 (SP-131) -- the three in-memory indexes, built from ONE traversal.
//
// ⚠️ WHY THIS EXISTS. `ScriviCore` is STATELESS: its only member is `CoreServices`.
// So a per-item question costs a FULL TRAVERSAL by construction -- `findSceneMetaPath`
// resolved the ENTIRE manuscript to turn one sceneID into one path, and the app calls
// those per-item endpoints once per scene. ⚠️ MEASURED on a 1,153-scene manuscript:
// `getSceneStoryTime` x N = 234-251 s, and the `openScene` loop a further 64-70 s, of a
// ~300 s frozen open. ✅ Nobody wrote a nested loop; the quadratic is EMERGENT.
//
// ⚠️ THE INDEX IS DERIVED AND DISPOSABLE -- NEVER A SECOND SOURCE OF TRUTH.
// EP-027 made the filesystem AUTHORITATIVE and that ruling stands. Any doubt ⇒ throw the
// index away and rebuild from disk.
//
// ⚠️ AUTHORITATIVE FOR *LOCATION ONLY* -- never for CONTENT and never for EXISTENCE.
// An index hit yields a PATH: a hypothesis about where something lives, nothing more. The
// caller STILL OPENS THE FILE, and that open IS the validation (Design §5.2, AC5a). A
// miss, or an open that fails, falls back to a REAL TRAVERSAL and rebuilds -- ⚠️ NEVER to
// a negative claim. ✅ *Absence is never deletion* (I-0183 destroyed 10 of 12
// relationships by treating an unreadable index as an empty one.)
//
// ✅ THE CONSEQUENCE WORTH KNOWING: a stale path cannot produce a WRONG ANSWER. It can
// only produce an EXTRA TRAVERSAL. That is what makes this safe with no filesystem
// watching -- of which Scrivi has none, on any platform.

// 3.1 SceneLocationIndex -- "where does this sceneID live?"
// 3.3 ManuscriptOrderIndex -- "what is the reading order?"
//
// Both are served from one ordered vector plus an id→ordinal map: the vector IS the
// reading order, and the map turns a point lookup into O(1). ⚠️ They are separate ACs
// because each has its own measured target, but they are ONE data structure -- splitting
// the storage would mean two copies of the same traversal's output.
struct SceneLocation {
    SceneID      sceneID;
    ChapterID    chapterID;
    std::string  title;
    std::string  chapterTitle;
    Slug         slug;
    std::string  status;
    RelativePath metadataPath;
    RelativePath contentPath;
    RelativePath chapterMetadataPath;
    std::size_t  ordinal = 0;   // 0-based position in manuscript order
};

// 3.2 SceneStoryTimeIndex -- "what is this scene's story time?"
//
// ⚠️ Its own index rather than a field on SceneLocation because the ACCESS PATTERN
// differs: the timeline reads EVERY scene's story time on open (FR-022m derives each
// offset from the previous scene's end, so the chain requires all N), whereas a location
// lookup is a point query.
using SceneStoryTime = schemas::SceneStoryTime;

// A scene's story time is "SET" when the writer has said something about it -- as opposed
// to sitting on the derived default chain.
//
// ⚠️ MEASURED on the 1,203-sidecar fixture: ZERO scenes have a storyTime block at all --
// the key is `null`. ✅ So `scrivi_list_story_times` returns an EMPTY ARRAY there, and the
// timeline draws its default chain with NO per-scene I/O. ⚠️ 234-251 s was spent
// discovering that nothing is set.
[[nodiscard]] bool storyTimeIsExplicitlySet(const SceneStoryTime& st);

// The three indexes for ONE open project, built from ONE `resolve()` pass.
//
// ⚠️ NOT COPYABLE by intent: a copy is a second thing that can go stale independently.
class ProjectIndex {
public:
    ProjectIndex()                               = default;
    ProjectIndex(const ProjectIndex&)            = delete;
    ProjectIndex& operator=(const ProjectIndex&) = delete;
    ProjectIndex(ProjectIndex&&)                 = default;
    ProjectIndex& operator=(ProjectIndex&&)      = default;

    // Builds all three indexes from one ManuscriptOrderResolver::resolve() pass.
    // ⚠️ A failure leaves the index EMPTY and INVALID, never half-built.
    [[nodiscard]] static Result<ProjectIndex> build(const AbsolutePath& projectRoot,
                                                    CoreServices&       services);

    // ⚠️ Location ONLY -- the caller must still open what this points at (AC5a).
    // std::nullopt means "not in this index", ⚠️ which is NOT "does not exist".
    [[nodiscard]] std::optional<SceneLocation> findScene(const SceneID& sceneID) const;

    // Story time for one scene. std::nullopt ⇒ not indexed; see findScene.
    [[nodiscard]] std::optional<SceneStoryTime> storyTime(const SceneID& sceneID) const;

    // Manuscript order -- the whole reading order, in order.
    [[nodiscard]] const std::vector<SceneLocation>& scenesInOrder() const { return scenes_; }

    // Every scene whose story time is EXPLICITLY SET, in manuscript order.
    // ✅ Sparse by design: on a project where nothing is set this is EMPTY, and empty is
    // the COMMON case, not an error (AC4).
    [[nodiscard]] std::vector<std::pair<SceneID, SceneStoryTime>> explicitStoryTimes() const;

    [[nodiscard]] bool        valid() const { return valid_; }
    [[nodiscard]] std::size_t sceneCount() const { return scenes_.size(); }

private:
    std::vector<SceneLocation>                       scenes_;
    std::unordered_map<std::string, std::size_t>     bySceneID_;   // sceneID → ordinal
    std::unordered_map<std::string, SceneStoryTime>  storyTimes_;  // sceneID → story time
    bool                                             valid_ = false;
};

} // namespace scrivi::manuscript
