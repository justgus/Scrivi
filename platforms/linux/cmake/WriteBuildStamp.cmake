# WriteBuildStamp.cmake — regenerate the build-stamp header.
#
# ⚠️ Run as a custom COMMAND (not a target) so Ninja treats its OUTPUT as a real
# dependency of main.cpp and rebuilds in the SAME pass. Touching main.cpp from a
# custom target instead makes the stamp trail reality by exactly one build — it
# looks like it works until you check it carefully. (Observed 2026-08-30.)
#
# ## ⚠️ The BUILD NUMBER is the point; the timestamp is only context
#
# ⚠️ A timestamp alone makes the user do arithmetic: "did the build I started at
# 15:51 produce the binary stamped 15:51:48, or is that the previous one?"
# ✅ A MONOTONIC COUNTER removes the judgement entirely — deploy says "built 48",
# the app says "build 48", done. (User's suggestion, 2026-08-30.)
#
# ⚠️ The counter lives in the BUILD TREE, not the source tree, so it is never
# committed and never conflicts between machines. It counts builds ON THIS RIG,
# which is exactly the question being asked ("is the rig running what I just
# pushed?") and deliberately NOT a release version.

set(COUNTER_FILE "${OUTFILE}.counter")

set(BUILD_NUMBER 0)
if(EXISTS "${COUNTER_FILE}")
    file(READ "${COUNTER_FILE}" BUILD_NUMBER)
    string(STRIP "${BUILD_NUMBER}" BUILD_NUMBER)
endif()
# ⚠️ Guard against a truncated or hand-edited counter: a non-numeric value would
# otherwise fail the MATH() and break the build over a cosmetic feature.
if(NOT BUILD_NUMBER MATCHES "^[0-9]+$")
    set(BUILD_NUMBER 0)
endif()
math(EXPR BUILD_NUMBER "${BUILD_NUMBER} + 1")
file(WRITE "${COUNTER_FILE}" "${BUILD_NUMBER}")

string(TIMESTAMP STAMP "%Y-%m-%d %H:%M:%S" UTC)
file(WRITE "${OUTFILE}"
     "#pragma once\n// Generated. Do not edit.\n"
     "#define SCRIVI_BUILD_NUMBER ${BUILD_NUMBER}\n"
     "#define SCRIVI_BUILD_STAMP \"${STAMP} UTC\"\n")
