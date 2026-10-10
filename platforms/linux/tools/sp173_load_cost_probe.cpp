// SP-173 Plan 6 / I-0285 AC3 — what the Linux load's two big phases spend: the core open and the per-scene body reads,
// plus the timeline read, each TIMED separately. Under strace (`sp173-load-cost-probe.sh`), a failed openat of
// "/scrivi-sp173-phase/<name>" marks each phase boundary in the trace, so every file call can be attributed to its phase.
// The same three C ABI calls, in the same order, as `EditorShell::load` makes them.
#include <scrivi/scrivi.h>

#include <chrono>
#include <cstdio>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <vector>

static std::vector<std::string> scenesOf(const std::string& e, std::string& pid) {
    std::vector<std::string> ids;
    // Only inside the "scenes" ARRAY (the envelope's keys are sorted, so "activeScene" — whose sceneID must not be counted
    // twice — comes BEFORE it): from its '[' to the matching ']'.
    size_t p = e.find('[', e.find("\"scenes\""));
    size_t end = p;
    for (int depth = 0; end < e.size(); ++end) {
        if (e[end] == '[') { ++depth; } else if (e[end] == ']' && --depth == 0) { break; }
    }
    while ((p = e.find("\"sceneID\"", p)) != std::string::npos && p < end) {
        size_t c = e.find(':', p), q1 = e.find('"', c), q2 = e.find('"', q1 + 1);
        ids.push_back(e.substr(q1 + 1, q2 - q1 - 1)); p = q2;
    }
    size_t pp = e.find("\"projectID\""), c = e.find(':', pp), q1 = e.find('"', c), q2 = e.find('"', q1 + 1);
    pid = e.substr(q1 + 1, q2 - q1 - 1);
    return ids;
}

static void mark(const char* phase) {
    const std::string path = std::string("/scrivi-sp173-phase/") + phase;
    const int fd = ::open(path.c_str(), O_RDONLY);   // fails by design: a marker in the trace
    if (fd >= 0) { ::close(fd); }
}

int main(int argc, char** argv) {
    if (argc < 3) { std::fprintf(stderr, "usage: %s <projectRoot> <appSupportRoot> [--batch]\n", argv[0]); return 2; }
    const char* root = argv[1]; const char* asr = argv[2];
    // --batch: the bodies as the apps read them since SP-173 fix 3 — chunks of 64 through scrivi_read_scene_texts.
    const bool batch = argc > 3 && std::string(argv[3]) == "--batch";
    using clk = std::chrono::steady_clock;
    auto ms = [](clk::time_point a, clk::time_point b) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
    };

    mark("open");
    const auto t0 = clk::now();
    const char* l = scrivi_open_project(root, asr, "identity-001");
    std::string le(l ? l : ""); scrivi_free(l);
    const auto t1 = clk::now();
    std::string pid; auto ids = scenesOf(le, pid);

    mark("bodies");
    if (batch) {
        for (size_t start = 0; start < ids.size(); start += 64) {
            std::string json = "[";
            for (size_t i = start; i < ids.size() && i < start + 64; ++i) { json += (i > start ? ",\"" : "\"") + ids[i] + "\""; }
            json += "]";
            scrivi_free(scrivi_read_scene_texts(root, json.c_str()));
        }
    } else {
        for (auto& id : ids) {
            scrivi_free(scrivi_open_scene_for_bulk_load(root, asr, pid.c_str(), id.c_str()));
        }
    }
    const auto t2 = clk::now();

    mark("timeline");
    scrivi_free(scrivi_load_timeline(root));
    const auto t3 = clk::now();
    mark("end");

    std::fprintf(stderr, "%s  scenes=%zu  open=%lld ms  bodies=%lld ms (%.1f ms/scene)  timeline=%lld ms\n",
                 batch ? "[batch]    " : "[per-scene]", ids.size(), (long long)ms(t0, t1), (long long)ms(t1, t2),
                 ids.empty() ? 0.0 : double(ms(t1, t2)) / double(ids.size()), (long long)ms(t2, t3));
    return 0;
}
