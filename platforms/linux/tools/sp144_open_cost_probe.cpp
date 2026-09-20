// sp144_open_cost_probe.cpp — SP-144 AC6's measurement harness.
//
// ⚠️ Drives the SHIPPED C ABI exactly as the app's landing→editor chain does, in
// either variant, so the two are comparable on the SAME project:
//
//   --old : landing openProject + editor openProject + N x scrivi_open_scene
//           (the pre-SP-144 behaviour: DOUBLE OPEN + one workspace-state write
//            per scene)
//   (none): landing openProject, handed over + N x scrivi_open_scene_for_bulk_load
//           (the shipped behaviour: ONE open, ONE workspace write)
//
// ⚠️ It is deliberately Qt-free so it can be built and run on the rig without the
// GUI, and so a measurement is never confounded by widget work.
//
// ⛔ It does NOT test the UI. Progress bars, responsiveness and the drive-pull
// assertions are a human pass; this produces only the syscall + wall-clock
// figures AC6 requires, which a human cannot count by eye.
//
// Driven by sp144-open-cost-probe.sh — run that, not this.

// SP-144 — the LANDING + EDITOR chain as the app now performs it.
//   --old : landing openProject + editor openProject + 61x open_scene   (before)
//   --new : landing openProject (handed over)      + 61x bulk_load      (after)
#include <scrivi/scrivi.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
static std::vector<std::string> scenesOf(const std::string& e, std::string& pid) {
    std::vector<std::string> ids; size_t p = 0;
    while ((p = e.find("\"sceneID\"", p)) != std::string::npos) {
        size_t c=e.find(':',p), q1=e.find('"',c), q2=e.find('"',q1+1);
        ids.push_back(e.substr(q1+1,q2-q1-1)); p=q2;
    }
    size_t pp=e.find("\"projectID\""), c=e.find(':',pp), q1=e.find('"',c), q2=e.find('"',q1+1);
    pid = e.substr(q1+1,q2-q1-1);
    return ids;
}
int main(int argc, char** argv) {
    const char* root = argv[1]; const char* asr = argv[2];
    const bool oldWay = (argc>3 && std::strcmp(argv[3],"--old")==0);
    // The LANDING open — both variants pay this.
    const char* l = scrivi_open_project(root, asr, "identity-001");
    std::string le(l?l:""); scrivi_free(l);
    std::string pid; auto ids = scenesOf(le, pid);
    if (oldWay) {
        // The editor opened it AGAIN.
        const char* s = scrivi_open_project(root, asr, "identity-001");
        std::string se(s?s:""); scrivi_free(s);
        ids = scenesOf(se, pid);
    }
    for (auto& id : ids) {
        const char* s = oldWay
            ? scrivi_open_scene(root, asr, pid.c_str(), id.c_str())
            : scrivi_open_scene_for_bulk_load(root, asr, pid.c_str(), id.c_str());
        scrivi_free(s);
    }
    std::fprintf(stderr, "scenes=%zu old=%d\n", ids.size(), (int)oldWay);
    return 0;
}
