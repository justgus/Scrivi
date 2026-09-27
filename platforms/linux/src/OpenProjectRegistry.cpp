#include "OpenProjectRegistry.hpp"

#include "ProjectSession.hpp"

bool OpenProjectRegistry::registerSession(ProjectSession* session)
{
    if (session == nullptr) {
        return false;
    }
    // ⚠️ A failed load has no identity. ⛔ Registering it under an empty key would
    // make the NEXT failed load look like the same already-open project, and R3
    // would then refuse to open anything after one failure.
    const QString projectID = session->projectID();
    if (projectID.isEmpty()) {
        return false;
    }
    sessions_.insert(projectID, session);
    return true;
}
