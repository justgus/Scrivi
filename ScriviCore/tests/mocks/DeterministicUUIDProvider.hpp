#pragma once

#include "scrivi/Services.hpp"

#include <string>

namespace scrivi::mocks {

// Returns IDs in the form "<prefix>-001", "<prefix>-002", etc.
// Each ID type has its own prefix and counter, so IDs are stable and readable in fixtures.
class DeterministicUUIDProvider final : public UUIDProvider {
public:
    ProjectID    newProjectID()    override { return ProjectID   {"proj-"    + next(proj_)}; }
    ManuscriptID newManuscriptID() override { return ManuscriptID{"ms-"      + next(ms_)}; }
    ChapterID    newChapterID()    override { return ChapterID   {"ch-"      + next(ch_)}; }
    SceneID      newSceneID()      override { return SceneID     {"scene-"   + next(scene_)}; }
    IdentityID   newIdentityID()   override { return IdentityID  {"identity-"+ next(identity_)}; }
    PersonaID    newPersonaID()    override { return PersonaID   {"persona-" + next(persona_)}; }
    SnapshotID   newSnapshotID()   override { return SnapshotID  {"snap-"    + next(snap_)}; }
    ObjectID     newObjectID()     override { return ObjectID    {"obj-"     + next(obj_)}; }

private:
    int proj_ = 0, ms_ = 0, ch_ = 0, scene_ = 0, identity_ = 0, persona_ = 0, snap_ = 0, obj_ = 0;

    static std::string next(int& counter) {
        ++counter;
        char buf[8];
        std::snprintf(buf, sizeof(buf), "%03d", counter);
        return buf;
    }
};

} // namespace scrivi::mocks
