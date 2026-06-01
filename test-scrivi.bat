ECHO OFF
cmake -S . -B build\windows-debug -G "Visual Studio 17 2022" -A x64 -DSCRIVI_BUILD_TESTS=ON
cmake --build build\windows-debug --config Debug
ctest --test-dir build\windows-debug -C Debug --output-on-failure