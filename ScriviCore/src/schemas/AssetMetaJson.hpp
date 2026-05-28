#pragma once

#include "scrivi/AssetTypes.hpp"
#include "scrivi/Result.hpp"

#include <string>
#include <string_view>

namespace scrivi::schemas {

std::string serializeAssetMeta(const AssetMeta& meta);
Result<AssetMeta> parseAssetMeta(std::string_view json);

} // namespace scrivi::schemas
