#pragma once

#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <string>

namespace scrivi::platform {

// KeychainSecureStore — the persistent SecureStore for Apple platforms ([I-0216],
// SP-151). ✅ RULED 2026-09-30: the Keychain, written in C++ against
// Security.framework's C API — ⛔ no Swift, per "no backend logic in Swift".
//
// ⚠️ WHY NOT `EncryptedFileSecureStore` (Linux's). Rejected for Apple on three counts:
// it needs OpenSSL, which Apple does not ship; it derives its key from
// `/etc/machine-id`, which macOS does not have; and that machine-bound key fails HARD
// after Migration Assistant or a Time Machine restore — `ensureLocalIdentity` returns
// an error rather than minting, so the writer could not start at all.
// ✅ The Keychain encrypts, isolates per user, and migrates with the user's account.
//
// ⚠️ BEFORE THIS, Apple had NO persistent store: `makeSecureStore()` fell through to
// an in-memory map, so a NEW identity was minted on EVERY launch and each session
// attributed work to a different author ([I-0216]).
//
// Each secret is one generic-password item: service = `service`, account = the key.
//
// ⚠️ macOS uses the LOGIN (file-based) keychain, ⛔ NOT the data-protection keychain:
// the latter needs a provisioning entitlement the app does not carry, and fails with
// errSecMissingEntitlement (-34018). ✅ iOS/visionOS have only the data-protection
// keychain, and every app there is entitled to its own items.
class KeychainSecureStore final : public SecureStore {
public:
    // ✅ The production service name. ⚠️ Tests pass their OWN, so a test run can never
    // read, overwrite or delete the writer's real identity.
    static constexpr const char* kDefaultService = "com.caposoft.scrivi.securestore";

    explicit KeychainSecureStore(std::string service = kDefaultService);

    Result<bool>        containsSecret(std::string_view key) override;
    Result<void>        putSecret(std::string_view key, const SecretBytes& value) override;
    Result<SecretBytes> getSecret(std::string_view key) override;

    // ⛔ TEST SUPPORT ONLY — removes one item. Not part of `SecureStore`, and never
    // called by the app: nothing in Scrivi deletes an identity.
    Result<void> removeSecret(std::string_view key);

private:
    std::string service_;
};

} // namespace scrivi::platform
