cmake_minimum_required(VERSION 3.24)

project(Scrivi LANGUAGES CXX)

option(SCRIVI_BUILD_TESTS "Build ScriviCore tests" ON)

if(SCRIVI_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(ScriviCore)
