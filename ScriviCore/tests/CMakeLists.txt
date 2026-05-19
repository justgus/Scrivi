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
)

target_link_libraries(ScriviCoreTests
    PRIVATE
        ScriviCore
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(ScriviCoreTests)
