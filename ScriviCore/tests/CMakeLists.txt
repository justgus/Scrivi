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
    integration/GolemImportTool.cpp
    integration/AppSupportLayoutTests.cpp
    integration/AppSupportRootCApiTests.cpp
    integration/IdentityTests.cpp
    integration/CreateProjectTests.cpp
    integration/OpenProjectTests.cpp
    integration/OpenProjectReadAmplificationTests.cpp
    integration/StructuralOpIndexRebuildTests.cpp
    integration/AppleDoubleSidecarTests.cpp
    integration/SaveSceneTests.cpp
    integration/ExternalChangeTests.cpp
    integration/GitSnapshotTests.cpp
    integration/MvpLoopTests.cpp
    integration/ApplyRepairTests.cpp
    integration/ObjectCrudTests.cpp
    integration/ObjectCApiTests.cpp
    integration/InspectorLayoutCApiTests.cpp
    integration/HistoricalEventTagsCApiTests.cpp
    integration/ObjectIndexTests.cpp
    integration/ProjectIndexTests.cpp
    integration/RelationshipTests.cpp
    integration/WorldTests.cpp
    integration/SceneNotesTests.cpp
    integration/AssetTests.cpp
    integration/AssetCApiTests.cpp
    integration/CommentTests.cpp
    integration/InboxTests.cpp
    integration/MultiSceneTests.cpp
    integration/CreateSceneTests.cpp
    integration/DeleteSceneTests.cpp
    integration/MergeSceneTests.cpp
    integration/RenameSceneTests.cpp
    integration/ReorderTests.cpp
    integration/ChapterSlugCollisionTests.cpp
    integration/ChapterMigrationTests.cpp
    integration/SceneIdentityTests.cpp
    integration/SceneSplitRepro.cpp
    integration/TimelineTests.cpp
    integration/SearchableContentTests.cpp
    integration/HistoryCApiTests.cpp
    integration/BuffersCApiTests.cpp
    integration/FragmentExtractTests.cpp
    integration/FragmentPasteTests.cpp
    integration/FragmentCutTests.cpp
    integration/EncryptedSecureStoreTests.cpp
)

# SP-144 / [I-0231] — the read-amplification bounds live HERE, not in the test
# source, so that tightening one is a visible, reviewable change rather than an
# edit buried in an assertion.
#
# ⚠️ EVERY VALUE BELOW IS A POST-FIX MEASUREMENT PLUS STATED HEADROOM.
# ⛔ Raising one to make a test pass is the defect, not the fix — the Sprint
# requires a recorded REASON, not a quiet bump.
target_compile_definitions(ScriviCoreTests PRIVATE
    # MEASURED 2: `migrateScenes` reads a chapter sidecar, then REWRITES it when
    # it rebuilds the scenes[] cache, and the rewrite correctly invalidates the
    # cached copy — so the next pass re-reads the NEW bytes. ✅ That second read
    # is the repair working, not amplification. Was 8 before the fix.
    SCRIVI_AMP_MAX_READS_PER_FILE=3

    # MEASURED 1 across a 24-resolution sweep (12 edges x 2 endpoints x 1 bound
    # world). ⚠️ Was 25 — verified by reverting the fix, not assumed.
    SCRIVI_AMP_MAX_ABSENT_PROBES=2

    # MEASURED 1.71 calls/file small vs 1.56 large — per-file cost FALLS as the
    # manuscript grows. 1.25 leaves room for fixture noise while still failing
    # on any return of per-scene re-resolution, whose signature is a RISING
    # per-file cost ([I-0196]).
    SCRIVI_AMP_SCALING_TOLERANCE=1.25
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
