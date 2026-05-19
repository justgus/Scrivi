add_library(ScriviCore STATIC)

target_compile_features(ScriviCore PUBLIC cxx_std_23)

target_include_directories(ScriviCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(ScriviCore
    PRIVATE
        src/public_api/ScriviCore.cpp
)

if(SCRIVI_BUILD_TESTS)
    add_subdirectory(tests)
endif()
