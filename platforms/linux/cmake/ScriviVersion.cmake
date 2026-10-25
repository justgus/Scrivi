# ScriviVersion.cmake — [T-0622] write Scrivi's one version into a header, on EVERY build (cmake -P; see CMakeLists.txt).
#
#   SCRIVI_VERSION       "<VERSION file>.<commits on HEAD>"   e.g. "1.0.296"
#   SCRIVI_BUILD_NUMBER  <commits on HEAD>                    e.g. 296
#   SCRIVI_COMMIT        "<short hash>[-dirty]"               e.g. "a1b2c3d"
#
# The same numbers as the Apple build (scripts/stamp-version.sh), so one commit reads the same everywhere.
# deploy-to-rig.sh ships a tree without git history, and puts the Mac's numbers in `.scrivi-version-override`
# ("<count> <hash>") instead; with no git and no override (the Docker image) it reads 1.0.0 (unknown).
# The header is rewritten only when the numbers change, so an unchanged commit recompiles nothing.
file(READ "${SRC}/VERSION" base)
string(STRIP "${base}" base)
set(count 0)
set(hash "unknown")
if(EXISTS "${SRC}/.scrivi-version-override")
    file(READ "${SRC}/.scrivi-version-override" override)
    string(STRIP "${override}" override)
    separate_arguments(parts UNIX_COMMAND "${override}")
    list(GET parts 0 count)
    list(GET parts 1 hash)
else()
    execute_process(COMMAND git -C "${SRC}" rev-list --count HEAD
                    OUTPUT_VARIABLE c RESULT_VARIABLE rc OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(rc EQUAL 0)
        set(count "${c}")
        execute_process(COMMAND git -C "${SRC}" rev-parse --short=7 HEAD
                        OUTPUT_VARIABLE hash OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        execute_process(COMMAND git -C "${SRC}" status --porcelain
                        OUTPUT_VARIABLE dirty OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        if(NOT dirty STREQUAL "")
            string(APPEND hash "-dirty")
        endif()
    endif()
endif()
file(WRITE "${OUT}.tmp" "#pragma once\n// GENERATED at build time by platforms/linux/cmake/ScriviVersion.cmake ([T-0622]) — not in the source tree.\n#define SCRIVI_VERSION \"${base}.${count}\"\n#define SCRIVI_BUILD_NUMBER ${count}\n#define SCRIVI_COMMIT \"${hash}\"\n")
file(COPY_FILE "${OUT}.tmp" "${OUT}" ONLY_IF_DIFFERENT)
file(REMOVE "${OUT}.tmp")
