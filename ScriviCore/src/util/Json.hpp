#pragma once

#include "scrivi/Result.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

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

    // Member key names of this object (empty if not an object). Order is
    // implementation-defined; callers must not depend on it.
    [[nodiscard]] std::vector<std::string> objectKeys() const;

    [[nodiscard]] std::string getString(std::string_view key, std::string_view defaultValue = "") const;
    [[nodiscard]] bool        getBool(std::string_view key, bool defaultValue = false) const;
    [[nodiscard]] int         getInt(std::string_view key, int defaultValue = 0) const;
    [[nodiscard]] int64_t     getInt64(std::string_view key, int64_t defaultValue = 0) const;
    [[nodiscard]] double      getDouble(std::string_view key, double defaultValue = 0.0) const;

    void setString(std::string_view key, std::string_view value);
    void setBool(std::string_view key, bool value);
    void setInt(std::string_view key, int value);
    void setInt64(std::string_view key, int64_t value);
    void setDouble(std::string_view key, double value);

    // String array support
    void appendStringToArray(std::string_view key, std::string_view value);
    [[nodiscard]] std::vector<std::string> getStringArray(std::string_view key) const;

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
