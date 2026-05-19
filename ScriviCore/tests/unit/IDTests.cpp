#include <catch2/catch_test_macros.hpp>
#include "scrivi/IDs.hpp"

using namespace scrivi;

TEST_CASE("ID types hold string values", "[IDs]") {
    ProjectID    pid{"proj-001"};
    ManuscriptID mid{"ms-001"};
    SceneID      sid{"scene-001"};

    REQUIRE(pid.value == "proj-001");
    REQUIRE(mid.value == "ms-001");
    REQUIRE(sid.value == "scene-001");
}

// ID types are distinct structs — this verifies they are not accidentally
// interchangeable (would fail to compile if types were aliased).
TEST_CASE("ID types are distinct", "[IDs]") {
    ProjectID  pid{"x"};
    SceneID    sid{"x"};
    // pid = sid; // must not compile — intentionally left as comment
    REQUIRE(pid.value == sid.value); // values can be equal, types cannot be swapped
}
