#include "Json.hpp"

#include "scrivi/Error.hpp"

#include <nlohmann/json.hpp>

namespace scrivi::util {

struct JsonDoc::Impl {
    nlohmann::json data;
};

JsonDoc::JsonDoc() : impl_(new Impl{}) {}
JsonDoc::~JsonDoc() { delete impl_; }
JsonDoc::JsonDoc(JsonDoc&& other) noexcept : impl_(other.impl_) { other.impl_ = nullptr; }
JsonDoc& JsonDoc::operator=(JsonDoc&& other) noexcept {
    if (this != &other) { delete impl_; impl_ = other.impl_; other.impl_ = nullptr; }
    return *this;
}

bool JsonDoc::contains(std::string_view key) const {
    return impl_->data.contains(std::string(key));
}

std::vector<std::string> JsonDoc::objectKeys() const {
    std::vector<std::string> keys;
    if (impl_->data.is_object()) {
        for (auto it = impl_->data.begin(); it != impl_->data.end(); ++it) {
            keys.push_back(it.key());
        }
    }
    return keys;
}

std::string JsonDoc::getString(std::string_view key, std::string_view defaultValue) const {
    auto k = std::string(key);
    if (impl_->data.contains(k) && impl_->data[k].is_string()) {
        return impl_->data[k].get<std::string>();
    }
    return std::string(defaultValue);
}

bool JsonDoc::getBool(std::string_view key, bool defaultValue) const {
    auto k = std::string(key);
    if (impl_->data.contains(k) && impl_->data[k].is_boolean()) {
        return impl_->data[k].get<bool>();
    }
    return defaultValue;
}

int JsonDoc::getInt(std::string_view key, int defaultValue) const {
    auto k = std::string(key);
    if (impl_->data.contains(k) && impl_->data[k].is_number_integer()) {
        return impl_->data[k].get<int>();
    }
    return defaultValue;
}

double JsonDoc::getDouble(std::string_view key, double defaultValue) const {
    auto k = std::string(key);
    if (impl_->data.contains(k) && impl_->data[k].is_number()) {
        return impl_->data[k].get<double>();
    }
    return defaultValue;
}

void JsonDoc::setString(std::string_view key, std::string_view value) {
    impl_->data[std::string(key)] = std::string(value);
}

void JsonDoc::setBool(std::string_view key, bool value) {
    impl_->data[std::string(key)] = value;
}

void JsonDoc::setInt(std::string_view key, int value) {
    impl_->data[std::string(key)] = value;
}

void JsonDoc::setInt64(std::string_view key, int64_t value) {
    impl_->data[std::string(key)] = value;
}

int64_t JsonDoc::getInt64(std::string_view key, int64_t defaultValue) const {
    auto k = std::string(key);
    if (impl_->data.contains(k) && impl_->data[k].is_number_integer()) {
        return impl_->data[k].get<int64_t>();
    }
    return defaultValue;
}

void JsonDoc::setDouble(std::string_view key, double value) {
    impl_->data[std::string(key)] = value;
}

void JsonDoc::appendStringToArray(std::string_view key, std::string_view value) {
    auto k = std::string(key);
    if (!impl_->data.contains(k) || !impl_->data[k].is_array()) {
        impl_->data[k] = nlohmann::json::array();
    }
    impl_->data[k].push_back(std::string(value));
}

std::vector<std::string> JsonDoc::getStringArray(std::string_view key) const {
    std::vector<std::string> result;
    auto k = std::string(key);
    if (impl_->data.contains(k) && impl_->data[k].is_array()) {
        for (const auto& elem : impl_->data[k]) {
            if (elem.is_string()) {
                result.push_back(elem.get<std::string>());
            }
        }
    }
    return result;
}

void JsonDoc::setSubDoc(std::string_view key, JsonDoc sub) {
    impl_->data[std::string(key)] = std::move(sub.impl_->data);
}

JsonDoc JsonDoc::getSubDoc(std::string_view key) const {
    JsonDoc sub;
    auto k = std::string(key);
    if (impl_->data.contains(k) && impl_->data[k].is_object()) {
        sub.impl_->data = impl_->data[k];
    }
    return sub;
}

void JsonDoc::appendToArray(std::string_view key, JsonDoc item) {
    auto k = std::string(key);
    if (!impl_->data.contains(k) || !impl_->data[k].is_array()) {
        impl_->data[k] = nlohmann::json::array();
    }
    impl_->data[k].push_back(std::move(item.impl_->data));
}

std::size_t JsonDoc::arraySize(std::string_view key) const {
    auto k = std::string(key);
    if (impl_->data.contains(k) && impl_->data[k].is_array()) {
        return impl_->data[k].size();
    }
    return 0;
}

JsonDoc JsonDoc::arrayItem(std::string_view key, std::size_t i) const {
    JsonDoc item;
    auto k = std::string(key);
    if (impl_->data.contains(k) && impl_->data[k].is_array() && i < impl_->data[k].size()) {
        item.impl_->data = impl_->data[k][i];
    }
    return item;
}

std::string JsonDoc::dump(int indent) const {
    return impl_->data.dump(indent);
}

Result<JsonDoc> parseJson(std::string_view utf8) {
    try {
        JsonDoc doc;
        doc.impl_->data = nlohmann::json::parse(utf8);
        return Result<JsonDoc>::success(std::move(doc));
    } catch (const nlohmann::json::parse_error& e) {
        return Result<JsonDoc>::failure({.code = ErrorCode::parseError, .message = e.what()});
    }
}

} // namespace scrivi::util
