#include "platform/KeychainSecureStore.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

#include <string>
#include <utility>

namespace scrivi::platform {

namespace {

// Owns one CF reference and releases it — the Security API hands back +1 objects
// from every Create/Copy call, and a leak per identity read adds up over a session.
template <typename T>
class CFOwned {
public:
    explicit CFOwned(T ref = nullptr) : ref_(ref) {}
    ~CFOwned() { if (ref_ != nullptr) { CFRelease(ref_); } }
    CFOwned(const CFOwned&)            = delete;
    CFOwned& operator=(const CFOwned&) = delete;
    [[nodiscard]] T get() const { return ref_; }
    T* out() { return &ref_; }
private:
    T ref_;
};

CFStringRef makeString(std::string_view s)
{
    return CFStringCreateWithBytes(kCFAllocatorDefault,
                                   reinterpret_cast<const UInt8*>(s.data()),
                                   static_cast<CFIndex>(s.size()),
                                   kCFStringEncodingUTF8, false);
}

// The query that names ONE item: generic password, this service, this key.
CFMutableDictionaryRef makeItemQuery(const std::string& service, std::string_view key)
{
    CFMutableDictionaryRef q = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFOwned<CFStringRef> svc(makeString(service));
    CFOwned<CFStringRef> acct(makeString(key));
    CFDictionarySetValue(q, kSecClass, kSecClassGenericPassword);
    CFDictionarySetValue(q, kSecAttrService, svc.get());
    CFDictionarySetValue(q, kSecAttrAccount, acct.get());
    return q;
}

Error keychainError(const char* what, OSStatus status)
{
    std::string message = std::string(what) + " (OSStatus " + std::to_string(status) + ")";
    CFOwned<CFStringRef> text(SecCopyErrorMessageString(status, nullptr));
    if (text.get() != nullptr) {
        char buf[256] = {};
        if (CFStringGetCString(text.get(), buf, sizeof(buf), kCFStringEncodingUTF8)) {
            message += ": ";
            message += buf;
        }
    }
    return {.code = ErrorCode::secureStoreError, .message = std::move(message)};
}

} // namespace

KeychainSecureStore::KeychainSecureStore(std::string service)
    : service_(std::move(service))
{
}

Result<bool> KeychainSecureStore::containsSecret(std::string_view key)
{
    CFOwned<CFMutableDictionaryRef> q(makeItemQuery(service_, key));
    CFDictionarySetValue(q.get(), kSecMatchLimit, kSecMatchLimitOne);
    const OSStatus status = SecItemCopyMatching(q.get(), nullptr);
    if (status == errSecSuccess) {
        return Result<bool>::success(true);
    }
    if (status == errSecItemNotFound) {
        return Result<bool>::success(false);
    }
    // ⛔ ANY OTHER STATUS IS AN ERROR, NOT "ABSENT". ⚠️ A locked or unreachable
    // keychain answering `false` would make `ensureLocalIdentity` MINT A NEW IDENTITY
    // over the real one — the very defect this store exists to end ([I-0216]).
    // ✅ Absence is never inferred from a failed read.
    return Result<bool>::failure(keychainError("keychain lookup failed", status));
}

Result<void> KeychainSecureStore::putSecret(std::string_view key, const SecretBytes& value)
{
    CFOwned<CFDataRef> data(CFDataCreate(kCFAllocatorDefault,
                                         reinterpret_cast<const UInt8*>(value.data()),
                                         static_cast<CFIndex>(value.size())));

    // ✅ Update in place when the item exists; add it otherwise.
    CFOwned<CFMutableDictionaryRef> q(makeItemQuery(service_, key));
    CFOwned<CFMutableDictionaryRef> changes(CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    CFDictionarySetValue(changes.get(), kSecValueData, data.get());
    OSStatus status = SecItemUpdate(q.get(), changes.get());
    if (status == errSecSuccess) {
        return Result<void>::success();
    }
    if (status != errSecItemNotFound) {
        return Result<void>::failure(keychainError("keychain update failed", status));
    }

    CFOwned<CFMutableDictionaryRef> add(makeItemQuery(service_, key));
    CFDictionarySetValue(add.get(), kSecValueData, data.get());
    // ✅ Readable once the device has been unlocked since boot, and NOT `ThisDeviceOnly`
    // — so the identity migrates with the writer's account and encrypted backups.
    CFDictionarySetValue(add.get(), kSecAttrAccessible, kSecAttrAccessibleAfterFirstUnlock);
    status = SecItemAdd(add.get(), nullptr);
    if (status != errSecSuccess) {
        return Result<void>::failure(keychainError("keychain add failed", status));
    }
    return Result<void>::success();
}

Result<SecretBytes> KeychainSecureStore::getSecret(std::string_view key)
{
    CFOwned<CFMutableDictionaryRef> q(makeItemQuery(service_, key));
    CFDictionarySetValue(q.get(), kSecMatchLimit, kSecMatchLimitOne);
    CFDictionarySetValue(q.get(), kSecReturnData, kCFBooleanTrue);
    CFOwned<CFTypeRef> result;
    const OSStatus status = SecItemCopyMatching(q.get(), result.out());
    if (status != errSecSuccess) {
        return Result<SecretBytes>::failure(keychainError("keychain read failed", status));
    }
    if (result.get() == nullptr || CFGetTypeID(result.get()) != CFDataGetTypeID()) {
        return Result<SecretBytes>::failure(
            {.code = ErrorCode::secureStoreError, .message = "keychain returned no data"});
    }
    const auto dataRef = static_cast<CFDataRef>(result.get());
    const auto* bytes  = reinterpret_cast<const std::byte*>(CFDataGetBytePtr(dataRef));
    return Result<SecretBytes>::success(
        SecretBytes(bytes, bytes + CFDataGetLength(dataRef)));
}

Result<void> KeychainSecureStore::removeSecret(std::string_view key)
{
    CFOwned<CFMutableDictionaryRef> q(makeItemQuery(service_, key));
    const OSStatus status = SecItemDelete(q.get());
    if (status == errSecSuccess || status == errSecItemNotFound) {
        return Result<void>::success();
    }
    return Result<void>::failure(keychainError("keychain delete failed", status));
}

} // namespace scrivi::platform
