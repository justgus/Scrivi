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
        src/public_api/scrivi_c_api.cpp
        src/identity/IdentityService.cpp
        src/platform/SystemUUIDProvider.cpp
        src/project_package/ProjectCreator.cpp
        src/project_package/ProjectValidator.cpp
        src/project_package/ProjectOpener.cpp
        src/manuscript/ManuscriptOrderResolver.cpp
        src/manuscript/ChapterCreator.cpp
        src/manuscript/ChapterDeleter.cpp
        src/manuscript/ChapterRenamer.cpp
        src/manuscript/ChapterReorderer.cpp
        src/manuscript/SceneCreator.cpp
        src/manuscript/SceneDeleter.cpp
        src/manuscript/SceneRenamer.cpp
        src/manuscript/SceneReorderer.cpp
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
        src/util/MarkdownStrip.cpp
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
        src/schemas/TimelineMetaJson.cpp
        src/schemas/StoryStructureJson.cpp
        src/schemas/HistoricalEventJson.cpp
        src/schemas/ExternalTimelineJson.cpp
        src/objects/ObjectStore.cpp
        src/assets/AssetStore.cpp
        src/comments/CommentStore.cpp
        src/inbox/InboxStore.cpp
        src/history/HistoryService.cpp
        src/history/HistoryStore.cpp
)

# Persistent SecureStore (SP-059 / AC4, per T-0058 trade study). On Linux the
# in-memory PrototypeSecureStore is replaced by EncryptedFileSecureStore
# (AES-256-GCM via OpenSSL 3), so the local identity survives process restart.
# Gated to Linux for now; Apple/Windows keep their current wiring. libssl-dev is
# in the Docker/CI apt lists.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(OpenSSL 3 REQUIRED)
    target_sources(ScriviCore PRIVATE src/platform/EncryptedFileSecureStore.cpp)
    target_compile_definitions(ScriviCore PRIVATE SCRIVI_HAS_ENCRYPTED_SECURE_STORE)
    target_link_libraries(ScriviCore PRIVATE OpenSSL::Crypto)
endif()

target_link_libraries(ScriviCore
    PRIVATE
        nlohmann_json::nlohmann_json
        $<$<PLATFORM_ID:Windows>:Shell32>
)

if(SCRIVI_BUILD_TESTS)
    add_subdirectory(tests)
endif()
