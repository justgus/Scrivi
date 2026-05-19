# third_party

Third-party dependencies are acquired via CMake FetchContent at configure time.

Current dependencies:
- **Catch2 v3.6.0** — test framework (test targets only)
- **nlohmann/json** — JSON parsing (to be added in T-0003, hidden behind internal Json wrapper)

Do not vendor source archives or submodules here unless FetchContent proves insufficient.
