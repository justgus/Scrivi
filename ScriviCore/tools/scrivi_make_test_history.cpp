// scrivi_make_test_history.cpp — FIXTURE BUILDER for [I-0331]'s long-history checks.
//
// ⚠️ NOT A TEST. It asserts nothing. It copies a project and drives a LONG undo history into the copy through the same
// scrivi_* calls the app makes, so the history's open, tree and undo can be measured against a history a writer could
// really have: typing across many scenes, deletes, replaces, pastes and cuts, undo runs that branch, redo, barriers,
// many sessions, and more events than the capacity so the oldest are evicted.
//
//   scrivi_make_test_history <sourceProject> <destProject> <appSupportRoot> [events] [sessions]
//
// ⚠️ It REFUSES to touch an existing destination and only READS the source. NEVER point the destination at real writing
// work (I-0150). The manuscript text in the copy changes: each session's edits are saved, as autosave would.
// ⚠️ Every event carries today's time: the core reads its own clock, so stale-branch age and idle rollover are not
// exercised.

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using scrivi::util::JsonDoc;

// A scrivi_* result, parsed and freed. A failed call stops the run: a fixture built past an error is not the fixture.
// Where the time went, per call (the time from the previous call to this one: the call itself, plus a little tool work).
std::map<std::string, std::pair<double, int>> gCost;
auto gLast = std::chrono::steady_clock::now();
JsonDoc call(const char* what, const char* raw)
{
    { auto now = std::chrono::steady_clock::now(); auto& c = gCost[what];
      c.first += std::chrono::duration<double>(now - gLast).count(); ++c.second; gLast = now; }
    const std::string text = raw ? raw : "";
    scrivi_free(raw);
    auto parsed = scrivi::util::parseJson(text);
    if (!parsed.ok() || !parsed.value().getBool("ok")) {
        std::fprintf(stderr, "FAILED: %s: %s\n", what, text.substr(0, 400).c_str());
        std::exit(1);
    }
    return parsed.value().getSubDoc("result");
}

std::string jsonString(const std::string& s) { JsonDoc d; d.setString("s", s); auto j = d.dump(-1); return j.substr(5, j.size() - 6); }

const std::vector<std::string> kSentences = {
    "The rain had not stopped since Lyon, and the horses steamed in the courtyard.",
    "He folded the letter twice and slid it beneath the lining of his hat.",
    "Nobody at the inn would say which road the cardinal's men had taken.",
    "She laughed, but her hand did not leave the hilt of the little knife.",
    "The candle guttered; somewhere below, a door was barred.",
    "It was a debt of honour, and honour, in Paris, was paid at dawn.",
    "The abbé spoke of the treasure as other men speak of the weather.",
    "From the battlements the sea looked like hammered pewter.",
    "\"Monsieur,\" said the valet, \"there is a gentleman who will not give his name.\"",
    "Three days later the ship came into Marseille with her flag at half-mast.",
    "He counted the steps from the wall to the window, as he had every night for six years.",
    "The queen's diamonds were not where the jeweller had sworn they would be.",
};
const std::vector<std::string> kWords = {"quietly", "at once", "the old count", "a stranger", "by moonlight", "in silence"};

}  // namespace

int main(int argc, char** argv)
{
    if (argc < 4) {
        std::fprintf(stderr, "usage: scrivi_make_test_history <sourceProject> <destProject> <appSupportRoot> "
                             "[events=25000] [sessions=120]\n");
        return 2;
    }
    const std::string source = argv[1], dest = argv[2], appSupport = argv[3];
    const int events   = argc > 4 ? std::max(1, std::atoi(argv[4])) : 25000;
    const int sessions = argc > 5 ? std::max(1, std::atoi(argv[5])) : 120;

    if (fs::exists(dest)) {
        std::fprintf(stderr, "REFUSING: %s already exists. This fixture only CREATES projects.\n", dest.c_str());
        return 3;
    }
    if (!fs::exists(fs::path(source) / "project.json")) {
        std::fprintf(stderr, "not a project: %s\n", source.c_str());
        return 2;
    }
    const auto t0 = std::chrono::steady_clock::now();
    fs::copy(source, dest, fs::copy_options::recursive);
    // AppleDouble files copied from a FAT/exFAT volume are not the project's.
    for (auto it = fs::recursive_directory_iterator(dest); it != fs::recursive_directory_iterator(); ++it) {
        if (it->path().filename().string().rfind("._", 0) == 0) { fs::remove(it->path()); }
    }
    const char* root = dest.c_str();

    // ⚠️ A fixed identity, NOT scrivi_ensure_local_identity: on a Mac that reaches the keychain, and an unsigned tool
    // rebuilt since the last run waits on a keychain prompt. The saves only record these strings as the author.
    const std::string identityID = "identity-history-fixture", personaID = "persona-history-fixture";
    const auto opened = call("open_project", scrivi_open_project(root, appSupport.c_str(), identityID.c_str()));
    const std::string projectID = opened.getString("projectID");

    struct Scene { std::string id, meta, content, text; };
    std::vector<Scene> scenes;
    for (std::size_t i = 0; i < opened.arraySize("scenes"); ++i) {
        const auto s = opened.arrayItem("scenes", i);
        scenes.push_back({s.getString("sceneID"), s.getString("metadataPath"), s.getString("contentPath"), {}});
    }
    {
        std::string ids = "[";
        for (std::size_t i = 0; i < scenes.size(); ++i) { ids += (i ? "," : "") + jsonString(scenes[i].id); }
        const auto texts = call("read_scene_texts", scrivi_read_scene_texts(root, (ids + "]").c_str()));
        std::map<std::string, std::string> byID;
        for (std::size_t i = 0; i < texts.arraySize("scenes"); ++i) {
            const auto t = texts.arrayItem("scenes", i);
            byID[t.getString("sceneID")] = t.getString("markdown");
        }
        for (auto& s : scenes) { s.text = byID[s.id]; }
    }
    std::map<std::string, std::size_t> indexOf;
    for (std::size_t i = 0; i < scenes.size(); ++i) { indexOf[scenes[i].id] = i; }
    std::printf("copied %s → %s: %zu scenes\n", source.c_str(), dest.c_str(), scenes.size());

    std::mt19937 rng(1844);   // deterministic: the same fixture every run
    auto pick = [&](std::size_t n) { return std::uniform_int_distribution<std::size_t>(0, n - 1)(rng); };
    auto chance = [&](int percent) { return static_cast<int>(pick(100)) < percent; };

    int recorded = 0, undos = 0, redos = 0, barriers = 0, branches = 0;
    for (int session = 0; session < sessions && recorded < events; ++session) {
        call("history_open", scrivi_history_open(root));
        std::set<std::size_t> touched, seeded;
        std::size_t focus = pick(scenes.size());   // a writer works in one stretch of the book per sitting
        const int quota = (events - recorded) / (sessions - session);

        for (int n = 0; n < quota; ++n) {
            if (chance(5)) { focus = pick(scenes.size()); }
            const std::size_t at = std::min(scenes.size() - 1, focus + pick(12));
            Scene& sc = scenes[at];
            if (seeded.insert(at).second) {
                call("history_seed_scene", scrivi_history_seed_scene(root, sc.id.c_str(), sc.text.c_str()));
            }
            const int roll = static_cast<int>(pick(100));
            if (roll < 4 && recorded > 0) {   // an undo run; most are followed by new typing, which branches
                const std::size_t steps = 1 + pick(6);
                for (std::size_t k = 0; k < steps; ++k) {
                    const auto u = call("history_undo", scrivi_history_undo(root));
                    if (!u.getBool("moved")) { break; }   // the root, or a barrier
                    if (u.contains("structuralInverse")) {
                        // The copied history's reversible structural node: the APP runs its inverse on disk, which this
                        // tool cannot. Step straight back so disk and history stay in agreement.
                        call("history_redo", scrivi_history_redo(root));
                        break;
                    }
                    ++undos;
                    for (std::size_t c = 0; c < u.arraySize("changes"); ++c) {
                        const auto ch = u.arrayItem("changes", c);
                        if (auto it = indexOf.find(ch.getString("sceneID")); it != indexOf.end()) {
                            scenes[it->second].text = ch.getString("newText");
                            touched.insert(it->second);
                        }
                    }
                }
                if (chance(35)) {
                    const auto r = call("history_redo", scrivi_history_redo(root));
                    if (r.contains("structuralInverse")) {
                        call("history_undo", scrivi_history_undo(root));   // as above: never cross one here
                    } else if (r.getBool("moved")) {
                        ++redos;
                        for (std::size_t c = 0; c < r.arraySize("changes"); ++c) {
                            const auto ch = r.arrayItem("changes", c);
                            if (auto it = indexOf.find(ch.getString("sceneID")); it != indexOf.end()) {
                                scenes[it->second].text = ch.getString("newText");
                            }
                        }
                    }
                }
                continue;
            }
            if (roll < 5) {
                JsonDoc p;
                p.setString("barrierKind", "structural");
                p.setString("note", "generated barrier");
                call("history_record_barrier", scrivi_history_record_barrier(root, p.dump(-1).c_str()));
                ++barriers;
                continue;
            }

            const std::string before = sc.text;
            std::string kind = "typing";
            std::size_t cursor = before.size();
            if (roll < 70) {          // type a sentence at the end, or after a paragraph break inside
                const auto brk = before.find("\n\n", pick(before.size() + 1));
                cursor = (brk == std::string::npos) ? before.size() : brk;
                sc.text.insert(cursor, std::string(cursor ? " " : "") + kSentences[pick(kSentences.size())]);
            } else if (roll < 82) {   // delete a sentence-sized span
                kind = "delete";
                if (before.size() < 80) { continue; }
                cursor = pick(before.size() - 60);
                sc.text.erase(cursor, 20 + pick(40));
            } else if (roll < 92) {   // replace a short span with a phrase
                kind = "replace";
                if (before.size() < 20) { continue; }
                cursor = pick(before.size() - 10);
                sc.text.replace(cursor, 3 + pick(6), kWords[pick(kWords.size())]);
            } else if (roll < 97) {   // paste a paragraph
                kind = "paste";
                sc.text += "\n\n" + kSentences[pick(kSentences.size())] + " " + kSentences[pick(kSentences.size())];
            } else {                  // cut a span
                kind = "cut";
                if (before.size() < 200) { continue; }
                cursor = pick(before.size() - 150);
                sc.text.erase(cursor, 50 + pick(100));
            }
            // The edits are byte offsets, so one can split a multi-byte character: drop the orphaned bytes (valid UTF-8).
            for (std::size_t i = 0; i < sc.text.size();) {
                const unsigned char c = static_cast<unsigned char>(sc.text[i]);
                const std::size_t len = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : (c >> 3) == 30 ? 4 : 0;
                bool ok = len > 0 && i + len <= sc.text.size();
                for (std::size_t k = 1; ok && k < len; ++k) { ok = (static_cast<unsigned char>(sc.text[i + k]) >> 6) == 2; }
                if (ok) { i += len; } else { sc.text.erase(i, 1); }
            }
            JsonDoc p;
            p.setString("kind", kind);
            p.setInt64("cursorBefore", static_cast<std::int64_t>(cursor));
            p.setInt64("cursorAfter", static_cast<std::int64_t>(std::min(cursor, sc.text.size())));
            const auto e = call("history_record_event",
                                scrivi_history_record_event(root, sc.id.c_str(), sc.text.c_str(), p.dump(-1).c_str()));
            if (e.getBool("noOp")) { continue; }
            if (e.getBool("createdBranch")) { ++branches; }
            ++recorded;
            touched.insert(at);
        }

        // As the app at a close: save each scene this sitting changed, tell history the bytes written, close.
        for (std::size_t i : touched) {
            const Scene& sc = scenes[i];
            call("save_scene", scrivi_save_scene(projectID.c_str(), root, appSupport.c_str(), sc.id.c_str(),
                                                 sc.meta.c_str(), sc.content.c_str(), sc.text.c_str(), 0, 0, 0.0,
                                                 identityID.c_str(), personaID.c_str(), "History Fixture"));
            call("history_note_scene_persisted",
                 scrivi_history_note_scene_persisted(root, sc.id.c_str(), sc.text.c_str()));
        }
        call("wait_for_writes", scrivi_wait_for_writes(root, 0, -1));
        call("history_close", scrivi_history_close(root));
        call("wait_for_writes", scrivi_wait_for_writes(root, 0, -1));
        std::printf("session %3d: %6d events, %5zu scenes touched\n", session + 1, recorded, touched.size());
        std::fflush(stdout);
    }
    // Self-check, as the next open will: every scene's disk text must match the history's head, or the app would flag it
    // as edited outside Scrivi (an externalChange barrier).
    int mismatched = 0;
    {
        std::string ids = "[";
        for (std::size_t i = 0; i < scenes.size(); ++i) { ids += std::string(i ? "," : "") + jsonString(scenes[i].id); }
        const auto texts = call("read_scene_texts", scrivi_read_scene_texts(root, (ids + "]").c_str()));
        const auto t1 = std::chrono::steady_clock::now();
        call("history_open", scrivi_history_open(root));
        std::printf("history_open on the result: %.0f ms\n",
                    std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t1).count());
        for (std::size_t i = 0; i < texts.arraySize("scenes"); ++i) {
            const auto t = texts.arrayItem("scenes", i);
            const auto v = call("history_validate_scene", scrivi_history_validate_scene(
                                    root, t.getString("sceneID").c_str(), t.getString("markdown").c_str()));
            if (v.getBool("externalChange")) { ++mismatched; }
        }
        // ⚠️ validate records a barrier for each mismatch, so a failed check leaves the fixture marked: rebuild it.
        call("wait_for_writes", scrivi_wait_for_writes(root, 0, -1));
        call("history_close", scrivi_history_close(root));
        call("wait_for_writes", scrivi_wait_for_writes(root, 0, -1));
    }
    call("close_project", scrivi_close_project(root));

    const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    std::printf("done in %.0f s: %d events recorded, %d undos, %d redos, %d branches, %d barriers\n",
                secs, recorded, undos, redos, branches, barriers);
    std::printf("self-check: %d of %zu scenes differ from the history's head%s\n", mismatched, scenes.size(),
                mismatched ? " — FAILED" : "");
    for (auto& [k, v] : gCost) { std::printf("  %-32s %8.1f s  %6d calls\n", k.c_str(), v.first, v.second); }
    return mismatched ? 1 : 0;
}
