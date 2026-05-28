#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::comments {

// Handles addComment / listComments / resolveComment.
// Thread files: comments/<scopeKind>/<targetID>.comments.json
class CommentStore {
public:
    explicit CommentStore(CoreServices& services);

    Result<AddCommentResult>     add(const AddCommentRequest& request);
    Result<ListCommentsResult>   list(const ListCommentsRequest& request);
    Result<ResolveCommentResult> resolve(const ResolveCommentRequest& request);

private:
    CoreServices& services_;

    AbsolutePath threadPath(const AbsolutePath& projectRoot,
                             const std::string& scopeKind,
                             const std::string& targetID) const;
};

} // namespace scrivi::comments
