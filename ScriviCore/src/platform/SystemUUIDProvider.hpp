#pragma once

#include "scrivi/Services.hpp"

namespace scrivi::platform {

// Generates UUID v7-style IDs with Scrivi type prefixes.
// Format: <prefix>_<xxxxxxxx-xxxx-7xxx-yxxx-xxxxxxxxxxxx>
// where the timestamp occupies the high 48 bits and the remainder is random.
class SystemUUIDProvider final : public UUIDProvider {
public:
    ProjectID    newProjectID()    override;
    ManuscriptID newManuscriptID() override;
    ChapterID    newChapterID()    override;
    SceneID      newSceneID()      override;
    IdentityID   newIdentityID()   override;
    PersonaID    newPersonaID()    override;
    SnapshotID   newSnapshotID()   override;
    ObjectID     newObjectID()     override;

private:
    static std::string generateV7(std::string_view prefix);
};

} // namespace scrivi::platform
