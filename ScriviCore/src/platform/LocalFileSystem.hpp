#pragma once

#include "scrivi/Services.hpp"

namespace scrivi::platform {

class LocalFileSystem final : public FileSystem {
public:
    Result<bool> exists(const AbsolutePath& path) override;
    Result<bool> isDirectory(const AbsolutePath& path) override;
    Result<void> createDirectories(const AbsolutePath& path) override;
    Result<Utf8Text> readTextFile(const AbsolutePath& path) override;
    Result<void> atomicWriteTextFile(const AbsolutePath& path, std::string_view utf8Text) override;
    Result<void> appendTextFile(const AbsolutePath& path, std::string_view utf8Text) override;
    Result<std::vector<AbsolutePath>> listDirectory(const AbsolutePath& path) override;
    Result<void> removeFile(const AbsolutePath& path) override;
};

} // namespace scrivi::platform
