#include "CommentJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

static constexpr std::string_view kCommentSchema = "scrivi.comments.v1";

std::string serializeCommentThread(const CommentThread& thread) {
    util::JsonDoc root;
    root.setString("schema",    std::string(kCommentSchema));
    root.setString("scopeKind", thread.scopeKind);
    root.setString("targetID",  thread.targetID);

    for (const auto& c : thread.comments) {
        util::JsonDoc commentDoc;
        commentDoc.setString("commentID", c.commentID);
        commentDoc.setString("body",      c.body);
        commentDoc.setBool("resolved",    c.resolved);
        commentDoc.setString("createdAt", c.createdAt);

        util::JsonDoc createdBy;
        createdBy.setString("identityID",   c.createdByIdentityID);
        createdBy.setString("personaID",    c.createdByPersonaID);
        createdBy.setString("displayName",  c.createdByDisplayName);
        commentDoc.setSubDoc("createdBy", std::move(createdBy));

        commentDoc.setString("resolvedAt",            c.resolvedAt.value_or(""));
        commentDoc.setString("resolvedByIdentityID",  c.resolvedByIdentityID.value_or(""));
        commentDoc.setString("resolvedByPersonaID",   c.resolvedByPersonaID.value_or(""));
        commentDoc.setString("resolvedByDisplayName", c.resolvedByDisplayName.value_or(""));

        root.appendToArray("comments", std::move(commentDoc));
    }

    return root.dump();
}

Result<CommentThread> parseCommentThread(std::string_view json) {
    auto r = parseAndValidateSchema(json, kCommentSchema);
    if (!r.ok()) { return Result<CommentThread>::failure(r.error()); }
    auto& doc = r.value();

    CommentThread thread;
    thread.schema    = std::string(kCommentSchema);
    thread.scopeKind = doc.getString("scopeKind");
    thread.targetID  = doc.getString("targetID");

    const auto count = doc.arraySize("comments");
    thread.comments.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        auto commentDoc = doc.arrayItem("comments", i);
        Comment c;
        c.commentID = commentDoc.getString("commentID");
        c.body      = commentDoc.getString("body");
        c.resolved  = commentDoc.getBool("resolved");
        c.createdAt = commentDoc.getString("createdAt");

        auto createdBy = commentDoc.getSubDoc("createdBy");
        c.createdByIdentityID   = createdBy.getString("identityID");
        c.createdByPersonaID    = createdBy.getString("personaID");
        c.createdByDisplayName  = createdBy.getString("displayName");

        auto ra = commentDoc.getString("resolvedAt");
        if (!ra.empty()) { c.resolvedAt = ra; }

        auto rid = commentDoc.getString("resolvedByIdentityID");
        if (!rid.empty()) { c.resolvedByIdentityID = rid; }

        auto rpid = commentDoc.getString("resolvedByPersonaID");
        if (!rpid.empty()) { c.resolvedByPersonaID = rpid; }

        auto rdn = commentDoc.getString("resolvedByDisplayName");
        if (!rdn.empty()) { c.resolvedByDisplayName = rdn; }

        thread.comments.push_back(std::move(c));
    }

    return Result<CommentThread>::success(std::move(thread));
}

} // namespace scrivi::schemas
