# Scrivi — Linux Qt/QML app
#
# Built only when -DSCRIVI_BUILD_LINUX=ON is passed to the root configure.
# Links the ScriviCore STATIC target from the parent build (no adapter — direct
# C ABI calls via scrivi.h, which ScriviCore exposes as a PUBLIC include).
#
# Toolchain pin (EP-020 Step-0, 2026-07-13): Ubuntu 24.04 LTS + Qt 6.4 (apt).
# See platforms/linux/README.md.

# Qt 6.4 is the pinned floor (Ubuntu 24.04 apt). Quick pulls in Qml/Gui/Core.
find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)

# Qt's AUTOMOC etc. are enabled per-target below via qt_standard_project_setup.
qt_standard_project_setup(REQUIRES 6.4)

qt_add_executable(scrivi_linux
    src/main.cpp
    src/ScriviBridge.cpp
    src/ScriviBridge.hpp
)

qt_add_qml_module(scrivi_linux
    URI Scrivi
    VERSION 1.0
    # Pin the qrc prefix so the load URL is deterministic across Qt minor
    # versions (6.4's default is :/<URI>, later versions default to :/qt/qml/).
    # main.cpp loads qrc:/Scrivi/qml/Main.qml.
    RESOURCE_PREFIX /
    QML_FILES
        qml/Main.qml
)

target_link_libraries(scrivi_linux
    PRIVATE
        ScriviCore          # brings scrivi.h via its PUBLIC include dir
        Qt6::Quick
)

# The qmltyperegistrar-generated TU (scrivi_linux_qmltyperegistrations.cpp) is
# emitted at the build-tree root and #includes "ScriviBridge.hpp" by bare name.
# On Qt 6.4 it doesn't inherit per-file include dirs, so add src/ explicitly so
# both that generated file and our own sources resolve the header.
target_include_directories(scrivi_linux PRIVATE src)

# Match ScriviCore's C++ standard so the shared headers compile identically.
target_compile_features(scrivi_linux PRIVATE cxx_std_23)
