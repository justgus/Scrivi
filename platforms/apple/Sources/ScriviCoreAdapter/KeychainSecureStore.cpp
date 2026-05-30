#include "KeychainSecureStore.hpp"

#include <Security/Security.h>

#include <string>
#include <vector>

namespace scrivi::apple {

static scrivi::Result<scrivi::SecretBytes> keychainGet(std::string_view key) {
    auto keyStr = CFStringCreateWithBytes(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8*>(key.data()),
        static_cast<CFIndex>(key.size()),
        kCFStringEncodingUTF8,
        false);

    const void* queryKeys[]   = { kSecClass, kSecAttrService, kSecAttrAccount, kSecReturnData, kSecMatchLimit };
    const void* queryValues[] = { kSecClassGenericPassword, CFSTR("com.scrivi.core"), keyStr, kCFBooleanTrue, kSecMatchLimitOne };
    auto query = CFDictionaryCreate(kCFAllocatorDefault, queryKeys, queryValues, 5,
                                    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFTypeRef result = nullptr;
    OSStatus status = SecItemCopyMatching(query, &result);
    CFRelease(query);
    CFRelease(keyStr);

    if (status == errSecItemNotFound) {
        return scrivi::Result<scrivi::SecretBytes>::failure(
            {scrivi::ErrorCode::secureStoreError, "key not found"});
    }
    if (status != errSecSuccess) {
        return scrivi::Result<scrivi::SecretBytes>::failure(
            {scrivi::ErrorCode::secureStoreError, "SecItemCopyMatching failed: " + std::to_string(status)});
    }

    auto data = static_cast<CFDataRef>(result);
    const auto* ptr = CFDataGetBytePtr(data);
    auto len = static_cast<std::size_t>(CFDataGetLength(data));

    scrivi::SecretBytes bytes(len);
    for (std::size_t i = 0; i < len; ++i)
        bytes[i] = static_cast<std::byte>(ptr[i]);

    CFRelease(result);
    return scrivi::Result<scrivi::SecretBytes>::success(std::move(bytes));
}

scrivi::Result<bool> KeychainSecureStore::containsSecret(std::string_view key) {
    auto r = keychainGet(key);
    return scrivi::Result<bool>::success(r.ok());
}

scrivi::Result<void> KeychainSecureStore::putSecret(
        std::string_view key, const scrivi::SecretBytes& value) {
    auto keyStr = CFStringCreateWithBytes(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8*>(key.data()),
        static_cast<CFIndex>(key.size()),
        kCFStringEncodingUTF8,
        false);

    auto cfData = CFDataCreate(
        kCFAllocatorDefault,
        reinterpret_cast<const UInt8*>(value.data()),
        static_cast<CFIndex>(value.size()));

    // Try update first.
    const void* queryKeys[]   = { kSecClass, kSecAttrService, kSecAttrAccount };
    const void* queryValues[] = { kSecClassGenericPassword, CFSTR("com.scrivi.core"), keyStr };
    auto query = CFDictionaryCreate(kCFAllocatorDefault, queryKeys, queryValues, 3,
                                    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    const void* updateKeys[]   = { kSecValueData };
    const void* updateValues[] = { cfData };
    auto attrs = CFDictionaryCreate(kCFAllocatorDefault, updateKeys, updateValues, 1,
                                    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    OSStatus status = SecItemUpdate(query, attrs);
    CFRelease(query);
    CFRelease(attrs);

    if (status == errSecItemNotFound) {
        // Item doesn't exist — add it.
        const void* addKeys[]   = { kSecClass, kSecAttrService, kSecAttrAccount, kSecValueData };
        const void* addValues[] = { kSecClassGenericPassword, CFSTR("com.scrivi.core"), keyStr, cfData };
        auto addQuery = CFDictionaryCreate(kCFAllocatorDefault, addKeys, addValues, 4,
                                          &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        status = SecItemAdd(addQuery, nullptr);
        CFRelease(addQuery);
    }

    CFRelease(cfData);
    CFRelease(keyStr);

    if (status != errSecSuccess) {
        return scrivi::Result<void>::failure(
            {scrivi::ErrorCode::secureStoreError, "Keychain write failed: " + std::to_string(status)});
    }
    return scrivi::Result<void>::success();
}

scrivi::Result<scrivi::SecretBytes> KeychainSecureStore::getSecret(std::string_view key) {
    return keychainGet(key);
}

} // namespace scrivi::apple
