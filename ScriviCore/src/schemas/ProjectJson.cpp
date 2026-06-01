#include "ProjectJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string serializeProject(const ProjectJsonData& d) {
    util::JsonDoc createdBy;
    createdBy.setString("identityID",            d.createdByIdentityID);
    createdBy.setString("personaID",             d.createdByPersonaID);
    createdBy.setString("displayNameAtCreation", d.createdByDisplayName);

    util::JsonDoc manuscript;
    manuscript.setString("path", d.manuscriptPath);

    util::JsonDoc identities;
    identities.setString("membersPath",  d.membersPath);
    identities.setString("personasPath", d.personasPath);

    util::JsonDoc gitSnapshots;
    gitSnapshots.setBool("enabled", d.gitSnapshotsEnabled);

    util::JsonDoc features;
    features.setSubDoc("gitSnapshots", std::move(gitSnapshots));

    util::JsonDoc doc;
    doc.setString("schema",    "scrivi.project.v1");
    doc.setString("projectID", d.projectID.value);
    doc.setString("title",     d.title);
    doc.setString("slug",      d.slug);
    doc.setString("createdAt", d.createdAt);
    doc.setSubDoc("createdBy",  std::move(createdBy));
    doc.setSubDoc("manuscript", std::move(manuscript));
    doc.setSubDoc("identities", std::move(identities));
    doc.setSubDoc("features",   std::move(features));

    return doc.dump();
}

Result<ProjectJsonData> parseProject(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.project.v1");
    if (!r.ok()) { return Result<ProjectJsonData>::failure(r.error()); }
    auto& doc = r.value();

    for (const auto* key : {"projectID", "title", "createdAt"}) {
        auto v = requireField(doc, key);
        if (!v.ok()) { return Result<ProjectJsonData>::failure(v.error()); }
    }

    ProjectJsonData data;
    data.projectID.value = doc.getString("projectID");
    data.title           = doc.getString("title");
    data.slug            = doc.getString("slug");
    data.createdAt       = doc.getString("createdAt");

    auto createdBy  = doc.getSubDoc("createdBy");
    data.createdByIdentityID  = createdBy.getString("identityID");
    data.createdByPersonaID   = createdBy.getString("personaID");
    data.createdByDisplayName = createdBy.getString("displayNameAtCreation");

    auto manuscript = doc.getSubDoc("manuscript");
    data.manuscriptPath = manuscript.getString("path");

    auto identities = doc.getSubDoc("identities");
    data.membersPath  = identities.getString("membersPath");
    data.personasPath = identities.getString("personasPath");

    auto features     = doc.getSubDoc("features");
    auto gitSnapshots = features.getSubDoc("gitSnapshots");
    data.gitSnapshotsEnabled = gitSnapshots.getBool("enabled");

    return Result<ProjectJsonData>::success(std::move(data));
}

} // namespace scrivi::schemas
