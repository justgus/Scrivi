cmake_minimum_required(VERSION 3.24)

project(Scrivi LANGUAGES CXX)

# Skip network update checks for FetchContent deps when already downloaded.
# Allows cmake -S . -B build to succeed without internet after the first run.
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

option(SCRIVI_BUILD_TESTS "Build ScriviCore tests" ON)
option(SCRIVI_BUILD_LINUX "Build the Linux Qt/QML app (platforms/linux)" OFF)

# Undefined-behaviour / address sanitizers (T-0413, SP-106). OFF by default so
# ordinary builds stay fast; CI runs one matrix leg with this ON.
#
# WHY THIS EXISTS (I-0121): rebalancedKeys(1) divided by zero for 17 days without
# any local build noticing. Integer division by zero is UB, and UB is allowed to
# differ by target: arm64 `sdiv` quietly yields 0, x86-64 `idiv` raises #DE →
# SIGFPE. So the developer's Mac and the macos-latest runner were green while
# ubuntu-latest crashed, and "all tests pass" meant "all tests pass on arm64".
#
# A sanitizer removes the architecture from the question: the check is emitted in
# the code, so the SAME defect fails on EVERY target. -fno-sanitize-recover=all
# makes it abort rather than print-and-continue, so ctest actually fails.
option(SCRIVI_ENABLE_SANITIZERS "Build with UndefinedBehaviorSanitizer + AddressSanitizer" OFF)

if(SCRIVI_ENABLE_SANITIZERS)
    set(SCRIVI_SANITIZER_FLAGS
        -fsanitize=undefined,address
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer)
    add_compile_options(${SCRIVI_SANITIZER_FLAGS})
    add_link_options(${SCRIVI_SANITIZER_FLAGS})
    message(STATUS "ScriviCore: sanitizers ENABLED (undefined,address; no-recover)")
endif()

if(SCRIVI_BUILD_TESTS)
    enable_testing()
else()
    # Loud, because a silently test-less build directory has now bitten twice
    # (SP-091, SP-092): ctest keeps passing by replaying a CACHED binary, so newly
    # added tests appear green without ever being compiled — in SP-092 that hid
    # seven real compile errors. If you see this, reconfigure:
    #     cmake -S . -B build -DSCRIVI_BUILD_TESTS=ON
    message(WARNING
        "SCRIVI_BUILD_TESTS=OFF — ScriviCoreTests will NOT be built. "
        "`ctest` may still report PASS from a stale cached binary. "
        "Reconfigure with -DSCRIVI_BUILD_TESTS=ON before trusting test results.")
endif()

add_subdirectory(ScriviCore)

# The Linux Qt/QML app is opt-in so the default ScriviCore build/CI stays lean and
# Qt-free. Enable with: cmake -S . -B build -DSCRIVI_BUILD_LINUX=ON
if(SCRIVI_BUILD_LINUX)
    add_subdirectory(platforms/linux)
endif()
