#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/IDs.hpp"

#include <string>
#include <vector>

namespace scrivi::schemas {

struct SnapshotEntryData {
    std::string snapshotID;
    std::string commitID;
    std::string label;
    std::string note;
    std::string createdAt;
    std::string createdByIdentityID;
    std::string createdByPersonaID;
    std::string createdByDisplayName;
};

struct SnapshotMetadataData {
    std::vector<SnapshotEntryData> snapshots;
};

std::string serializeSnapshotMetadata(const SnapshotMetadataData& data);
Result<SnapshotMetadataData> parseSnapshotMetadata(std::string_view json);

} // namespace scrivi::schemas
