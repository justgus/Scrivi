#pragma once

#include "scrivi/Error.hpp"
#include <cassert>
#include <optional>
#include <utility>

namespace scrivi {

template <typename T>
class Result {
public:
    static Result success(T value) {
        Result r;
        r.value_ = std::move(value);
        return r;
    }

    static Result failure(Error error) {
        Result r;
        r.error_ = std::move(error);
        return r;
    }

    bool ok() const { return value_.has_value(); }

    const T& value() const {
        assert(ok());
        return *value_;
    }

    T& value() {
        assert(ok());
        return *value_;
    }

    const Error& error() const {
        assert(!ok());
        return *error_;
    }

private:
    std::optional<T> value_;
    std::optional<Error> error_;
};

template <>
class Result<void> {
public:
    static Result success() { return Result(true, {}); }

    static Result failure(Error error) {
        return Result(false, std::move(error));
    }

    bool ok() const { return ok_; }

    const Error& error() const {
        assert(!ok_);
        return error_;
    }

private:
    Result(bool ok, Error error) : ok_(ok), error_(std::move(error)) {}

    bool ok_ = false;
    Error error_;
};

} // namespace scrivi
