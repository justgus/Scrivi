// EncryptedFileSecureStore tests (SP-059 / AC4, T-0058 Option L-2).
//
// Linux-only: the store is compiled only where SCRIVI_HAS_ENCRYPTED_SECURE_STORE
// is defined (see ScriviCore/CMakeLists.txt). Guarded so the macOS ctest job,
// which does not build the store, still compiles this TU to nothing.
#if defined(SCRIVI_HAS_ENCRYPTED_SECURE_STORE)

#include <catch2/catch_test_macros.hpp>

#include "platform/EncryptedFileSecureStore.hpp"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace {

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-securestore-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    std::string str() const { return path.string(); }
};

scrivi::SecretBytes bytesOf(std::string_view s) {
    scrivi::SecretBytes b(s.size());
    std::memcpy(b.data(), s.data(), s.size());
    return b;
}

std::string stringOf(const scrivi::SecretBytes& b) {
    return {reinterpret_cast<const char*>(b.data()), b.size()};
}

} // namespace

TEST_CASE("EncryptedFileSecureStore - round-trips a secret within one instance",
          "[integration][securestore][SP-059]") {
    TempDir dir;
    scrivi::platform::EncryptedFileSecureStore store(dir.str());

    const auto secret = bytesOf("scrivi.identity.v1 bundle payload");
    REQUIRE(store.putSecret("scrivi.identity.v1", secret).ok());

    auto contains = store.containsSecret("scrivi.identity.v1");
    REQUIRE(contains.ok());
    CHECK(contains.value());

    auto got = store.getSecret("scrivi.identity.v1");
    REQUIRE(got.ok());
    CHECK(stringOf(got.value()) == "scrivi.identity.v1 bundle payload");
}

TEST_CASE("EncryptedFileSecureStore - secret survives a fresh store instance "
          "(persistence)", "[integration][securestore][SP-059]") {
    TempDir dir;
    const auto secret = bytesOf("persisted across instances");

    {
        scrivi::platform::EncryptedFileSecureStore writer(dir.str());
        REQUIRE(writer.putSecret("scrivi.identity.v1", secret).ok());
    }

    // A brand-new store over the same dir simulates an app restart: it must read
    // the previously-written secret back (this is the AC4 guarantee).
    scrivi::platform::EncryptedFileSecureStore reader(dir.str());
    auto contains = reader.containsSecret("scrivi.identity.v1");
    REQUIRE(contains.ok());
    CHECK(contains.value());

    auto got = reader.getSecret("scrivi.identity.v1");
    REQUIRE(got.ok());
    CHECK(stringOf(got.value()) == "persisted across instances");
}

TEST_CASE("EncryptedFileSecureStore - containsSecret false for absent key; "
          "getSecret fails", "[integration][securestore][SP-059]") {
    TempDir dir;
    scrivi::platform::EncryptedFileSecureStore store(dir.str());

    auto contains = store.containsSecret("nope");
    REQUIRE(contains.ok());
    CHECK_FALSE(contains.value());

    auto got = store.getSecret("nope");
    CHECK_FALSE(got.ok());
    CHECK(got.error().code == scrivi::ErrorCode::secureStoreError);
}

TEST_CASE("EncryptedFileSecureStore - stored file is not plaintext",
          "[integration][securestore][SP-059]") {
    TempDir dir;
    scrivi::platform::EncryptedFileSecureStore store(dir.str());

    const std::string plain = "TOP-SECRET-MARKER-1234567890";
    REQUIRE(store.putSecret("scrivi.identity.v1", bytesOf(plain)).ok());

    // The on-disk .enc file must not contain the plaintext marker.
    const fs::path encFile = fs::path(dir.str()) / "scrivi.identity.v1.enc";
    REQUIRE(fs::exists(encFile));
    std::ifstream in(encFile, std::ios::binary);
    const std::string ondisk((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
    CHECK(ondisk.find(plain) == std::string::npos);
}

TEST_CASE("EncryptedFileSecureStore - a tampered ciphertext fails authentication",
          "[integration][securestore][SP-059]") {
    TempDir dir;
    scrivi::platform::EncryptedFileSecureStore store(dir.str());
    REQUIRE(store.putSecret("scrivi.identity.v1", bytesOf("payload")).ok());

    // Flip a byte in the ciphertext region (past the 12-byte nonce + 16-byte tag).
    const fs::path encFile = fs::path(dir.str()) / "scrivi.identity.v1.enc";
    std::string blob;
    {
        std::ifstream in(encFile, std::ios::binary);
        blob.assign((std::istreambuf_iterator<char>(in)),
                    std::istreambuf_iterator<char>());
    }
    REQUIRE(blob.size() > 28);
    blob[blob.size() - 1] ^= 0x01;
    {
        std::ofstream out(encFile, std::ios::binary | std::ios::trunc);
        out.write(blob.data(), static_cast<std::streamsize>(blob.size()));
    }

    // GCM auth must reject the tampered file rather than returning garbage.
    auto got = store.getSecret("scrivi.identity.v1");
    CHECK_FALSE(got.ok());
    CHECK(got.error().code == scrivi::ErrorCode::secureStoreError);
}

#endif // SCRIVI_HAS_ENCRYPTED_SECURE_STORE
