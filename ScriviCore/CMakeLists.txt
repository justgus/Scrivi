include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(ScriviCore STATIC)

target_compile_features(ScriviCore PUBLIC cxx_std_23)

target_include_directories(ScriviCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(ScriviCore
    PRIVATE
        src/public_api/ScriviCore.cpp
        src/identity/IdentityService.cpp
        src/platform/SystemUUIDProvider.cpp
        src/project_package/ProjectCreator.cpp
        src/project_package/ProjectValidator.cpp
        src/project_package/ProjectOpener.cpp
        src/manuscript/ManuscriptOrderResolver.cpp
        src/manuscript/SceneReader.cpp
        src/manuscript/SceneWriter.cpp
        src/workspace/WorkspaceStateService.cpp
        src/repair/RepairClassifier.cpp
        src/repair/ExternalChangeScanner.cpp
        src/repair/RepairDispatcher.cpp
        src/repair/RepairHandlers.cpp
        src/git/SystemGitProvider.cpp
        src/git/SnapshotService.cpp
        src/domain/IDs.cpp
        src/domain/Slug.cpp
        src/domain/TextStats.cpp
        src/platform/AppSupportLayout.cpp
        src/util/Process.cpp
        src/util/PathUtils.cpp
        src/util/Json.cpp
        src/util/AtomicWrite.cpp
        src/util/Hash.cpp
        src/platform/LocalFileSystem.cpp
        src/schemas/ProjectJson.cpp
        src/schemas/ManuscriptMetaJson.cpp
        src/schemas/ChapterMetaJson.cpp
        src/schemas/SceneMetaJson.cpp
        src/schemas/ProjectMembersJson.cpp
        src/schemas/ProjectPersonasJson.cpp
        src/schemas/WorkspaceStateJson.cpp
        src/schemas/SnapshotMetadataJson.cpp
        src/schemas/RepairIssueJson.cpp
        src/schemas/ObjectJson.cpp
        src/schemas/AssetMetaJson.cpp
        src/schemas/CommentJson.cpp
        src/objects/ObjectStore.cpp
        src/assets/AssetStore.cpp
        src/comments/CommentStore.cpp
        src/inbox/InboxStore.cpp
)

target_link_libraries(ScriviCore
    PRIVATE
        nlohmann_json::nlohmann_json
)

if(SCRIVI_BUILD_TESTS)
    add_subdirectory(tests)
endif()
