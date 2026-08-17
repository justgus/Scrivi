#include "objects/RelationTypes.hpp"

#include "util/Json.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>

namespace scrivi::objects {

namespace {

constexpr std::string_view kSchema     = "scrivi.relation-types.v1";
constexpr std::string_view kSceneToken = "scene";   // not an ObjectKind (§8)

std::string directionToStr(CanonicalDirection d) {
    return d == CanonicalDirection::lexical ? "lexical" : "source-to-target";
}

CanonicalDirection directionFromStr(std::string_view s) {
    return s == "lexical" ? CanonicalDirection::lexical
                          : CanonicalDirection::sourceToTarget;
}

// Writes one endpoint constraint. "Any kind" is represented by ABSENCE, never by
// an explicit null — JsonDoc cannot tell an absent key from a null (both read as
// ""), so absence is the only unambiguous encoding.
void setKindConstraint(util::JsonDoc& doc, std::string_view key,
                       const std::optional<ObjectKind>& kind, bool isScene) {
    if (isScene) { doc.setString(key, std::string(kSceneToken)); return; }
    if (kind)    { doc.setString(key, objectKindName(*kind)); }
    // else: leave the key absent = any kind
}

void readKindConstraint(const util::JsonDoc& doc, std::string_view key,
                        std::optional<ObjectKind>& outKind, bool& outIsScene) {
    const std::string raw = doc.getString(key);
    outIsScene = (raw == kSceneToken);
    outKind    = outIsScene ? std::nullopt : objectKindFromName(raw);
    // raw empty (absent) or unrecognised → nullopt = any kind
}

} // namespace

RelationTypeStore::RelationTypeStore(CoreServices& services)
    : services_(services) {}

AbsolutePath RelationTypeStore::path(const AbsolutePath& projectRoot) {
    return util::join(util::join(projectRoot, "objects"), "relation-types.json");
}

std::vector<RelationType> RelationTypeStore::seedTypes() {
    // Verbatim from Worldbuilding Object Model v0.2 §5.1 (ruled 2026-08-12 R2:
    // seed all four, scene-endpoint types included).
    std::vector<RelationType> out;

    // ⚠️ I-0125 (SP-102 R5, user-ruled 2026-08-17): `sourceKind` is deliberately
    // UNSET — any kind may appear in a scene.
    //
    // It was `ObjectKind::character`, which meant only a character could ever be
    // linked to a scene by this type. But the Apple layer gives `appears-in` to
    // EIGHT of the ten object cards (chronicle, building, vehicle, item, map,
    // artifact, faction, rule), so creating any of those from its card wrote the
    // object to disk and then failed at the edge — reported to the writer as a
    // failed creation, leaving an object she was told did not exist. Eight of ten
    // cards could never link anything, and no test caught it because the suites
    // exercise `character`.
    //
    // The inverse label is now kind-neutral: "has characters" was already wrong the
    // moment a chronicle or a faction used the type.
    RelationType appearsIn;
    appearsIn.code          = "appears-in";
    appearsIn.forwardLabel  = "appears in";
    appearsIn.inverseLabel  = "features";
    appearsIn.targetIsScene = true;
    out.push_back(std::move(appearsIn));

    RelationType locatedAt;
    locatedAt.code          = "located-at";
    locatedAt.forwardLabel  = "takes place at";
    locatedAt.inverseLabel  = "hosts";
    locatedAt.sourceIsScene = true;
    locatedAt.targetKind    = ObjectKind::location;
    out.push_back(std::move(locatedAt));

    RelationType siblingOf;
    siblingOf.code               = "sibling-of";
    siblingOf.forwardLabel       = "sibling of";
    siblingOf.inverseLabel       = "sibling of";
    siblingOf.sourceKind         = ObjectKind::character;
    siblingOf.targetKind         = ObjectKind::character;
    siblingOf.canonicalDirection = CanonicalDirection::lexical;
    siblingOf.symmetric          = true;
    out.push_back(std::move(siblingOf));

    // §3.4 (amended 2026-08-12): citations attach to OBJECTS, not scenes. The
    // first type unconstrained on BOTH ends — a citation may document any kind.
    RelationType cites;
    cites.code         = "cites";
    cites.forwardLabel = "cites";
    cites.inverseLabel = "documented by";
    out.push_back(std::move(cites));

    return out;
}

Result<void> RelationTypeStore::validate(const RelationType& type) {
    if (type.code.empty()) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument, .message = "relation type requires a code"});
    }
    if (type.forwardLabel.empty() || type.inverseLabel.empty()) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "relation type '" + type.code + "' requires both labels"});
    }

    if (type.symmetric) {
        // §5.3: symmetric canonicalization sorts the endpoint IDs. A symmetric
        // type declaring source-to-target would normalize "A rel B" and
        // "B rel A" to two DIFFERENT canonical edges, reintroducing exactly the
        // duplication this model exists to prevent.
        if (type.canonicalDirection != CanonicalDirection::lexical) {
            return Result<void>::failure(
                {.code = ErrorCode::invalidArgument,
                 .message = "symmetric relation type '" + type.code +
                            "' must use canonicalDirection 'lexical'"});
        }
        if (type.forwardLabel != type.inverseLabel) {
            return Result<void>::failure(
                {.code = ErrorCode::invalidArgument,
                 .message = "symmetric relation type '" + type.code +
                            "' must have identical forward and inverse labels"});
        }
    } else if (type.canonicalDirection == CanonicalDirection::lexical) {
        // The inverse trap: lexical ordering discards the declared direction, so
        // an asymmetric type could no longer tell "cites" from "documented by".
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "asymmetric relation type '" + type.code +
                        "' cannot use canonicalDirection 'lexical'"});
    }

    return Result<void>::success();
}

Result<std::vector<RelationType>>
RelationTypeStore::load(const AbsolutePath& projectRoot) const {
    auto& fs = *services_.fileSystem;
    auto  p  = path(projectRoot);

    auto existsR = fs.exists(p);
    if (existsR.ok() && existsR.value()) {
        if (auto textR = fs.readTextFile(p); textR.ok()) {
            if (auto docR = util::parseJson(textR.value()); docR.ok()) {
                const auto& doc = docR.value();
                if (doc.getString("schema") == kSchema) {
                    std::vector<RelationType> types;
                    const auto count = doc.arraySize("types");
                    types.reserve(count);
                    for (std::size_t i = 0; i < count; ++i) {
                        auto item = doc.arrayItem("types", i);
                        RelationType t;
                        t.code         = item.getString("code");
                        if (t.code.empty()) { continue; }   // skip junk rows
                        t.forwardLabel = item.getString("forwardLabel");
                        t.inverseLabel = item.getString("inverseLabel");
                        readKindConstraint(item, "sourceKind", t.sourceKind, t.sourceIsScene);
                        readKindConstraint(item, "targetKind", t.targetKind, t.targetIsScene);
                        t.canonicalDirection = directionFromStr(item.getString("canonicalDirection"));
                        t.symmetric          = item.getBool("symmetric");
                        types.push_back(std::move(t));
                    }
                    if (!types.empty()) {
                        return Result<std::vector<RelationType>>::success(std::move(types));
                    }
                }
            }
        }
        // Present but unusable — fall through and re-seed. A project must always
        // be able to relate things.
    }

    auto seeded = seedTypes();
    if (auto r = write(projectRoot, seeded); !r.ok()) {
        return Result<std::vector<RelationType>>::failure(r.error());
    }
    return Result<std::vector<RelationType>>::success(std::move(seeded));
}

Result<void> RelationTypeStore::write(const AbsolutePath& projectRoot,
                                       const std::vector<RelationType>& types) const {
    auto& fs = *services_.fileSystem;

    auto objectsDir = util::join(projectRoot, "objects");
    if (auto r = fs.createDirectories(objectsDir); !r.ok()) { return r; }

    util::JsonDoc root;
    root.setString("schema", std::string(kSchema));

    for (const auto& t : types) {
        util::JsonDoc item;
        item.setString("code",         t.code);
        item.setString("forwardLabel", t.forwardLabel);
        item.setString("inverseLabel", t.inverseLabel);
        setKindConstraint(item, "sourceKind", t.sourceKind, t.sourceIsScene);
        setKindConstraint(item, "targetKind", t.targetKind, t.targetIsScene);
        item.setString("canonicalDirection", directionToStr(t.canonicalDirection));
        item.setBool("symmetric", t.symmetric);
        root.appendToArray("types", std::move(item));
    }

    return fs.atomicWriteTextFile(path(projectRoot), root.dump());
}

Result<void> RelationTypeStore::upsert(const AbsolutePath& projectRoot,
                                        const RelationType& type) const {
    if (auto r = validate(type); !r.ok()) { return r; }

    auto loadedR = load(projectRoot);
    if (!loadedR.ok()) { return Result<void>::failure(loadedR.error()); }

    auto types = std::move(loadedR.value());
    auto it = std::find_if(types.begin(), types.end(),
                           [&](const RelationType& t) { return t.code == type.code; });
    if (it != types.end()) { *it = type; } else { types.push_back(type); }

    return write(projectRoot, types);
}

Result<RelationType> RelationTypeStore::find(const AbsolutePath& projectRoot,
                                              const std::string& code) const {
    auto loadedR = load(projectRoot);
    if (!loadedR.ok()) { return Result<RelationType>::failure(loadedR.error()); }

    for (const auto& t : loadedR.value()) {
        if (t.code == code) { return Result<RelationType>::success(t); }
    }

    return Result<RelationType>::failure(
        {.code = ErrorCode::invalidArgument, .message = "unknown relation type: '" + code + "'"});
}

} // namespace scrivi::objects
