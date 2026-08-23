#pragma once

// RelationType vocabulary — objects/relation-types.json (EP-031 SP-096, T-0373).
// Design: Worldbuilding Object Model v0.2 §5.1, §5.3.
//
// A RelationType supplies the BIDIRECTIONAL vocabulary for an edge. One stored
// edge renders as `forwardLabel` from one endpoint and `inverseLabel` from the
// other (§5.2) — the two labels are two renderings of one relationship, never
// two records.

#include "scrivi/ObjectTypes.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"

#include <optional>
#include <string>
#include <vector>

namespace scrivi::objects {

// How an edge is normalized to a canonical direction before write/lookup (§5.3).
enum class CanonicalDirection {
    sourceToTarget,   // asymmetric: normalize to the declared source→target order
    lexical,          // symmetric: sort the two endpoint IDs lexically
};

struct RelationType {
    std::string code;
    std::string forwardLabel;
    std::string inverseLabel;

    // Kind constraints. `nullopt` = ANY kind — written as an ABSENT key, never
    // as an explicit null: JsonDoc::getString() cannot distinguish an absent key
    // from a JSON null (both yield ""), so absence is the only representation
    // that round-trips unambiguously. `cites` (§3.4) is the first type
    // unconstrained on BOTH ends.
    std::optional<ObjectKind> sourceKind;
    std::optional<ObjectKind> targetKind;

    // Scene-endpoint constraints. ObjectKind cannot express "scene" (scenes are
    // not objects, §8), so a constrained scene end is flagged separately.
    bool sourceIsScene = false;
    bool targetIsScene = false;

    CanonicalDirection canonicalDirection = CanonicalDirection::sourceToTarget;
    bool               symmetric          = false;
};

class RelationTypeStore {
public:
    explicit RelationTypeStore(CoreServices& services);

    [[nodiscard]] static AbsolutePath path(const AbsolutePath& projectRoot);

    // The four types every project starts with (§5.1, ruled 2026-08-12 R2).
    [[nodiscard]] static std::vector<RelationType> seedTypes();

    // Loads the vocabulary, seeding it when the file is missing, unreadable, or
    // malformed. Never fails because of file contents — a project must always be
    // able to relate things (same repair-before-validation posture as the object
    // index).
    //
    // ⚠️ **Also RECONCILES the seeded types** (T-0441, ruled 2026-08-21): every
    // code in `seedTypes()` is added if missing and replaced if it differs, so a
    // seed change reaches projects created before it. ⚠️ **Writer-authored codes
    // are never touched and nothing is ever deleted**; the file is writer-editable
    // by design. ⚠️ **Accepted consequence: a seeded type the writer deliberately
    // edited is overwritten** — chosen over leaving a hand-edited `appears-in`
    // broken forever with no explanation (I-0125's symptom, still live for old
    // projects until this shipped).
    //
    // ⚠️ Writes ONLY when reconciliation actually changed something. An
    // unconditional rewrite would churn mtimes and Git status on every open.
    [[nodiscard]] Result<std::vector<RelationType>> load(const AbsolutePath& projectRoot) const;

    [[nodiscard]] Result<void> write(const AbsolutePath& projectRoot,
                                     const std::vector<RelationType>& types) const;

    // Adds or replaces by `code`. Rejects declarations that would make §5.3's
    // duplicate rule unsound — see validate().
    [[nodiscard]] Result<void> upsert(const AbsolutePath& projectRoot,
                                      const RelationType& type) const;

    [[nodiscard]] Result<RelationType> find(const AbsolutePath& projectRoot,
                                            const std::string& code) const;

    // A symmetric type MUST be lexical and MUST carry identical labels: §5.3's
    // canonicalization sorts endpoints for symmetric types, so a symmetric type
    // declaring source-to-target would silently admit duplicate edges — exactly
    // the Cumberland failure this model exists to prevent.
    [[nodiscard]] static Result<void> validate(const RelationType& type);

private:
    CoreServices& services_;
};

} // namespace scrivi::objects
