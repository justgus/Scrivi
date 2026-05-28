#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/CommentTypes.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"
#include "platform/LocalFileSystem.hpp"

#include <chrono>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

struct CommentFixture {
    fs::path projectDir;
    fs::path appSupportDir;

    scrivi::platform::LocalFileSystem        fileSystem;
    scrivi::mocks::DeterministicUUIDProvider uuidProvider;
    scrivi::mocks::FixedClock                clock{"2026-05-28T10:00:00Z"};
    scrivi::mocks::MockGitProvider           gitProvider;
    scrivi::mocks::MockSecureStore           secureStore;
    scrivi::ScriviCore                       core;

    const scrivi::AuthorshipRef author{
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    const scrivi::AuthorshipRef resolver{
        scrivi::IdentityID{"identity-002"},
        scrivi::PersonaID {"persona-002"},
        "Reviewer"
    };

    CommentFixture()
        : projectDir(fs::temp_directory_path() /
                     ("scrivi-comment-test-" + std::to_string(
                         std::chrono::steady_clock::now().time_since_epoch().count())))
        , appSupportDir(projectDir / "appsupport")
        , core([&]{
            scrivi::CoreServices svc;
            svc.fileSystem   = &fileSystem;
            svc.uuidProvider = &uuidProvider;
            svc.clock        = &clock;
            svc.gitProvider  = &gitProvider;
            svc.secureStore  = &secureStore;
            svc.logger       = nullptr;
            return svc;
          }())
    {
        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);

        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.string();
        req.appSupportRoot  = appSupportDir.string();
        req.title           = "Comment Test Project";
        req.slug            = "comment-test-project";
        req.author          = author;
        (void)core.createProject(req);
    }

    ~CommentFixture() { fs::remove_all(projectDir); }

    scrivi::AddCommentRequest makeAddReq(const std::string& scopeKind,
                                          const std::string& targetID,
                                          const std::string& body) const
    {
        scrivi::AddCommentRequest req;
        req.projectRootPath = projectDir.string();
        req.scopeKind       = scopeKind;
        req.targetID        = targetID;
        req.body            = body;
        req.author          = author;
        return req;
    }

    scrivi::ListCommentsRequest makeListReq(const std::string& scopeKind,
                                             const std::string& targetID) const
    {
        scrivi::ListCommentsRequest req;
        req.projectRootPath = projectDir.string();
        req.scopeKind       = scopeKind;
        req.targetID        = targetID;
        return req;
    }

    scrivi::ResolveCommentRequest makeResolveReq(const std::string& scopeKind,
                                                  const std::string& targetID,
                                                  const std::string& commentID) const
    {
        scrivi::ResolveCommentRequest req;
        req.projectRootPath = projectDir.string();
        req.scopeKind       = scopeKind;
        req.targetID        = targetID;
        req.commentID       = commentID;
        req.resolver        = resolver;
        return req;
    }
};

// ---------------------------------------------------------------------------
// T-0045: scene scope — add creates thread file
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(CommentFixture, "addComment creates thread file for scene scope",
                 "[integration][T-0045]")
{
    auto result = core.addComment(makeAddReq("scene", "scene_01", "Needs more tension here."));
    REQUIRE(result.ok());
    REQUIRE(result.value().added == true);
    REQUIRE_FALSE(result.value().commentID.empty());

    // Thread file was written.
    auto threadPath = projectDir / "comments" / "scene" / "scene_01.comments.json";
    REQUIRE(fs::exists(threadPath));
}

// ---------------------------------------------------------------------------
// T-0045: scene scope — listComments returns all comments
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(CommentFixture, "listComments returns all comments for scene scope",
                 "[integration][T-0045]")
{
    REQUIRE(core.addComment(makeAddReq("scene", "scene_01", "First note.")).ok());
    REQUIRE(core.addComment(makeAddReq("scene", "scene_01", "Second note.")).ok());

    auto result = core.listComments(makeListReq("scene", "scene_01"));
    REQUIRE(result.ok());
    REQUIRE(result.value().comments.size() == 2);
    REQUIRE(result.value().scopeKind == "scene");
    REQUIRE(result.value().targetID  == "scene_01");
}

// ---------------------------------------------------------------------------
// T-0045: scene scope — resolveComment sets resolved=true and resolvedAt
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(CommentFixture, "resolveComment sets resolved flag and resolvedAt for scene scope",
                 "[integration][T-0045]")
{
    auto addResult = core.addComment(makeAddReq("scene", "scene_01", "Fix this."));
    REQUIRE(addResult.ok());
    const auto commentID = addResult.value().commentID;

    auto resolveResult = core.resolveComment(makeResolveReq("scene", "scene_01", commentID));
    REQUIRE(resolveResult.ok());
    REQUIRE(resolveResult.value().resolved == true);

    // Verify via listComments.
    auto listResult = core.listComments(makeListReq("scene", "scene_01"));
    REQUIRE(listResult.ok());
    REQUIRE(listResult.value().comments.size() == 1);
    const auto& c = listResult.value().comments[0];
    REQUIRE(c.resolved == true);
    REQUIRE(c.resolvedAt.has_value());
    REQUIRE(c.resolvedByDisplayName.has_value());
    REQUIRE(c.resolvedByDisplayName.value() == "Reviewer");
}

// ---------------------------------------------------------------------------
// T-0045: object scope — add, list, resolve
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(CommentFixture, "addComment creates thread file for object scope",
                 "[integration][T-0045]")
{
    auto result = core.addComment(makeAddReq("object", "character_01", "Age feels inconsistent."));
    REQUIRE(result.ok());
    REQUIRE(result.value().added == true);

    auto threadPath = projectDir / "comments" / "object" / "character_01.comments.json";
    REQUIRE(fs::exists(threadPath));
}

TEST_CASE_METHOD(CommentFixture, "listComments returns all comments for object scope",
                 "[integration][T-0045]")
{
    REQUIRE(core.addComment(makeAddReq("object", "character_01", "Backstory note.")).ok());
    REQUIRE(core.addComment(makeAddReq("object", "character_01", "Arc note.")).ok());

    auto result = core.listComments(makeListReq("object", "character_01"));
    REQUIRE(result.ok());
    REQUIRE(result.value().comments.size() == 2);
}

TEST_CASE_METHOD(CommentFixture, "resolveComment sets resolved flag for object scope",
                 "[integration][T-0045]")
{
    auto addResult = core.addComment(makeAddReq("object", "character_01", "Resolve me."));
    REQUIRE(addResult.ok());
    const auto commentID = addResult.value().commentID;

    REQUIRE(core.resolveComment(makeResolveReq("object", "character_01", commentID)).ok());

    auto listResult = core.listComments(makeListReq("object", "character_01"));
    REQUIRE(listResult.ok());
    REQUIRE(listResult.value().comments[0].resolved == true);
}

// ---------------------------------------------------------------------------
// T-0045: scene and object scopes are independent (different thread files)
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(CommentFixture, "scene and object scopes write to separate thread files",
                 "[integration][T-0045]")
{
    REQUIRE(core.addComment(makeAddReq("scene",  "target_01", "Scene comment.")).ok());
    REQUIRE(core.addComment(makeAddReq("object", "target_01", "Object comment.")).ok());

    auto sceneList  = core.listComments(makeListReq("scene",  "target_01"));
    auto objectList = core.listComments(makeListReq("object", "target_01"));
    REQUIRE(sceneList.ok());
    REQUIRE(objectList.ok());
    REQUIRE(sceneList.value().comments.size()  == 1);
    REQUIRE(objectList.value().comments.size() == 1);
    REQUIRE(sceneList.value().comments[0].body  == "Scene comment.");
    REQUIRE(objectList.value().comments[0].body == "Object comment.");
}
