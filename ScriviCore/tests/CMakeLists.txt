include(FetchContent)

FetchContent_Declare(
    Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.6.0.tar.gz
    URL_HASH SHA256=485932259a75c7c6b72d4b874242c489ea5155d17efa345eb8cc72159f49f356
)

FetchContent_MakeAvailable(Catch2)

add_executable(ScriviCoreTests
    unit/ResultTests.cpp
    unit/IDTests.cpp
    unit/PathUtilsTests.cpp
    unit/SlugTests.cpp
    unit/TextStatsTests.cpp
    unit/MarkdownStripTests.cpp
    unit/JsonSchemaTests.cpp
    unit/HistoryServiceTests.cpp
    unit/LocalFileSystemTests.cpp
    unit/OrderKeyTests.cpp
    unit/ChapterIndexTests.cpp
    integration/AppSupportLayoutTests.cpp
    integration/IdentityTests.cpp
    integration/CreateProjectTests.cpp
    integration/OpenProjectTests.cpp
    integration/SaveSceneTests.cpp
    integration/ExternalChangeTests.cpp
    integration/GitSnapshotTests.cpp
    integration/MvpLoopTests.cpp
    integration/ApplyRepairTests.cpp
    integration/ObjectCrudTests.cpp
    integration/AssetTests.cpp
    integration/CommentTests.cpp
    integration/InboxTests.cpp
    integration/MultiSceneTests.cpp
    integration/CreateSceneTests.cpp
    integration/DeleteSceneTests.cpp
    integration/RenameSceneTests.cpp
    integration/ReorderTests.cpp
    integration/ChapterSlugCollisionTests.cpp
    integration/ChapterMigrationTests.cpp
    integration/SceneIdentityTests.cpp
    integration/SceneSplitRepro.cpp
    integration/TimelineTests.cpp
    integration/SearchableContentTests.cpp
    integration/HistoryCApiTests.cpp
    integration/EncryptedSecureStoreTests.cpp
)

# EncryptedFileSecureStore is Linux-only (SP-059 / AC4). Its symbols live in the
# ScriviCore static lib; the test TU needs the same compile define to enable its
# body (and pulls OpenSSL headers transitively via the store header).
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(ScriviCoreTests PRIVATE SCRIVI_HAS_ENCRYPTED_SECURE_STORE)
    find_package(OpenSSL 3 REQUIRED)
    target_link_libraries(ScriviCoreTests PRIVATE OpenSSL::Crypto)
endif()

target_include_directories(ScriviCoreTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/ScriviCore/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ScriviCoreTests
    PRIVATE
        ScriviCore
        Catch2::Catch2WithMain
)

# On MSVC: compile source as UTF-8 so em-dash literals in test names survive.
# On Windows at runtime: set the process code page to UTF-8 so CTest name
# matching works correctly with non-ASCII test names.
if(MSVC)
    target_compile_options(ScriviCoreTests PRIVATE /utf-8)
    target_compile_options(ScriviCore PRIVATE /utf-8)
    # Embed UTF-8 active code page manifest so non-ASCII test names survive
    # CTest's PRE_TEST subprocess launch on Windows.
    target_sources(ScriviCoreTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/utf8.manifest)
endif()

include(CTest)
include(Catch)
# DISCOVERY_MODE PRE_TEST defers test enumeration to run time — required for
# the Xcode generator, which sandboxes post-build scripts and kills the
# subprocess that BUILD_TIME mode tries to launch.
catch_discover_tests(ScriviCoreTests DISCOVERY_MODE PRE_TEST)
