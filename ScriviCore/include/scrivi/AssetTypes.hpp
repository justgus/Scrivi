#pragma once

#include "scrivi/Types.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace scrivi {

enum class AssetCategory : std::uint8_t { image, audio, video, document, other };

inline std::string assetCategorySubdir(AssetCategory cat) {
    switch (cat) {
        case AssetCategory::image:    return "images";
        case AssetCategory::audio:    return "audio";
        case AssetCategory::video:    return "video";
        case AssetCategory::document: return "documents";
        case AssetCategory::other:    return "other";
    }
    return "other";
}

inline std::string assetCategoryString(AssetCategory cat) {
    switch (cat) {
        case AssetCategory::image:    return "image";
        case AssetCategory::audio:    return "audio";
        case AssetCategory::video:    return "video";
        case AssetCategory::document: return "document";
        case AssetCategory::other:    return "other";
    }
    return "other";
}

inline AssetCategory assetCategoryFromString(const std::string& s) {
    if (s == "image")    { return AssetCategory::image; }
    if (s == "audio")    { return AssetCategory::audio; }
    if (s == "video")    { return AssetCategory::video; }
    if (s == "document") { return AssetCategory::document; }
    return AssetCategory::other;
}

struct AssetMeta {
    std::string      assetID;
    Slug             slug;
    std::string      filename;     // basename only, e.g. "map.png"
    AssetCategory    category = AssetCategory::other;
    std::string      mimeType;
    ISO8601Timestamp importedAt;

    std::string importedByIdentityID;
    std::string importedByPersonaID;
    std::string importedByDisplayName;

    std::string title;
    std::string notes;
    std::vector<std::string> tags;
};

} // namespace scrivi
