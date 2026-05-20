#include "ProjectMembersJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string serializeProjectMembers(const ProjectMembersData& d) {
    util::JsonDoc doc;
    doc.setString("schema", "scrivi.projectMembers.v1");

    for (auto& m : d.members) {
        util::JsonDoc entry;
        entry.setString("identityID",      m.identityID.value);
        entry.setString("role",            m.role);
        entry.setString("status",          m.status);
        entry.setString("defaultPersonaID",m.defaultPersonaID);
        entry.setString("joinedAt",        m.joinedAt);
        doc.appendToArray("members", std::move(entry));
    }

    return doc.dump();
}

Result<ProjectMembersData> parseProjectMembers(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.projectMembers.v1");
    if (!r.ok()) return Result<ProjectMembersData>::failure(r.error());
    auto& doc = r.value();

    auto v = requireField(doc, "members");
    if (!v.ok()) return Result<ProjectMembersData>::failure(v.error());

    ProjectMembersData data;
    auto n = doc.arraySize("members");
    for (std::size_t i = 0; i < n; ++i) {
        auto item = doc.arrayItem("members", i);
        MemberEntry m;
        m.identityID.value = item.getString("identityID");
        m.role             = item.getString("role");
        m.status           = item.getString("status");
        m.defaultPersonaID = item.getString("defaultPersonaID");
        m.joinedAt         = item.getString("joinedAt");
        data.members.push_back(std::move(m));
    }

    return Result<ProjectMembersData>::success(std::move(data));
}

} // namespace scrivi::schemas
