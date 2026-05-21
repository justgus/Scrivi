#include "identity/IdentityService.hpp"

#include "util/Json.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>

namespace scrivi::identity {

namespace {

// Serialize identity bundle to JSON text, then wrap as SecretBytes.
SecretBytes bundleToBytes(const std::string& json) {
    SecretBytes bytes(json.size());
    std::memcpy(bytes.data(), json.data(), json.size());
    return bytes;
}

std::string bytesToString(const SecretBytes& bytes) {
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

// Generate a 32-byte random secret as a hex string (placeholder key material).
// Uses a non-deterministic engine; this is forward-compatible with a real key
// format once a cryptographic design is approved.
std::string generateSecretHex() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned> dist(0, 255);

    char buf[65];
    for (int i = 0; i < 32; ++i)
        std::snprintf(buf + i * 2, 3, "%02x", dist(gen));
    buf[64] = '\0';
    return buf;
}

// Derive a stable synthetic device ID from std::random_device (one-time, stored
// in the bundle so it is consistent across calls on the same device).
std::string generateDeviceID() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dist;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "device-%016llx", dist(gen));
    return buf;
}

} // namespace

IdentityService::IdentityService(CoreServices& services)
    : services_(services) {}

Result<EnsureIdentityResult> IdentityService::ensureLocalIdentity(
    const EnsureIdentityRequest& request)
{
    // --- Check for existing identity ---
    auto containsR = services_.secureStore->containsSecret(kIdentityKey);
    if (!containsR.ok())
        return Result<EnsureIdentityResult>::failure(containsR.error());

    if (containsR.value()) {
        // Load and return existing identity.
        auto getBytesR = services_.secureStore->getSecret(kIdentityKey);
        if (!getBytesR.ok())
            return Result<EnsureIdentityResult>::failure(getBytesR.error());

        auto parseR = util::parseJson(bytesToString(getBytesR.value()));
        if (!parseR.ok())
            return Result<EnsureIdentityResult>::failure(
                {ErrorCode::secureStoreError, "identity bundle corrupt"});

        auto& doc = parseR.value();
        EnsureIdentityResult result;
        result.identityID.value       = doc.getString("identityID");
        result.defaultPersonaID.value = doc.getString("personaID");
        result.displayName            = doc.getString("displayName");
        result.createdNewIdentity     = false;
        return Result<EnsureIdentityResult>::success(std::move(result));
    }

    // --- Create new identity ---
    auto identityID = services_.uuidProvider->newIdentityID();
    auto personaID  = services_.uuidProvider->newPersonaID();

    // Build and store bundle atomically — if putSecret fails, nothing is persisted.
    util::JsonDoc doc;
    doc.setString("identityID",   identityID.value);
    doc.setString("personaID",    personaID.value);
    doc.setString("displayName",  request.requestedDisplayName);
    doc.setString("deviceID",     generateDeviceID());
    doc.setString("secretMaterial", generateSecretHex());

    auto putR = services_.secureStore->putSecret(
        kIdentityKey, bundleToBytes(doc.dump(0)));
    if (!putR.ok())
        return Result<EnsureIdentityResult>::failure(putR.error());

    EnsureIdentityResult result;
    result.identityID         = identityID;
    result.defaultPersonaID   = personaID;
    result.displayName        = request.requestedDisplayName;
    result.createdNewIdentity = true;
    return Result<EnsureIdentityResult>::success(std::move(result));
}

} // namespace scrivi::identity
