#pragma once

#include "scrivi/Services.hpp"

#include <map>
#include <string>

namespace scrivi::mocks {

class MockSecureStore final : public SecureStore {
public:
    Result<bool> containsSecret(std::string_view key) override {
        return Result<bool>::success(store_.count(std::string(key)) > 0);
    }

    Result<void> putSecret(std::string_view key, const SecretBytes& value) override {
        store_[std::string(key)] = value;
        return Result<void>::success();
    }

    Result<SecretBytes> getSecret(std::string_view key) override {
        auto it = store_.find(std::string(key));
        if (it == store_.end())
            return Result<SecretBytes>::failure({ErrorCode::secureStoreError, "key not found"});
        return Result<SecretBytes>::success(it->second);
    }

private:
    std::map<std::string, SecretBytes> store_;
};

} // namespace scrivi::mocks
