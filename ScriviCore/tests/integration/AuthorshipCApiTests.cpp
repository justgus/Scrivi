// EP-058 S1 (SP-209): authorship through the C ABI — the ordered authors of a project and of a world, writing as, the
// "unnamed" rule and the prompt it raises on open (Q1, Q3), a world's first author (Q2), the pen-name menu (Q4), and
// `controlledBy` as a list (Q5). Design: docs/Scrivi_Authorship_Design_v0_2.md. Through `scrivi_*`, never the facade
// ([I-0113]).

#include <catch2/catch_test_macros.hpp>

#include "identity/AuthorStore.hpp"
#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
using scrivi::util::JsonDoc;
using scrivi::util::parseJson;

JsonDoc envelope(const char* raw) {
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    return std::move(parsed.value());
}

JsonDoc okResult(const char* raw) {
    auto e = envelope(raw);
    INFO("envelope: " << e.dump());
    REQUIRE(e.getBool("ok"));
    return e.getSubDoc("result");
}

void refused(const char* raw) {
    auto e = envelope(raw);
    INFO("envelope: " << e.dump());
    CHECK_FALSE(e.getBool("ok"));
}

std::string slurp(const fs::path& p) {
    std::ifstream in(p);
    return {std::istreambuf_iterator<char>(in), {}};
}

// The process remembers the identity's name (Q1); a test that sets it puts it back.
struct RecordedName {
    explicit RecordedName(const std::string& n) { scrivi::identity::rememberIdentityName(n); }
    ~RecordedName() { scrivi::identity::rememberIdentityName(""); }
};

struct Dirs {
    fs::path project, support, world;
    explicit Dirs(const std::string& author = "Ada Lovelace") {
        static std::atomic<int> n{0};
        const std::string stem = "scrivi-authorship-" + std::to_string(n.fetch_add(1)) + "-" +
                                 std::to_string(reinterpret_cast<std::uintptr_t>(this));
        project = fs::temp_directory_path() / (stem + ".scrivi");
        support = fs::temp_directory_path() / (stem + "-support");
        world   = fs::temp_directory_path() / (stem + "-World.scrivworld");
        fs::create_directories(support);
        okResult(scrivi_create_project(root(), support.c_str(), "Once", "once", "identity-001", "persona-001",
                                       author.c_str()));
    }
    ~Dirs() {
        std::error_code ec;
        fs::remove_all(project, ec);
        fs::remove_all(support, ec);
        fs::remove_all(world, ec);
    }
    const char* root() const { return project.c_str(); }
    JsonDoc open(const char* identity = "identity-001") {
        auto r = okResult(scrivi_open_project(root(), support.c_str(), identity));
        scrivi_free(scrivi_close_project(root()));
        return r;
    }
    JsonDoc authors(const char* identity = "identity-001") { return okResult(scrivi_get_authors(root(), identity, "")); }
};

std::vector<std::string> names(const JsonDoc& view) {
    std::vector<std::string> out;
    for (std::size_t i = 0; i < view.arraySize("authors"); ++i) {
        out.push_back(view.arrayItem("authors", i).getString("displayName"));
    }
    return out;
}

std::string idOf(const JsonDoc& view, const std::string& name) {
    for (std::size_t i = 0; i < view.arraySize("authors"); ++i) {
        if (view.arrayItem("authors", i).getString("displayName") == name) {
            return view.arrayItem("authors", i).getString("personaID");
        }
    }
    return {};
}

} // namespace

TEST_CASE("Authorship: a new project opens with its author, written as, and nothing to ask (AC3)",
          "[authorship][capi]") {
    Dirs d;
    const auto opened = d.open();
    CHECK(names(opened) == std::vector<std::string>{"Ada Lovelace"});
    CHECK(opened.arrayItem("authors", 0).getString("personaID") == "persona-001");
    CHECK_FALSE(opened.arrayItem("authors", 0).getBool("unnamed"));
    CHECK(opened.getSubDoc("writingAs").getString("displayName") == "Ada Lovelace");
    CHECK(opened.getString("authorPrompt") == "none");
}

TEST_CASE("Authorship: a package credited to \"Scrivi\" asks for a name, and naming it keeps the ID (AC3, Q1, Q3)",
          "[authorship][capi]") {
    Dirs d("Scrivi");
    auto opened = d.open();
    CHECK(opened.arrayItem("authors", 0).getBool("unnamed"));
    CHECK(opened.getString("authorPrompt") == "nameAuthor");

    okResult(scrivi_set_authors(d.root(), R"({"authors":[{"personaID":"persona-001","displayName":"  Kit Marlowe "}]})"));
    opened = d.open();
    CHECK(names(opened) == std::vector<std::string>{"Kit Marlowe"});   // trimmed
    CHECK(opened.arrayItem("authors", 0).getString("personaID") == "persona-001");
    CHECK_FALSE(opened.arrayItem("authors", 0).getBool("unnamed"));
    CHECK(opened.getSubDoc("writingAs").getString("personaID") == "persona-001");
    CHECK(opened.getString("authorPrompt") == "none");
}

TEST_CASE("Authorship: the identity's recorded name counts as unnamed (Q1)", "[authorship][capi]") {
    RecordedName host("flitwick-5.local");
    Dirs d("flitwick-5.local");
    const auto view = d.authors();
    CHECK(view.arrayItem("authors", 0).getBool("unnamed"));
    CHECK(view.getString("authorPrompt") == "nameAuthor");
}

// Q1 end to end: the name comes from scrivi_ensure_local_identity itself. ⚠️ On Apple that is the real login Keychain, so
// the case is hidden there, as the Keychain tests are (`ScriviCoreTests "[keychain]"`); Linux's store is a file.
#if defined(__APPLE__)
#define SCRIVI_IDENTITY_TAGS "[authorship][capi][.keychain]"
#else
#define SCRIVI_IDENTITY_TAGS "[authorship][capi]"
#endif
TEST_CASE("Authorship: scrivi_ensure_local_identity's name is the one treated as unnamed (Q1)", SCRIVI_IDENTITY_TAGS) {
    Dirs d;
    const auto identity = okResult(scrivi_ensure_local_identity("scrivi-test-host", d.support.c_str()));
    const std::string name = identity.getString("displayName");
    okResult(scrivi_set_authors(d.root(), (R"({"authors":[{"personaID":"persona-001","displayName":")" + name + "\"}]}").c_str()));
    CHECK(d.authors().arrayItem("authors", 0).getBool("unnamed"));
    scrivi::identity::rememberIdentityName("");
}

TEST_CASE("Authorship: several authors in order — added, reordered, retired, returned (AC1, design Q2)",
          "[authorship][capi]") {
    Dirs d;
    // A co-author who does not use Scrivi is just a name (design Q2).
    okResult(scrivi_set_authors(d.root(),
        R"({"authors":[{"personaID":"persona-001","displayName":"Ada Lovelace"},{"displayName":"Charles Babbage"}]})"));
    auto view = d.authors();
    CHECK(names(view) == std::vector<std::string>{"Ada Lovelace", "Charles Babbage"});
    const std::string babbage = idOf(view, "Charles Babbage");
    REQUIRE_FALSE(babbage.empty());
    CHECK(babbage != "persona-001");

    okResult(scrivi_set_authors(d.root(), (R"({"authors":[{"personaID":")" + babbage +
        R"(","displayName":"Charles Babbage"},{"personaID":"persona-001","displayName":"Ada Lovelace"}]})").c_str()));
    CHECK(names(d.authors()) == std::vector<std::string>{"Charles Babbage", "Ada Lovelace"});

    // Left out: retired, never deleted — the file still holds it, and the byline does not.
    okResult(scrivi_set_authors(d.root(), R"({"authors":[{"personaID":"persona-001","displayName":"Ada Lovelace"}]})"));
    CHECK(names(d.authors()) == std::vector<std::string>{"Ada Lovelace"});
    const auto file = slurp(d.project / "identities" / "project-personas.json");
    CHECK(file.find(babbage) != std::string::npos);
    CHECK(file.find("\"retired\"") != std::string::npos);

    // A retired author may return, with the same ID.
    okResult(scrivi_set_authors(d.root(), (R"({"authors":[{"personaID":"persona-001","displayName":"Ada Lovelace"},{"personaID":")" +
        babbage + R"(","displayName":"C. Babbage"}]})").c_str()));
    view = d.authors();
    CHECK(names(view) == std::vector<std::string>{"Ada Lovelace", "C. Babbage"});
    CHECK(idOf(view, "C. Babbage") == babbage);
}

TEST_CASE("Authorship: what set_authors and set_writing_as refuse (AC1, AC2)", "[authorship][capi]") {
    Dirs d;
    const auto before = slurp(d.project / "identities" / "project-personas.json");
    refused(scrivi_set_authors(d.root(), R"({"authors":[]})"));
    refused(scrivi_set_authors(d.root(), R"({"authors":[{"displayName":"   "}]})"));
    refused(scrivi_set_authors(d.root(), R"({"authors":[{"personaID":"persona-nope","displayName":"X"}]})"));
    refused(scrivi_set_authors(d.root(), R"({"authors":[{"personaID":"persona-001","displayName":"A"},{"personaID":"persona-001","displayName":"B"}]})"));
    refused(scrivi_set_authors(d.root(), R"([1,2])"));
    refused(scrivi_set_authors(d.root(), "not json"));
    CHECK(slurp(d.project / "identities" / "project-personas.json") == before);   // nothing half-written

    okResult(scrivi_set_authors(d.root(), R"({"authors":[{"displayName":"New"}]})"));   // persona-001 retired
    refused(scrivi_set_writing_as(d.root(), "identity-001", "persona-001"));
    refused(scrivi_set_writing_as(d.root(), "identity-001", "persona-nope"));
    refused(scrivi_set_writing_as(d.root(), "", idOf(d.authors(), "New").c_str()));
    // Its author retired, the identity writes as no one: the app asks which author it is.
    CHECK(d.authors().getString("authorPrompt") == "chooseAuthor");
    refused(scrivi_get_authors((d.project / "not-a-package").c_str(), "identity-001", ""));
}

TEST_CASE("Authorship: a second machine is asked which author it is, then writes as it (AC2, design Q3, Q5)",
          "[authorship][capi]") {
    Dirs d;
    okResult(scrivi_set_authors(d.root(),
        R"({"authors":[{"personaID":"persona-001","displayName":"Ada Lovelace"},{"displayName":"Charles Babbage"}]})"));
    auto opened = d.open("identity-002");
    CHECK(opened.getString("authorPrompt") == "chooseAuthor");
    CHECK_FALSE(opened.contains("writingAs"));

    const std::string babbage = idOf(opened, "Charles Babbage");
    okResult(scrivi_set_writing_as(d.root(), "identity-002", babbage.c_str()));
    opened = d.open("identity-002");
    CHECK(opened.getString("authorPrompt") == "none");
    CHECK(opened.getSubDoc("writingAs").getString("personaID") == babbage);
    CHECK(d.open("identity-001").getSubDoc("writingAs").getString("personaID") == "persona-001");   // unchanged

    // A duo: two identities writing as ONE author — controlledBy keeps both (Q5).
    okResult(scrivi_set_writing_as(d.root(), "identity-002", "persona-001"));
    const auto file = parseJson(slurp(d.project / "identities" / "project-personas.json"));
    REQUIRE(file.ok());
    const auto ada = file.value().arrayItem("personas", 0);
    REQUIRE(ada.getString("personaID") == "persona-001");
    REQUIRE(ada.arraySize("controlledBy") == 2);
    CHECK(ada.arrayItem("controlledBy", 0).getString("value") == "identity-001");
    CHECK(ada.arrayItem("controlledBy", 1).getString("value") == "identity-002");
    const auto members = slurp(d.project / "identities" / "project-members.json");
    CHECK(members.find("identity-002") != std::string::npos);
}

TEST_CASE("Authorship: a world takes the project's writing-as author, then keeps its own list (AC4, Q2)",
          "[authorship][capi]") {
    Dirs d;
    okResult(scrivi_create_world(d.root(), d.world.c_str(), "Eskandar", "Year One"));
    const auto worldPersonas = d.world / "identities" / "world-personas.json";
    REQUIRE_FALSE(fs::exists(worldPersonas));

    // Without a project to take it from: no authors, nothing written.
    auto view = okResult(scrivi_get_authors(d.world.c_str(), "identity-001", ""));
    CHECK(view.arraySize("authors") == 0);
    CHECK(view.getString("authorPrompt") == "nameAuthor");
    CHECK_FALSE(fs::exists(worldPersonas));

    view = okResult(scrivi_get_authors(d.world.c_str(), "identity-001", d.root()));
    CHECK(names(view) == std::vector<std::string>{"Ada Lovelace"});
    CHECK(view.getSubDoc("writingAs").getString("personaID") == "persona-001");
    CHECK(view.getString("authorPrompt") == "none");
    CHECK(fs::exists(worldPersonas));
    CHECK(fs::exists(d.world / "identities" / "world-members.json"));

    // Independent: the world's list changes, the project's does not.
    okResult(scrivi_set_authors(d.world.c_str(),
        R"({"authors":[{"personaID":"persona-001","displayName":"Ada Lovelace"},{"displayName":"Mary Shelley"}]})"));
    CHECK(names(okResult(scrivi_get_authors(d.world.c_str(), "identity-001", d.root()))) ==
          std::vector<std::string>{"Ada Lovelace", "Mary Shelley"});
    CHECK(names(d.authors()) == std::vector<std::string>{"Ada Lovelace"});
}

TEST_CASE("Authorship: the pen-name menu round-trips outside every package (AC5, Q4)", "[authorship][capi]") {
    Dirs d;
    auto menu = okResult(scrivi_get_pen_names(d.support.c_str()));
    CHECK(menu.arraySize("penNames") == 0);
    CHECK(menu.getString("default").empty());

    okResult(scrivi_put_pen_names(d.support.c_str(), R"({"penNames":["Kit Marlowe"," J. R. Gustafson "],"default":"Kit Marlowe"})"));
    menu = okResult(scrivi_get_pen_names(d.support.c_str()));
    CHECK(menu.getStringArray("penNames") == std::vector<std::string>{"Kit Marlowe", "J. R. Gustafson"});
    CHECK(menu.getString("default") == "Kit Marlowe");
    CHECK(fs::exists(d.support / "pen-names.json"));
    CHECK(slurp(d.support / "pen-names.json").find("scrivi.penNames.v1") != std::string::npos);

    refused(scrivi_put_pen_names(d.support.c_str(), R"({"penNames":["A",""]})"));
    refused(scrivi_put_pen_names(d.support.c_str(), R"({"penNames":["A","A"]})"));
    refused(scrivi_put_pen_names(d.support.c_str(), R"({"penNames":["A"],"default":"B"})"));
    refused(scrivi_put_pen_names(d.support.c_str(), "[]"));
    refused(scrivi_get_pen_names(""));
    CHECK(okResult(scrivi_get_pen_names(d.support.c_str())).getString("default") == "Kit Marlowe");   // unchanged

    okResult(scrivi_put_pen_names(d.support.c_str(), R"({"penNames":[]})"));
    CHECK(okResult(scrivi_get_pen_names(d.support.c_str())).arraySize("penNames") == 0);
}
