#pragma once

#include "scrivi/Error.hpp"
#include "scrivi/Result.hpp"
#include "util/Json.hpp"

#include <string>
#include <string_view>

namespace scrivi::schemas {

// Returns validationError if the field is missing from doc.
inline Result<void> requireField(const util::JsonDoc& doc, std::string_view key) {
    if (!doc.contains(key))
        return Result<void>::failure({ErrorCode::validationError,
            std::string("missing required field: ") + std::string(key)});
    return Result<void>::success();
}

// Parses JSON and validates the schema tag matches expectedSchema.
inline Result<util::JsonDoc> parseAndValidateSchema(std::string_view json,
                                                     std::string_view expectedSchema) {
    auto result = util::parseJson(json);
    if (!result.ok()) return Result<util::JsonDoc>::failure(result.error());
    auto doc = std::move(result.value());
    auto schema = doc.getString("schema");
    if (schema != expectedSchema)
        return Result<util::JsonDoc>::failure({ErrorCode::validationError,
            "unexpected schema: " + schema});
    return Result<util::JsonDoc>::success(std::move(doc));
}

} // namespace scrivi::schemas
