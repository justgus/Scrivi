#include "WorldJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

namespace {
constexpr std::string_view kWorldSchema   = "scrivi.world.v1";
constexpr std::string_view kBindingSchema = "scrivi.world-binding.v1";
} // namespace

// ---------------------------------------------------------------------------
// world.json
// ---------------------------------------------------------------------------

std::string serializeWorld(const worlds::WorldRecord& w) {
    util::JsonDoc epoch;
    epoch.setString("label", w.epochLabel);

    util::JsonDoc root;
    root.setString("schema",      std::string(kWorldSchema));
    root.setString("worldID",     w.worldID);
    root.setString("displayName", w.displayName);
    root.setSubDoc("epoch",       std::move(epoch));
    root.setString("createdAt",   w.createdAt);
    root.setString("modifiedAt",  w.modifiedAt);
    root.setInt   ("formatVersion", w.formatVersion);
    return root.dump();
}

Result<worlds::WorldRecord> parseWorld(std::string_view json) {
    auto r = parseAndValidateSchema(json, kWorldSchema);
    if (!r.ok()) { return Result<worlds::WorldRecord>::failure(r.error()); }
    auto& doc = r.value();

    worlds::WorldRecord w;
    w.worldID       = doc.getString("worldID");
    w.displayName   = doc.getString("displayName");
    w.epochLabel    = doc.getSubDoc("epoch").getString("label");
    w.createdAt     = doc.getString("createdAt");
    w.modifiedAt    = doc.getString("modifiedAt");
    w.formatVersion = doc.getInt("formatVersion", 1);

    if (w.worldID.empty()) {
        return Result<worlds::WorldRecord>::failure(
            {.code = ErrorCode::validationError, .message = "world.json has no worldID"});
    }
    return Result<worlds::WorldRecord>::success(std::move(w));
}

// ---------------------------------------------------------------------------
// binding.json
// ---------------------------------------------------------------------------

std::string serializeWorldBinding(const worlds::WorldBindingRecord& b) {
    util::JsonDoc ref;
    ref.setString("lastKnownPath",         b.reference.lastKnownPath);
    ref.setString("lastKnownAbsolutePath", b.reference.lastKnownAbsolutePath);
    ref.setString("volumeLabel",           b.reference.volumeLabel);

    util::JsonDoc root;
    root.setString("schema",        std::string(kBindingSchema));
    root.setString("worldID",       b.worldID);
    root.setString("displayName",   b.displayName);
    root.setInt64 ("epochOffsetMs", b.epochOffsetMs);
    root.setSubDoc("reference",     std::move(ref));

    for (const auto& e : b.cachedIndex) {
        util::JsonDoc item;
        item.setString("objectID",    e.objectID);
        item.setString("kind",        e.kind);
        item.setString("displayName", e.displayName);
        root.appendToArray("cachedIndex", std::move(item));
    }

    return root.dump();
}

Result<worlds::WorldBindingRecord> parseWorldBinding(std::string_view json) {
    auto r = parseAndValidateSchema(json, kBindingSchema);
    if (!r.ok()) { return Result<worlds::WorldBindingRecord>::failure(r.error()); }
    auto& doc = r.value();

    worlds::WorldBindingRecord b;
    b.worldID       = doc.getString("worldID");
    b.displayName   = doc.getString("displayName");
    b.epochOffsetMs = doc.getInt64("epochOffsetMs");

    auto ref = doc.getSubDoc("reference");
    b.reference.lastKnownPath         = ref.getString("lastKnownPath");
    b.reference.lastKnownAbsolutePath = ref.getString("lastKnownAbsolutePath");
    b.reference.volumeLabel           = ref.getString("volumeLabel");

    const auto count = doc.arraySize("cachedIndex");
    b.cachedIndex.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        auto item = doc.arrayItem("cachedIndex", i);
        worlds::CachedWorldObject e;
        e.objectID    = item.getString("objectID");
        e.kind        = item.getString("kind");
        e.displayName = item.getString("displayName");
        if (!e.objectID.empty()) { b.cachedIndex.push_back(std::move(e)); }
    }

    if (b.worldID.empty()) {
        return Result<worlds::WorldBindingRecord>::failure(
            {.code = ErrorCode::validationError, .message = "binding.json has no worldID"});
    }
    return Result<worlds::WorldBindingRecord>::success(std::move(b));
}

} // namespace scrivi::schemas
