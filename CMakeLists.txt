cmake_minimum_required(VERSION 3.24)

project(Scrivi LANGUAGES CXX)

# Skip network update checks for FetchContent deps when already downloaded.
# Allows cmake -S . -B build to succeed without internet after the first run.
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

option(SCRIVI_BUILD_TESTS "Build ScriviCore tests" ON)
option(SCRIVI_BUILD_LINUX "Build the Linux Qt/QML app (platforms/linux)" OFF)

if(SCRIVI_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(ScriviCore)

# The Linux Qt/QML app is opt-in so the default ScriviCore build/CI stays lean and
# Qt-free. Enable with: cmake -S . -B build -DSCRIVI_BUILD_LINUX=ON
if(SCRIVI_BUILD_LINUX)
    add_subdirectory(platforms/linux)
endif()
