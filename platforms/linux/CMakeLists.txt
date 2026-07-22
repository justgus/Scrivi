# Scrivi — Linux Qt/QML app
#
# Built only when -DSCRIVI_BUILD_LINUX=ON is passed to the root configure.
# Links the ScriviCore STATIC target from the parent build (no adapter — direct
# C ABI calls via scrivi.h, which ScriviCore exposes as a PUBLIC include).
#
# Toolchain pin (EP-020 Step-0, 2026-07-13): Ubuntu 24.04 LTS + Qt 6.4 (apt).
# See platforms/linux/README.md.

# Qt 6.4 is the pinned floor (Ubuntu 24.04 apt). Quick pulls in Qml/Gui/Core.
# Widgets backs the New Project folder picker: ScriviBridge::chooseFolder uses a
# Qt Widgets QFileDialog (SP-059 / T-0227) instead of the Qt Quick FolderDialog,
# which could not select the folder being viewed and needed the extra
# Qt.labs.folderlistmodel runtime module.
# QuickWidgets provides QQuickWidget — the SP-061 Widgets-host embeds the landing
# QML in a QQuickWidget inside a QMainWindow (the shell flip).
find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Widgets QuickWidgets)

# Qt's AUTOMOC etc. are enabled per-target below via qt_standard_project_setup.
qt_standard_project_setup(REQUIRES 6.4)

qt_add_executable(scrivi_linux
    src/main.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/RecentsStore.cpp
    src/RecentsStore.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
    # SP-061 (EP-022): the Widgets host shell + native editor screen.
    src/ScriviWindow.cpp
    src/ScriviWindow.hpp
    src/EditorShell.cpp
    src/EditorShell.hpp
    src/SceneDocument.cpp
    src/SceneDocument.hpp
    # SP-062 (EP-022): the editable writing surface with boundary integrity.
    src/ManuscriptEditor.cpp
    src/ManuscriptEditor.hpp
    # SP-067 (EP-023): the scene navigator with app-mediated drag-reorder.
    src/NavigatorTree.cpp
    src/NavigatorTree.hpp
)

qt_add_qml_module(scrivi_linux
    URI Scrivi
    VERSION 1.0
    # Pin the qrc prefix so the load URL is deterministic across Qt minor
    # versions (6.4's default is :/<URI>, later versions default to :/qt/qml/).
    # main.cpp loads qrc:/Scrivi/qml/Landing.qml.
    RESOURCE_PREFIX /
    QML_FILES
        qml/Landing.qml
        qml/NewProjectDialog.qml
)

target_link_libraries(scrivi_linux
    PRIVATE
        ScriviCore          # brings scrivi.h via its PUBLIC include dir
        Qt6::Quick
        Qt6::Widgets        # QFileDialog picker + QMainWindow/QPlainTextEdit shell
        Qt6::QuickWidgets   # QQuickWidget host for the landing QML (shell flip)
)

# The qmltyperegistrar-generated TU (scrivi_linux_qmltyperegistrations.cpp) is
# emitted at the build-tree root and #includes "ScriviBridge.hpp" by bare name.
# On Qt 6.4 it doesn't inherit per-file include dirs, so add src/ explicitly so
# both that generated file and our own sources resolve the header.
target_include_directories(scrivi_linux PRIVATE src)

# Match ScriviCore's C++ standard so the shared headers compile identically.
target_compile_features(scrivi_linux PRIVATE cxx_std_23)

# --- Headless persistence smoke (SP-059 / T-0228) -------------------------
# A console (no-GUI) harness that exercises the same AppSupport + RecentsStore +
# ScriviBridge code the app uses, proving identity (AC4) and recents (AC5)
# persist across a simulated restart. Driven by the persistence_smoke.sh script
# (two passes against one temp $XDG_DATA_HOME). Does not open a window — but links
# Widgets because ScriviBridge references QFileDialog (chooseFolder), unused here.
qt_add_executable(scrivi_linux_persistence_smoke
    tests/persistence_smoke.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/RecentsStore.cpp
    src/RecentsStore.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
)

target_link_libraries(scrivi_linux_persistence_smoke
    PRIVATE
        ScriviCore
        Qt6::Core
        Qt6::Qml            # ScriviBridge/RecentsStore headers include
                            # <qqmlintegration.h> for QML_ELEMENT (no GUI/display)
        Qt6::Widgets        # ScriviBridge references QFileDialog (link-only here)
)

target_include_directories(scrivi_linux_persistence_smoke PRIVATE src)
target_compile_features(scrivi_linux_persistence_smoke PRIVATE cxx_std_23)

# --- Headless full-loop lifecycle smoke (SP-060 / T-0233) -----------------
# Drives create → open (ready) → close → reopen-recent (and across a restart)
# through the same ScriviBridge/RecentsStore code the app uses. Driven by
# lifecycle_smoke.sh. Same link set as the persistence smoke (no window).
qt_add_executable(scrivi_linux_lifecycle_smoke
    tests/lifecycle_smoke.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/RecentsStore.cpp
    src/RecentsStore.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
)

target_link_libraries(scrivi_linux_lifecycle_smoke
    PRIVATE
        ScriviCore
        Qt6::Core
        Qt6::Qml
        Qt6::Widgets        # ScriviBridge references QFileDialog (link-only here)
)

target_include_directories(scrivi_linux_lifecycle_smoke PRIVATE src)
target_compile_features(scrivi_linux_lifecycle_smoke PRIVATE cxx_std_23)

# --- Headless scene-load smoke (SP-061 / T-0237, EP-022 AC1/AC2) -----------
# Builds a 3-scene fixture (distinct bodies via the C ABI), then replicates
# EditorShell::load's assembly — scrivi_open_project + scrivi_open_scene loop +
# SceneDocument::build — and asserts every body landed in the one QTextDocument at
# the offset the map records. Driven by scene_load_smoke.sh. Links Qt6::Gui for
# QTextDocument (offscreen platform via QT_QPA_PLATFORM=offscreen in the script).
qt_add_executable(scrivi_linux_scene_load_smoke
    tests/scene_load_smoke.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
    src/SceneDocument.cpp
    src/SceneDocument.hpp
)

target_link_libraries(scrivi_linux_scene_load_smoke
    PRIVATE
        ScriviCore
        Qt6::Core
        Qt6::Gui            # QTextDocument / QTextCursor (SceneDocument)
        Qt6::Qml            # ScriviBridge header pulls <qqmlintegration.h>
        Qt6::Widgets        # ScriviBridge references QFileDialog (link-only here)
)

target_include_directories(scrivi_linux_scene_load_smoke PRIVATE src)
target_compile_features(scrivi_linux_scene_load_smoke PRIVATE cxx_std_23)

# --- Headless editable-map smoke (SP-062 / T-0238) -------------------------
# Pure SceneDocument logic (no project I/O): builds a 3-scene fixture and asserts
# the boundary classification (editable body vs. protected heading/separator) and
# the offset-map maintenance on insert + delete. Guards the edit-routing authority
# that keeps edits inside their owning scene. Driven by editor_map_smoke.sh.
qt_add_executable(scrivi_linux_editor_map_smoke
    tests/editor_map_smoke.cpp
    src/SceneDocument.cpp
    src/SceneDocument.hpp
)

target_link_libraries(scrivi_linux_editor_map_smoke
    PRIVATE
        Qt6::Core
        Qt6::Gui            # QTextDocument / QTextCursor (SceneDocument)
        Qt6::Widgets        # QPlainTextDocumentLayout (SceneDocument ctor)
)

target_include_directories(scrivi_linux_editor_map_smoke PRIVATE src)
target_compile_features(scrivi_linux_editor_map_smoke PRIVATE cxx_std_23)

# --- Headless per-scene save smoke (SP-062 / T-0239, EP-022 AC3) -----------
# Drives create → open → saveScene(new body) → reopen and asserts the new bytes
# came back (via open_scene and via the reopened activeScene) — proving
# scrivi_save_scene wrote the real .md through the same ScriviBridge::saveScene the
# editor's auto-save uses. Driven by scene_save_smoke.sh (no display).
qt_add_executable(scrivi_linux_scene_save_smoke
    tests/scene_save_smoke.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
)

target_link_libraries(scrivi_linux_scene_save_smoke
    PRIVATE
        ScriviCore
        Qt6::Core
        Qt6::Qml            # ScriviBridge header pulls <qqmlintegration.h>
        Qt6::Widgets        # ScriviBridge references QFileDialog (link-only here)
)

target_include_directories(scrivi_linux_scene_save_smoke PRIVATE src)
target_compile_features(scrivi_linux_scene_save_smoke PRIVATE cxx_std_23)

# --- Headless in-editor create-scene smoke (SP-062 / T-0240, EP-022 AC3) ---
# Drives the ⌘↩ create path headlessly: bridge.createScene after the active scene
# (asserts it persists in order on reopen) + SceneDocument::insertSceneAfter (asserts
# the new empty scene splices into the map and later offsets shift). Needs Widgets
# for QApplication + QPlainTextDocumentLayout (SceneDocument). scene_create_smoke.sh.
qt_add_executable(scrivi_linux_scene_create_smoke
    tests/scene_create_smoke.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
    src/SceneDocument.cpp
    src/SceneDocument.hpp
)

target_link_libraries(scrivi_linux_scene_create_smoke
    PRIVATE
        ScriviCore
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Widgets
)

target_include_directories(scrivi_linux_scene_create_smoke PRIVATE src)
target_compile_features(scrivi_linux_scene_create_smoke PRIVATE cxx_std_23)

# --- Headless in-navigator delete smoke (SP-065 / T-0253, EP-023 AC1/AC2) ---
# Drives the navigator Delete path headlessly: builds a 4-scene/2-chapter fixture,
# splices scenes/chapters out of a SceneDocument via removeScene / removeChapter
# (asserts the offset map stays well-formed and survivors keep their bodies), then
# deletes via bridge.deleteScene / deleteChapter and asserts the removals persist on
# reopen. Needs Widgets for QApplication + QPlainTextDocumentLayout (SceneDocument).
# scene_delete_smoke.sh.
qt_add_executable(scrivi_linux_scene_delete_smoke
    tests/scene_delete_smoke.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
    src/SceneDocument.cpp
    src/SceneDocument.hpp
)

target_link_libraries(scrivi_linux_scene_delete_smoke
    PRIVATE
        ScriviCore
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Widgets
)

target_include_directories(scrivi_linux_scene_delete_smoke PRIVATE src)
target_compile_features(scrivi_linux_scene_delete_smoke PRIVATE cxx_std_23)

# --- Headless in-navigator rename smoke (SP-066 / T-0257, EP-023 AC3) -------
# Drives the navigator Rename path headlessly: builds a 2-chapter fixture, renames a
# scene + a chapter via bridge.renameScene / renameChapter (asserts the sidecar title
# round-trips on reopen, incl. blank→cleared), and applies the label/heading update to
# a SceneDocument (setSceneTitle: no document change; setChapterTitle: live heading
# rewrite via reflowBoundaryAt with the offset map staying well-formed). Needs Widgets
# for QApplication + QPlainTextDocumentLayout (SceneDocument). scene_rename_smoke.sh.
qt_add_executable(scrivi_linux_scene_rename_smoke
    tests/scene_rename_smoke.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
    src/SceneDocument.cpp
    src/SceneDocument.hpp
)

target_link_libraries(scrivi_linux_scene_rename_smoke
    PRIVATE
        ScriviCore
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Widgets
)

target_include_directories(scrivi_linux_scene_rename_smoke PRIVATE src)
target_compile_features(scrivi_linux_scene_rename_smoke PRIVATE cxx_std_23)

# --- Headless scene/chapter reorder smoke (SP-067 / T-0263, EP-023 AC4 + ---
# --- I-0063/I-0064) --------------------------------------------------------
# Builds a 5-scene/2-chapter fixture, splices scenes within + across chapters via
# SceneDocument::moveScene (offset map well-formed, bodies preserved, chapter-first
# re-promotion), persists a cross-chapter reorder via ScriviBridge::reorderScene
# (asserts the new order on reopen), replays the I-0064 chapter-split orchestration
# (create_chapter → reorder_chapter → reorder_scene followers → save head/tail) for the
# end-of-scene + mid-scene cases, and renumbers created "Chapter N" chapters (I-0063).
# Needs Widgets for QApplication + QPlainTextDocumentLayout (SceneDocument).
# scene_reorder_smoke.sh.
qt_add_executable(scrivi_linux_scene_reorder_smoke
    tests/scene_reorder_smoke.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
    src/SceneDocument.cpp
    src/SceneDocument.hpp
)

target_link_libraries(scrivi_linux_scene_reorder_smoke
    PRIVATE
        ScriviCore
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Widgets
)

target_include_directories(scrivi_linux_scene_reorder_smoke PRIVATE src)
target_compile_features(scrivi_linux_scene_reorder_smoke PRIVATE cxx_std_23)

# --- Headless chapter-reorder smoke (SP-073 / T-0296, EP-023 AC5) -----------
# Chapter drag-reorder as a container: SceneDocument::moveChapter block splice (map
# well-formed, bodies + member order preserved, no-op landings refused),
# ScriviBridge::reorderChapter persistence (reopen order + bodies), and the SP-073
# stale-path refresh — the reorder reslugs the chapter's folder, so a save must go
# through the refreshed paths (the I-0074/I-0079 class). Needs Widgets for
# QApplication + QPlainTextDocumentLayout (SceneDocument). chapter_reorder_smoke.sh.
qt_add_executable(scrivi_linux_chapter_reorder_smoke
    tests/chapter_reorder_smoke.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
    src/SceneDocument.cpp
    src/SceneDocument.hpp
)

target_link_libraries(scrivi_linux_chapter_reorder_smoke
    PRIVATE
        ScriviCore
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Widgets
)

target_include_directories(scrivi_linux_chapter_reorder_smoke PRIVATE src)
target_compile_features(scrivi_linux_chapter_reorder_smoke PRIVATE cxx_std_23)

# --- Headless scene/chapter merge smoke (SP-076 / T-0306, EP-028 AC5/AC6/AC7) --
# Builds a 5-scene/2-chapter fixture, then through the bridge (the layer EditorShell's
# merge slots call): merges a scene into the previous same-chapter scene via
# ScriviBridge::mergeScene (asserts on reopen that the absorbed scene's files are gone
# and the survivor holds both bodies, blank-line joined); and merges a whole chapter into
# the previous one via ScriviBridge::mergeChapter (asserts every scene relocated —
# nothing lost, the I-0083 fix — the emptied chapter removed, bodies intact). Needs
# Widgets for QApplication. scene_merge_smoke.sh.
qt_add_executable(scrivi_linux_scene_merge_smoke
    tests/scene_merge_smoke.cpp
    src/AppSupport.cpp
    src/AppSupport.hpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
    src/SceneDocument.cpp
    src/SceneDocument.hpp
)

target_link_libraries(scrivi_linux_scene_merge_smoke
    PRIVATE
        ScriviCore
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Widgets
)

target_include_directories(scrivi_linux_scene_merge_smoke PRIVATE src)
target_compile_features(scrivi_linux_scene_merge_smoke PRIVATE cxx_std_23)

# --- Headless quit-path smoke (SP-062 regression fix) ----------------------
# Guards the shell-flip regression where the landing Quit button did nothing: a
# QQuickWidget's engine doesn't auto-wire QQmlEngine::quit() to the app, so main.cpp
# connects it. This asserts that emitting QQmlEngine::quit() exits the app(0).
# quit_smoke.sh (offscreen platform). Needs QuickWidgets for QQuickWidget.
qt_add_executable(scrivi_linux_quit_smoke
    tests/quit_smoke.cpp
)

target_link_libraries(scrivi_linux_quit_smoke
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Widgets
        Qt6::QuickWidgets
)

target_include_directories(scrivi_linux_quit_smoke PRIVATE src)
target_compile_features(scrivi_linux_quit_smoke PRIVATE cxx_std_23)
