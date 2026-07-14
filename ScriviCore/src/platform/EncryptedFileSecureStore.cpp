#include "platform/EncryptedFileSecureStore.hpp"

#include "scrivi/Error.hpp"

#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/params.h>
#include <openssl/rand.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>
#include <pwd.h>

namespace scrivi::platform {

namespace {

namespace fs = std::filesystem;

constexpr std::size_t kKeyLen   = 32;   // AES-256
constexpr std::size_t kNonceLen = 12;   // GCM standard nonce
constexpr std::size_t kTagLen   = 16;   // GCM tag
constexpr std::size_t kSaltLen  = 16;

Error ioErr(std::string msg) {
    return Error{.code = ErrorCode::secureStoreError, .message = std::move(msg)};
}

// Reads an entire file into a byte string. Returns false if the file is absent
// or unreadable.
bool readAll(const fs::path& p, std::string& out) {
    std::ifstream in(p, std::ios::binary);
    if (!in) return false;
    out.assign((std::istreambuf_iterator<char>(in)),
               std::istreambuf_iterator<char>());
    return true;
}

// Atomically writes bytes to a file (write temp + rename) so a crash mid-write
// never leaves a truncated secret/salt.
bool writeAtomic(const fs::path& p, const std::string& bytes) {
    const fs::path tmp = fs::path(p).concat(".tmp");
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        if (!out) return false;
    }
    std::error_code ec;
    fs::rename(tmp, p, ec);
    if (ec) { fs::remove(tmp, ec); return false; }
    return true;
}

std::string readMachineId() {
    std::string id;
    if (readAll("/etc/machine-id", id) && !id.empty()) return id;
    // Some systems only have /var/lib/dbus/machine-id.
    if (readAll("/var/lib/dbus/machine-id", id) && !id.empty()) return id;
    return "no-machine-id";   // stable within a host even if the file is absent
}

std::string currentUsername() {
    if (struct passwd* pw = getpwuid(getuid())) {
        if (pw->pw_name) return pw->pw_name;
    }
    return "unknown";
}

} // namespace

EncryptedFileSecureStore::EncryptedFileSecureStore(std::string secureDir)
    : secureDir_(std::move(secureDir)) {}

std::string EncryptedFileSecureStore::fileFor(std::string_view key) const {
    // Sanitize the key into a filename: keep [A-Za-z0-9._-], map anything else to
    // '_'. Scrivi's keys ("scrivi.identity.v1") are already filename-safe.
    std::string name;
    name.reserve(key.size());
    for (char c : key) {
        const bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                        (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
        name.push_back(ok ? c : '_');
    }
    return (fs::path(secureDir_) / (name + ".enc")).string();
}

Result<std::string> EncryptedFileSecureStore::deriveKey() const {
    std::error_code ec;
    fs::create_directories(secureDir_, ec);
    if (ec) return Result<std::string>::failure(
        ioErr("cannot create secure dir: " + ec.message()));

    // Load or generate the persistent salt.
    const fs::path saltPath = fs::path(secureDir_) / "store.salt";
    std::string salt;
    if (!readAll(saltPath, salt) || salt.size() != kSaltLen) {
        salt.resize(kSaltLen);
        if (RAND_bytes(reinterpret_cast<unsigned char*>(salt.data()),
                       static_cast<int>(kSaltLen)) != 1) {
            return Result<std::string>::failure(ioErr("RAND_bytes(salt) failed"));
        }
        if (!writeAtomic(saltPath, salt)) {
            return Result<std::string>::failure(ioErr("cannot persist salt"));
        }
    }

    // Input keying material: machine-id + uid + username (belt-and-suspenders).
    const std::string ikm = readMachineId() + "|" +
                            std::to_string(getuid()) + "|" +
                            currentUsername();

    // HKDF-SHA256(ikm, salt, info) → 32-byte AES key, via the EVP_KDF API.
    EVP_KDF* kdf = EVP_KDF_fetch(nullptr, "HKDF", nullptr);
    if (!kdf) return Result<std::string>::failure(ioErr("EVP_KDF_fetch(HKDF) failed"));
    EVP_KDF_CTX* kctx = EVP_KDF_CTX_new(kdf);
    EVP_KDF_free(kdf);
    if (!kctx) return Result<std::string>::failure(ioErr("EVP_KDF_CTX_new failed"));

    static const char kInfo[] = "scrivi.securestore.v1";
    std::array<OSSL_PARAM, 5> params{};
    std::size_t i = 0;
    params[i++] = OSSL_PARAM_construct_utf8_string(
        OSSL_KDF_PARAM_DIGEST, const_cast<char*>("SHA256"), 0);
    params[i++] = OSSL_PARAM_construct_octet_string(
        OSSL_KDF_PARAM_KEY, const_cast<char*>(ikm.data()), ikm.size());
    params[i++] = OSSL_PARAM_construct_octet_string(
        OSSL_KDF_PARAM_SALT, salt.data(), salt.size());
    params[i++] = OSSL_PARAM_construct_octet_string(
        OSSL_KDF_PARAM_INFO, const_cast<char*>(kInfo), sizeof(kInfo) - 1);
    params[i] = OSSL_PARAM_construct_end();

    std::string key(kKeyLen, '\0');
    const int rc = EVP_KDF_derive(kctx,
        reinterpret_cast<unsigned char*>(key.data()), kKeyLen, params.data());
    EVP_KDF_CTX_free(kctx);
    if (rc != 1) return Result<std::string>::failure(ioErr("EVP_KDF_derive failed"));

    return Result<std::string>::success(std::move(key));
}

Result<bool> EncryptedFileSecureStore::containsSecret(std::string_view key) {
    std::error_code ec;
    const bool exists = fs::exists(fileFor(key), ec);
    return Result<bool>::success(exists && !ec);
}

Result<void> EncryptedFileSecureStore::putSecret(std::string_view key,
                                                 const SecretBytes& value) {
    auto keyR = deriveKey();
    if (!keyR.ok()) return Result<void>::failure(keyR.error());
    const std::string& aesKey = keyR.value();

    // Fresh random nonce per write (GCM must never reuse a nonce under one key).
    std::array<unsigned char, kNonceLen> nonce{};
    if (RAND_bytes(nonce.data(), static_cast<int>(kNonceLen)) != 1) {
        return Result<void>::failure(ioErr("RAND_bytes(nonce) failed"));
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return Result<void>::failure(ioErr("EVP_CIPHER_CTX_new failed"));

    auto fail = [&](const char* m) {
        EVP_CIPHER_CTX_free(ctx);
        return Result<void>::failure(ioErr(m));
    };

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
        return fail("EncryptInit(aes-256-gcm) failed");
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, kNonceLen, nullptr) != 1)
        return fail("set IV len failed");
    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr,
            reinterpret_cast<const unsigned char*>(aesKey.data()), nonce.data()) != 1)
        return fail("EncryptInit key/iv failed");

    std::string cipher(value.size(), '\0');
    int outLen = 0;
    if (EVP_EncryptUpdate(ctx,
            reinterpret_cast<unsigned char*>(cipher.data()), &outLen,
            reinterpret_cast<const unsigned char*>(value.data()),
            static_cast<int>(value.size())) != 1)
        return fail("EncryptUpdate failed");
    int total = outLen;
    if (EVP_EncryptFinal_ex(ctx,
            reinterpret_cast<unsigned char*>(cipher.data()) + total, &outLen) != 1)
        return fail("EncryptFinal failed");
    total += outLen;
    cipher.resize(static_cast<std::size_t>(total));

    std::array<unsigned char, kTagLen> tag{};
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, kTagLen, tag.data()) != 1)
        return fail("get GCM tag failed");
    EVP_CIPHER_CTX_free(ctx);

    // File layout: [nonce (12)] [tag (16)] [ciphertext].
    std::string blob;
    blob.reserve(kNonceLen + kTagLen + cipher.size());
    blob.append(reinterpret_cast<const char*>(nonce.data()), kNonceLen);
    blob.append(reinterpret_cast<const char*>(tag.data()), kTagLen);
    blob.append(cipher);

    if (!writeAtomic(fileFor(key), blob))
        return Result<void>::failure(ioErr("cannot write secret file"));
    return Result<void>::success();
}

Result<SecretBytes> EncryptedFileSecureStore::getSecret(std::string_view key) {
    std::string blob;
    if (!readAll(fileFor(key), blob))
        return Result<SecretBytes>::failure(ioErr("key not found"));
    if (blob.size() < kNonceLen + kTagLen)
        return Result<SecretBytes>::failure(ioErr("secret file truncated"));

    auto keyR = deriveKey();
    if (!keyR.ok()) return Result<SecretBytes>::failure(keyR.error());
    const std::string& aesKey = keyR.value();

    const auto* nonce = reinterpret_cast<const unsigned char*>(blob.data());
    const auto* tag   = nonce + kNonceLen;
    const char* cipher = blob.data() + kNonceLen + kTagLen;
    const std::size_t cipherLen = blob.size() - kNonceLen - kTagLen;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return Result<SecretBytes>::failure(ioErr("EVP_CIPHER_CTX_new failed"));

    auto fail = [&](const char* m) {
        EVP_CIPHER_CTX_free(ctx);
        return Result<SecretBytes>::failure(ioErr(m));
    };

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
        return fail("DecryptInit(aes-256-gcm) failed");
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, kNonceLen, nullptr) != 1)
        return fail("set IV len failed");
    if (EVP_DecryptInit_ex(ctx, nullptr, nullptr,
            reinterpret_cast<const unsigned char*>(aesKey.data()), nonce) != 1)
        return fail("DecryptInit key/iv failed");

    std::string plain(cipherLen, '\0');
    int outLen = 0;
    if (EVP_DecryptUpdate(ctx,
            reinterpret_cast<unsigned char*>(plain.data()), &outLen,
            reinterpret_cast<const unsigned char*>(cipher),
            static_cast<int>(cipherLen)) != 1)
        return fail("DecryptUpdate failed");
    int total = outLen;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, kTagLen,
            const_cast<unsigned char*>(tag)) != 1)
        return fail("set GCM tag failed");

    // Non-positive DecryptFinal => auth failed (tampered file or wrong key,
    // e.g. machine-id changed). Surface as a store error so the caller can
    // re-enroll rather than trusting garbage.
    if (EVP_DecryptFinal_ex(ctx,
            reinterpret_cast<unsigned char*>(plain.data()) + total, &outLen) <= 0)
        return fail("secret authentication failed (tampered or re-imaged host)");
    total += outLen;
    EVP_CIPHER_CTX_free(ctx);
    plain.resize(static_cast<std::size_t>(total));

    SecretBytes out(plain.size());
    std::memcpy(out.data(), plain.data(), plain.size());
    return Result<SecretBytes>::success(std::move(out));
}

} // namespace scrivi::platform
