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
    Result<void> createFileExclusive(const AbsolutePath& path, std::string_view utf8Text) override;
    Result<void> appendTextFile(const AbsolutePath& path, std::string_view utf8Text) override;
    Result<std::vector<AbsolutePath>> listDirectory(const AbsolutePath& path) override;
    Result<void> removeFile(const AbsolutePath& path) override;
    Result<void> renamePath(const AbsolutePath& from, const AbsolutePath& to) override;
    Result<void> copyFileInBlocks(const AbsolutePath& from,
                                  const AbsolutePath& to,
                                  std::size_t blockSize,
                                  const std::function<Result<void>()>& onBlock) override;
};

} // namespace scrivi::platform
