#pragma once

#include "scrivi/Result.hpp"

#include <string>
#include <string_view>

// Opaque JSON document handle — nlohmann types do not appear in this header.
namespace scrivi::util {

class JsonDoc {
public:
    JsonDoc();
    ~JsonDoc();
    JsonDoc(JsonDoc&&) noexcept;
    JsonDoc& operator=(JsonDoc&&) noexcept;

    JsonDoc(const JsonDoc&) = delete;
    JsonDoc& operator=(const JsonDoc&) = delete;

    [[nodiscard]] bool contains(std::string_view key) const;

    [[nodiscard]] std::string getString(std::string_view key, std::string_view defaultValue = "") const;
    [[nodiscard]] bool        getBool(std::string_view key, bool defaultValue = false) const;
    [[nodiscard]] int         getInt(std::string_view key, int defaultValue = 0) const;
    [[nodiscard]] double      getDouble(std::string_view key, double defaultValue = 0.0) const;

    void setString(std::string_view key, std::string_view value);
    void setBool(std::string_view key, bool value);
    void setInt(std::string_view key, int value);
    void setDouble(std::string_view key, double value);

    // Embeds a nested JsonDoc as a sub-object under key.
    void setSubDoc(std::string_view key, JsonDoc sub);

    // Returns a sub-object as a JsonDoc, or an empty doc if missing/not-object.
    [[nodiscard]] JsonDoc getSubDoc(std::string_view key) const;

    // Array support: appends a sub-object to an array field (creates array if absent).
    void appendToArray(std::string_view key, JsonDoc item);

    // Returns the number of elements in an array field (0 if missing or not an array).
    [[nodiscard]] std::size_t arraySize(std::string_view key) const;

    // Returns element i of an array field as a JsonDoc.
    [[nodiscard]] JsonDoc arrayItem(std::string_view key, std::size_t i) const;

    [[nodiscard]] std::string dump(int indent = 2) const;

private:
    struct Impl;
    Impl* impl_ = nullptr;

    friend Result<JsonDoc> parseJson(std::string_view utf8);
};

Result<JsonDoc> parseJson(std::string_view utf8);

} // namespace scrivi::util
