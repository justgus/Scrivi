#pragma once

#include "scrivi/RepairIssue.hpp"
#include "scrivi/Result.hpp"
#include "util/Json.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace scrivi::schemas {

// Serialize a list of RepairIssues (with suggestedActions) to a JSON string.
std::string serializeRepairIssues(const std::vector<RepairIssue>& issues);

// Parse a JSON string back into a list of RepairIssues.
// Returns parseError on malformed JSON or validationError on schema mismatch.
Result<std::vector<RepairIssue>> parseRepairIssues(std::string_view json);

// Append each RepairIssue as a sub-object into an existing JsonDoc array.
// Use this to embed issues directly into a scan/open result envelope.
void appendRepairIssuesToDoc(util::JsonDoc& doc,
                              std::string_view arrayKey,
                              const std::vector<RepairIssue>& issues);

} // namespace scrivi::schemas
