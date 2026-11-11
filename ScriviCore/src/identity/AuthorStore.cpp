#include "AuthorStore.hpp"

#include "util/PathUtils.hpp"

#include <algorithm>
#include <mutex>

namespace scrivi::identity {

namespace {

std::mutex& nameMutex() { static std::mutex m; return m; }
std::string& recordedName() { static std::string s; return s; }

template <typename T> Result<T> fail(ErrorCode code, std::string message) {
    return Result<T>::failure({.code = code, .message = std::move(message)});
}

std::string trimmed(const std::string& s) {
    const auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) { return {}; }
    return s.substr(b, s.find_last_not_of(" \t\r\n") - b + 1);
}

bool contains(const std::vector<std::string>& v, const std::string& x) { return std::ranges::find(v, x) != v.end(); }

} // namespace

void rememberIdentityName(const std::string& name) {
    std::lock_guard lock(nameMutex());
    recordedName() = name;
}

std::vector<std::string> unnamedNames() {
    std::lock_guard lock(nameMutex());
    std::vector<std::string> names{"Scrivi"};
    if (!recordedName().empty() && recordedName() != "Scrivi") { names.push_back(recordedName()); }
    return names;
}

AuthorStore::AuthorStore(const CoreServices& services, AbsolutePath packageRoot)
    : services_(services), root_(std::move(packageRoot)) {}

Result<bool> AuthorStore::isWorld() {
    auto project = services_.fileSystem->exists(util::join(root_, "project.json"));
    if (!project.ok()) { return Result<bool>::failure(project.error()); }
    if (project.value()) { return Result<bool>::success(false); }
    auto world = services_.fileSystem->exists(util::join(root_, "world.json"));
    if (!world.ok()) { return Result<bool>::failure(world.error()); }
    if (world.value()) { return Result<bool>::success(true); }
    return fail<bool>(ErrorCode::invalidArgument, "not a project or world package: " + root_);
}

Result<std::string> AuthorStore::path(const char* which) {
    auto world = isWorld();
    if (!world.ok()) { return Result<std::string>::failure(world.error()); }
    return Result<std::string>::success(
        util::join(root_, std::string("identities/") + (world.value() ? "world-" : "project-") + which + ".json"));
}

Result<bool> AuthorStore::hasAuthorFiles() {
    auto p = path("personas");
    if (!p.ok()) { return Result<bool>::failure(p.error()); }
    return services_.fileSystem->exists(p.value());
}

Result<schemas::ProjectPersonasData> AuthorStore::readPersonas() {
    auto p = path("personas");
    if (!p.ok()) { return Result<schemas::ProjectPersonasData>::failure(p.error()); }
    auto there = services_.fileSystem->exists(p.value());
    if (!there.ok()) { return Result<schemas::ProjectPersonasData>::failure(there.error()); }
    if (!there.value()) { return Result<schemas::ProjectPersonasData>::success({}); }
    auto text = services_.fileSystem->readTextFile(p.value());
    if (!text.ok()) { return Result<schemas::ProjectPersonasData>::failure(text.error()); }
    return schemas::parseProjectPersonas(text.value());
}

Result<schemas::ProjectMembersData> AuthorStore::readMembers() {
    auto p = path("members");
    if (!p.ok()) { return Result<schemas::ProjectMembersData>::failure(p.error()); }
    auto there = services_.fileSystem->exists(p.value());
    if (!there.ok()) { return Result<schemas::ProjectMembersData>::failure(there.error()); }
    if (!there.value()) { return Result<schemas::ProjectMembersData>::success({}); }
    auto text = services_.fileSystem->readTextFile(p.value());
    if (!text.ok()) { return Result<schemas::ProjectMembersData>::failure(text.error()); }
    return schemas::parseProjectMembers(text.value());
}

Result<void> AuthorStore::write(const schemas::ProjectPersonasData& personas, const schemas::ProjectMembersData& members) {
    auto pp = path("personas");
    auto mp = path("members");
    if (!pp.ok()) { return Result<void>::failure(pp.error()); }
    if (!mp.ok()) { return Result<void>::failure(mp.error()); }
    if (auto r = services_.fileSystem->createDirectories(util::join(root_, "identities")); !r.ok()) { return r; }
    // Personas first: a member never names a persona that is not on disk.
    if (auto r = services_.fileSystem->atomicWriteTextFile(pp.value(), schemas::serializeProjectPersonas(personas)); !r.ok()) {
        return r;
    }
    return services_.fileSystem->atomicWriteTextFile(mp.value(), schemas::serializeProjectMembers(members));
}

Result<AuthorsView> AuthorStore::read(const IdentityID& identity) {
    auto personas = readPersonas();
    if (!personas.ok()) { return Result<AuthorsView>::failure(personas.error()); }
    auto members = readMembers();
    if (!members.ok()) { return Result<AuthorsView>::failure(members.error()); }

    const auto unnamed = unnamedNames();
    AuthorsView view;
    for (const auto& p : personas.value().personas) {
        if (p.status != "active") { continue; }
        view.authors.push_back({p.personaID, p.displayName, contains(unnamed, p.displayName)});
    }
    for (const auto& m : members.value().members) {
        if (identity.value.empty() || m.identityID.value != identity.value || m.status != "active") { continue; }
        for (const auto& a : view.authors) {
            if (a.personaID.value == m.defaultPersonaID) { view.writingAs = a; }
        }
    }
    // Q3: naming the work comes first (every author unnamed, or none at all); then who this identity is.
    const bool allUnnamed = std::ranges::all_of(view.authors, [](const Author& a) { return a.unnamed; });
    if (allUnnamed) {
        view.prompt = "nameAuthor";
    } else if (!identity.value.empty() && !view.writingAs) {
        view.prompt = "chooseAuthor";
    } else {
        view.prompt = "none";
    }
    return Result<AuthorsView>::success(std::move(view));
}

Result<void> AuthorStore::setAuthors(const std::vector<AuthorEdit>& edits) {
    if (edits.empty()) { return fail<void>(ErrorCode::validationError, "a work needs at least one author"); }
    auto personas = readPersonas();
    if (!personas.ok()) { return Result<void>::failure(personas.error()); }
    auto members = readMembers();
    if (!members.ok()) { return Result<void>::failure(members.error()); }

    auto old = std::move(personas.value().personas);
    schemas::ProjectPersonasData next;
    std::vector<std::string> kept;
    for (const auto& e : edits) {
        const std::string name = trimmed(e.displayName);
        if (name.empty()) { return fail<void>(ErrorCode::validationError, "an author's name is empty"); }
        if (e.personaID.empty()) {
            next.personas.push_back({.personaID = services_.uuidProvider->newPersonaID(), .displayName = name,
                                     .personaKind = "individual", .controlledBy = {},
                                     .createdAt = services_.clock->nowUTC(), .status = "active"});
            continue;
        }
        if (contains(kept, e.personaID)) {
            return fail<void>(ErrorCode::validationError, "an author is listed twice: " + e.personaID);
        }
        auto it = std::ranges::find_if(old, [&](const auto& p) { return p.personaID.value == e.personaID; });
        if (it == old.end()) { return fail<void>(ErrorCode::validationError, "unknown author: " + e.personaID); }
        auto p = *it;
        p.displayName = name;   // a rename keeps the ID
        p.status = "active";    // a retired author may return
        next.personas.push_back(std::move(p));
        kept.push_back(e.personaID);
    }
    // An author left out is retired, never deleted: sidecars, objects and comments refer to it by ID.
    for (auto& p : old) {
        if (contains(kept, p.personaID.value)) { continue; }
        p.status = "retired";
        next.personas.push_back(std::move(p));
    }
    return write(next, members.value());
}

Result<void> AuthorStore::setWritingAs(const IdentityID& identity, const PersonaID& persona) {
    if (identity.value.empty()) { return fail<void>(ErrorCode::invalidArgument, "identityID is required"); }
    auto personas = readPersonas();
    if (!personas.ok()) { return Result<void>::failure(personas.error()); }
    auto members = readMembers();
    if (!members.ok()) { return Result<void>::failure(members.error()); }

    auto& list = personas.value().personas;
    auto it = std::ranges::find_if(list, [&](const auto& p) { return p.personaID.value == persona.value; });
    if (it == list.end() || it->status != "active") {
        return fail<void>(ErrorCode::validationError, "not an active author: " + persona.value);
    }
    if (!contains(it->controlledBy, identity.value)) { it->controlledBy.push_back(identity.value); }

    auto& ms = members.value().members;
    auto m = std::ranges::find_if(ms, [&](const auto& x) { return x.identityID.value == identity.value; });
    if (m == ms.end()) {
        ms.push_back({.identityID = identity, .role = "member", .status = "active", .defaultPersonaID = persona.value,
                      .joinedAt = services_.clock->nowUTC()});
    } else {
        m->status = "active";
        m->defaultPersonaID = persona.value;
    }
    return write(personas.value(), members.value());
}

Result<void> AuthorStore::seed(const IdentityID& identity, const Author& author) {
    const auto now = services_.clock->nowUTC();
    schemas::ProjectPersonasData personas;
    personas.personas.push_back({.personaID = author.personaID, .displayName = author.displayName,
                                 .personaKind = "individual", .controlledBy = {identity.value}, .createdAt = now,
                                 .status = "active"});
    schemas::ProjectMembersData members;
    members.members.push_back({.identityID = identity, .role = "owner", .status = "active",
                               .defaultPersonaID = author.personaID.value, .joinedAt = now});
    return write(personas, members);
}

} // namespace scrivi::identity
