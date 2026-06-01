#include "platform/SystemUUIDProvider.hpp"

#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <random>
#include <string>
#include <string_view>

namespace scrivi::platform {

namespace {

// Returns milliseconds since Unix epoch as a 48-bit value.
uint64_t unixMillis() {
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()).count());
}

// Build a UUID v7 string: xxxxxxxx-xxxx-7xxx-yxxx-xxxxxxxxxxxx
// Bit layout per RFC 9562:
//   bits 0–47:   unix_ts_ms  (48 bits)
//   bits 48–51:  version = 7 (4 bits)
//   bits 52–63:  rand_a      (12 bits)
//   bits 64–65:  variant = 0b10 (2 bits)
//   bits 66–127: rand_b      (62 bits)
std::string makeV7() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist64;
    std::uniform_int_distribution<uint32_t> dist32;

    uint64_t ts   = unixMillis() & 0x0000FFFFFFFFFFFF; // 48 bits
    uint64_t rand = dist64(gen);

    // Assemble high 64 bits: ts(48) | 0x7 (4) | rand_a (12)
    uint64_t hi = (ts << 16)
                | (uint64_t{0x7} << 12)
                | ((rand >> 52) & 0x0FFF);

    // Assemble low 64 bits: 0b10 (2) | rand_b (62)
    uint64_t lo = (uint64_t{0x8} << 60)   // variant 0b10xx
                | (rand & 0x3FFFFFFFFFFFFFFF);

    char buf[37];
    std::snprintf(buf, sizeof(buf),
        "%08" PRIx32 "-%04" PRIx16 "-%04" PRIx16 "-%04" PRIx16 "-%012" PRIx64,
        static_cast<uint32_t>(hi >> 32),
        static_cast<uint16_t>(hi >> 16),
        static_cast<uint16_t>(hi),
        static_cast<uint16_t>(lo >> 48),
        static_cast<uint64_t>(lo & 0x0000FFFFFFFFFFFF));

    return buf;
}

} // namespace

std::string SystemUUIDProvider::generateV7(std::string_view prefix) {
    return std::string(prefix) + "_" + makeV7();
}

ProjectID    SystemUUIDProvider::newProjectID()    { return ProjectID   {generateV7("project")}; }
ManuscriptID SystemUUIDProvider::newManuscriptID() { return ManuscriptID{generateV7("manuscript")}; }
ChapterID    SystemUUIDProvider::newChapterID()    { return ChapterID   {generateV7("chapter")}; }
SceneID      SystemUUIDProvider::newSceneID()      { return SceneID     {generateV7("scene")}; }
IdentityID   SystemUUIDProvider::newIdentityID()   { return IdentityID  {generateV7("identity")}; }
PersonaID    SystemUUIDProvider::newPersonaID()    { return PersonaID   {generateV7("persona")}; }
SnapshotID   SystemUUIDProvider::newSnapshotID()   { return SnapshotID  {generateV7("snapshot")}; }
ObjectID     SystemUUIDProvider::newObjectID()     { return ObjectID    {generateV7("character")}; }

} // namespace scrivi::platform
