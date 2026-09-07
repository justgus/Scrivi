// scrivi_world_probe.cpp — T-0477 (EP-038 / SP-124) INSTRUMENTATION ONLY.
//
// ⚠️ SHIPS NO PRODUCT LOGIC AND MAKES NO DECISION. It prints, verbatim, what
// the C ABI reports about a project's worlds — so a drive-loss run can record
// THE CORE'S VERDICT at each phase instead of inferring it from OS signals.
//
//   scrivi_world_status_probe <projectRoot> [worldID]
//
// ⚠️ WHY THIS EXISTS. SP-124 §4's probe table names "what
// `scrivi_get_world_status` returns at each phase" and calls the core's verdict
// THE INPUT TO T-0478 — and no artifact captured it. Without it, S3 measures
// what LINUX reports and INFERS what SCRIVI concludes, which is exactly the
// substitution EP-038 exists to prevent.
//
// ⚠️ DELIBERATELY Qt-FREE. It links only ScriviCore, so it builds on a rig that
// has no Qt configured and cannot be broken by the app's build state. The
// question it answers is a CORE question.
//
// ⚠️ ENVELOPES ARE PRINTED RAW, NOT PARSED. A probe that interprets the JSON
// would decide what matters before the finding is known; the run needs the
// bytes. Diff the BEFORE and AFTER captures — the diff is the finding.

#include "scrivi/scrivi.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

// Prints an envelope and frees it. `scrivi_free` is required for every
// `const char*` the ABI returns.
void emit(const char* label, const char* envelope) {
    std::printf("### %s\n", label);
    if (envelope == nullptr) {
        // ⚠️ A NULL return is itself a finding — record it rather than crashing.
        std::printf("(NULL returned — the ABI produced no envelope)\n\n");
        return;
    }
    std::printf("%s\n\n", envelope);
    scrivi_free(envelope);
}

// Pulls the `worldID` values out of a scrivi_list_worlds envelope WITHOUT a
// JSON dependency: this tool must build against nothing but ScriviCore, and a
// probe is not the place to introduce a parser. Scanning for the key is
// sufficient to enumerate ids for the per-world calls; the RAW envelope is
// printed regardless, so nothing depends on this being clever.
std::vector<std::string> scrapeWorldIDs(const char* json) {
    std::vector<std::string> ids;
    if (json == nullptr) { return ids; }

    const std::string s(json);
    const std::string key = "\"worldID\"";
    std::size_t pos = 0;

    while ((pos = s.find(key, pos)) != std::string::npos) {
        pos = s.find(':', pos + key.size());
        if (pos == std::string::npos) { break; }
        const std::size_t open = s.find('"', pos);
        if (open == std::string::npos) { break; }
        const std::size_t close = s.find('"', open + 1);
        if (close == std::string::npos) { break; }
        ids.push_back(s.substr(open + 1, close - open - 1));
        pos = close + 1;
    }
    return ids;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr,
            "usage: %s <projectRoot> [worldID]\n\n"
            "  projectRoot  a .scrivi project directory\n"
            "  worldID      optional; omitted = every world the project binds\n\n"
            "Prints raw scrivi_list_worlds / scrivi_get_world_status envelopes.\n"
            "Run it BEFORE and AFTER a volume event and diff the output.\n",
            argv[0]);
        return 2;
    }

    const char* projectRoot = argv[1];
    const char* onlyWorld   = (argc >= 3) ? argv[2] : nullptr;

    std::printf("# scrivi world-status probe (T-0477)\n");
    std::printf("# project : %s\n", projectRoot);
    std::printf("# worldID : %s\n", onlyWorld ? onlyWorld : "<all bound worlds>");
    std::printf("\n");

    // 1. The project's worlds. Captured even when a single worldID was named,
    //    because the LIST is where a world silently disappearing would show.
    const char* listRaw = scrivi_list_worlds(projectRoot);
    const std::string listCopy = (listRaw != nullptr) ? listRaw : "";
    emit("scrivi_list_worlds", listRaw);

    // 2. Per-world status — the actual verdict T-0478 consumes.
    std::vector<std::string> ids;
    if (onlyWorld != nullptr) {
        ids.emplace_back(onlyWorld);
    } else {
        ids = scrapeWorldIDs(listCopy.c_str());
    }

    if (ids.empty()) {
        std::printf("### scrivi_get_world_status\n");
        std::printf("(no worldIDs found — the project binds no worlds, or the\n"
                    " list envelope reported an error above)\n\n");
        return 0;
    }

    for (const auto& id : ids) {
        const std::string label = "scrivi_get_world_status — " + id;
        emit(label.c_str(), scrivi_get_world_status(projectRoot, id.c_str()));
    }

    return 0;
}
