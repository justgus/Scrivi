#pragma once

#include "scrivi/Services.hpp"
#include "scrivi/Result.hpp"

#include <string_view>

namespace scrivi::apple {

// macOS Keychain implementation of scrivi::SecureStore.
// Secrets are stored as kSecClassGenericPassword items under
// service "com.scrivi.core". Survives process restart.
class KeychainSecureStore final : public scrivi::SecureStore {
public:
    scrivi::Result<bool>             containsSecret(std::string_view key) override;
    scrivi::Result<void>             putSecret(std::string_view key, const scrivi::SecretBytes& value) override;
    scrivi::Result<scrivi::SecretBytes> getSecret(std::string_view key) override;
};

} // namespace scrivi::apple
