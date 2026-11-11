#pragma once

// EP-058 S1 (SP-209): the ordered authors of a project OR a world, and which of them an identity writes as.
// Design: docs/Scrivi_Authorship_Design_v0_2.md §3, §4.1, §4.4. An author is a persona in the package
// (`identities/{project|world}-personas.json`); writing as is the member's `defaultPersonaID`
// (`identities/{project|world}-members.json`). A persona is never deleted — sidecars, objects and comments refer to it by ID.

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "schemas/ProjectMembersJson.hpp"
#include "schemas/ProjectPersonasJson.hpp"

#include <optional>
#include <string>
#include <vector>

namespace scrivi::identity {

struct Author {
    PersonaID   personaID;
    std::string displayName;
    bool        unnamed = false;   // credited only to "Scrivi" or this identity's recorded name (SP-209 Q1)
};

struct AuthorsView {
    std::vector<Author>   authors;     // active, in byline order
    std::optional<Author> writingAs;   // this identity's author, when it is a member writing as an active author
    std::string           prompt;      // "none" | "nameAuthor" | "chooseAuthor" (SP-209 Q3)
};

struct AuthorEdit {
    std::string personaID;   // empty: a new author
    std::string displayName;
};

// Q1: the names the code ever wrote as an author before a writer could set one — "Scrivi" (Linux) and the identity's
// recorded name (Apple: the host name at mint). The core learns the second from scrivi_ensure_local_identity, so no open
// reads the SecureStore.
void rememberIdentityName(const std::string& name);
std::vector<std::string> unnamedNames();

class AuthorStore {
public:
    AuthorStore(const CoreServices& services, AbsolutePath packageRoot);

    // A world package without `identities/` has no authors yet (Q2 seeds it).
    Result<bool> hasAuthorFiles();
    Result<AuthorsView> read(const IdentityID& identity);
    Result<void> setAuthors(const std::vector<AuthorEdit>& edits);
    Result<void> setWritingAs(const IdentityID& identity, const PersonaID& persona);
    // Q2: a world's first author is the opening project's writing-as author for this identity.
    Result<void> seed(const IdentityID& identity, const Author& author);

private:
    Result<bool> isWorld();
    Result<std::string> path(const char* which);   // "personas" | "members"
    Result<schemas::ProjectPersonasData> readPersonas();
    Result<schemas::ProjectMembersData> readMembers();
    Result<void> write(const schemas::ProjectPersonasData& personas, const schemas::ProjectMembersData& members);

    CoreServices services_;
    AbsolutePath root_;
};

} // namespace scrivi::identity
