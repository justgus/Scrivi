#pragma once

#include "scrivi/CommentTypes.hpp"
#include "scrivi/Result.hpp"

#include <string>
#include <string_view>

namespace scrivi::schemas {

std::string serializeCommentThread(const CommentThread& thread);
Result<CommentThread> parseCommentThread(std::string_view json);

} // namespace scrivi::schemas
