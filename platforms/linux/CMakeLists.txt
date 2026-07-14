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
find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Widgets)

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
        qml/ProjectWindow.qml
)

target_link_libraries(scrivi_linux
    PRIVATE
        ScriviCore          # brings scrivi.h via its PUBLIC include dir
        Qt6::Quick
        Qt6::Widgets        # QFileDialog folder picker (ScriviBridge::chooseFolder)
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
