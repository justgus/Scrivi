# Scrivi DevOps and Build Environment Plan v0.1

**Project:** Scrivi  
**Document:** DevOps and Build Environment Plan  
**Version:** 0.1  
**Status:** Draft / Approved Direction Pending Review  
**Date:** 2026-05-18

---

## 1. Purpose

This document defines the development, build, test, and CI environment for Scrivi before implementation begins.

The goal is to make sure ScriviCore can be built and tested consistently on:

```text
macOS
Linux
Windows 11
GitHub Actions
```

before significant backend code is written.

This is a DevOps/bootstrap phase, not a coding phase.

---

## 2. Approved Context

ScriviCore is planned as:

```text
C++24 shared backend core
CMake-based portable build
nlohmann/json for JSON
Catch2 for tests
GitProvider abstraction with SystemGitProvider first
SecureStore abstraction with MockSecureStore for tests and Apple Keychain first
Direct Swift/C++ interop for Apple integration
```

The core must support:

```text
local Mac command-line build
Xcode-based development workflow
Linux Docker build/test on MacBook
native Windows 11 build/test
GitHub Actions CI on macOS, Ubuntu, and Windows
```

---

## 3. High-Level Build Strategy

### 3.1 Canonical build system

The canonical portable build system for ScriviCore is:

```text
CMake
```

### 3.2 Apple development workflow

Apple development should support:

```text
command-line CMake build
CMake-generated Xcode project for core development
future Scrivi.xcworkspace for Apple app shell integration
```

### 3.3 Linux local workflow

Linux local validation should use:

```text
Docker container on the MacBook
Debian or Ubuntu-style build image
mounted source tree
CMake configure/build/test inside container
```

### 3.4 Windows local workflow

Windows validation should use:

```text
native Windows 11 machine
Visual Studio 2022 or Build Tools
Desktop development with C++
CMake
Developer PowerShell / Developer Command Prompt
```

Windows containers are not required.

### 3.5 CI workflow

GitHub Actions should validate:

```text
ubuntu-latest
macos-latest
windows-latest
```

---

## 4. GitHub Repository Setup

### 4.1 Repository creation

Recommended initial repository:

```text
Repository name: Scrivi
Visibility: private initially
Default branch: main
```

Private first is recommended because the project is still in foundational design and early implementation.

The repository can become public later if desired.

### 4.2 Initial repository contents

Before coding, commit:

```text
README.md
LICENSE
.gitignore
CMakeLists.txt
CMakePresets.json
docs/
.github/workflows/ci.yml
devops/docker/linux/Dockerfile
scripts/
ScriviCore/
```

At first, `ScriviCore/` may contain only the build skeleton and empty test target.

### 4.3 Branch protection

Recommended GitHub branch protection for `main`:

```text
Require pull request before merging
Require status checks before merging
Require branches to be up to date before merging
Require conversation resolution before merging
Block force pushes
Block deletions
```

Required status checks should eventually include:

```text
ci / macos
ci / ubuntu
ci / windows
```

Exact check names depend on the final workflow names.

### 4.4 Pull request workflow

Recommended workflow:

```text
feature branches only
no direct pushes to main
small PRs
CI must pass
docs updated with design changes
```

Suggested branch naming:

```text
devops/bootstrap
core/result-type
core/project-creation
core/open-project
core/git-provider
```

---

## 5. macOS Development Environment

### 5.1 Required tools

Install:

```text
Xcode
Xcode Command Line Tools
Git
CMake
Ninja or Make
Docker Desktop
```

### 5.2 Verify Xcode

```bash
xcode-select -p
xcodebuild -version
```

If command line tools are missing:

```bash
xcode-select --install
```

### 5.3 Install Homebrew if needed

Homebrew is not strictly required, but it is the simplest way to install CMake and Ninja on macOS.

Verify:

```bash
brew --version
```

If Homebrew is unavailable, install it from the official Homebrew site.

### 5.4 Install CMake and Ninja

```bash
brew install cmake ninja
```

Verify:

```bash
cmake --version
ninja --version
git --version
```

### 5.5 macOS command-line build

From the repository root:

```bash
cmake -S . -B build/macos-debug \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSCRIVI_BUILD_TESTS=ON

cmake --build build/macos-debug

ctest --test-dir build/macos-debug --output-on-failure
```

### 5.6 macOS clean build

```bash
rm -rf build/macos-debug

cmake -S . -B build/macos-debug \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSCRIVI_BUILD_TESTS=ON

cmake --build build/macos-debug
ctest --test-dir build/macos-debug --output-on-failure
```

---

## 6. Xcode Development Workflow

### 6.1 Xcode role

Xcode should be an Apple development workflow, not the canonical portable build definition.

The source of truth remains:

```text
CMakeLists.txt
ScriviCore source files
```

### 6.2 Generate Xcode project for ScriviCore

From the repository root:

```bash
cmake -S . -B build/xcode \
  -G Xcode \
  -DCMAKE_XCODE_GENERATE_SCHEME=ON \
  -DSCRIVI_BUILD_TESTS=ON
```

Then open:

```bash
open build/xcode/Scrivi.xcodeproj
```

### 6.3 Xcode command-line build

```bash
cmake --build build/xcode --config Debug
ctest --test-dir build/xcode -C Debug --output-on-failure
```

### 6.4 Future `.xcworkspace` strategy

Do not make the initial portable core depend on a hand-authored Xcode workspace.

Later, when the Apple app shell exists, create:

```text
Scrivi.xcworkspace/
  ScriviApple.xcodeproj
  ScriviCore integration
```

The Apple shell should call ScriviCore through direct Swift/C++ interop.

### 6.5 Guardrail

Do not add Apple-only build assumptions to ScriviCore.

ScriviCore should remain buildable by CMake on:

```text
macOS
Linux
Windows
```

---

## 7. Linux Docker Build Environment

### 7.1 Purpose

Use Docker on the MacBook to validate Linux builds locally.

Docker is not used for Windows builds.

### 7.2 Docker directory

Recommended path:

```text
devops/docker/linux/Dockerfile
```

### 7.3 Dockerfile

Initial Debian/Ubuntu-style Dockerfile:

```dockerfile
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    cmake \
    ninja-build \
    git \
    ca-certificates \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work

CMD ["/bin/bash", "-lc", "cmake -S . -B build/linux-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DSCRIVI_BUILD_TESTS=ON && cmake --build build/linux-debug && ctest --test-dir build/linux-debug --output-on-failure"]
```

### 7.4 Build Docker image

From repository root:

```bash
docker build -f devops/docker/linux/Dockerfile -t scrivi-core-linux .
```

### 7.5 Run Linux build/test in container

From repository root:

```bash
docker run --rm \
  -v "$PWD:/work" \
  scrivi-core-linux
```

### 7.6 Interactive container session

```bash
docker run --rm -it \
  -v "$PWD:/work" \
  scrivi-core-linux \
  /bin/bash
```

Inside the container:

```bash
cmake -S . -B build/linux-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSCRIVI_BUILD_TESTS=ON

cmake --build build/linux-debug
ctest --test-dir build/linux-debug --output-on-failure
```

### 7.7 Linux container guardrail

The Linux container validates Linux builds.

It does not replace:

```text
native macOS build
native Windows build
GitHub Actions CI
```

---

## 8. Windows 11 Native Development Environment

### 8.1 Required tools

On the Windows 11 machine, install:

```text
Visual Studio 2022 Community or Build Tools for Visual Studio 2022
Desktop development with C++ workload
C++ CMake tools for Windows
Git for Windows
CMake if not using Visual Studio-bundled CMake
Ninja optional
```

### 8.2 Visual Studio installation

Install Visual Studio 2022 Community or Build Tools.

Select workload:

```text
Desktop development with C++
```

Make sure the following components are installed:

```text
MSVC v143 or newer C++ toolset
Windows SDK
C++ CMake tools for Windows
```

### 8.3 Git for Windows

Install Git for Windows.

Verify in PowerShell:

```powershell
git --version
```

### 8.4 Developer PowerShell

Use:

```text
Developer PowerShell for VS 2022
```

or:

```text
x64 Native Tools Command Prompt for VS 2022
```

This ensures MSVC compiler tools are on the path.

### 8.5 Verify compiler and CMake

In Developer PowerShell:

```powershell
cl
cmake --version
git --version
```

### 8.6 Clone repository

```powershell
git clone https://github.com/<OWNER>/Scrivi.git
cd Scrivi
```

Replace `<OWNER>` with the actual GitHub owner.

### 8.7 Configure/build/test with Visual Studio generator

```powershell
cmake -S . -B build\windows-debug -G "Visual Studio 17 2022" -A x64 -DSCRIVI_BUILD_TESTS=ON

cmake --build build\windows-debug --config Debug

ctest --test-dir build\windows-debug -C Debug --output-on-failure
```

### 8.8 Configure/build/test with Ninja, optional

If Ninja is installed and MSVC environment is active:

```powershell
cmake -S . -B build\windows-ninja-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DSCRIVI_BUILD_TESTS=ON

cmake --build build\windows-ninja-debug

ctest --test-dir build\windows-ninja-debug --output-on-failure
```

### 8.9 Visual Studio CMake workflow

Visual Studio can open a folder containing `CMakeLists.txt` directly.

Workflow:

```text
Open Visual Studio
File → Open → Folder
Select Scrivi repository root
Choose CMake configuration
Build ScriviCore
Run tests
```

### 8.10 Windows guardrails

Do not require Windows containers.

Do not require WSL for native Windows validation.

WSL can be useful later, but it validates Linux-like behavior, not native Windows behavior.

---

## 9. GitHub Actions CI

### 9.1 Goal

CI should build and test ScriviCore on:

```text
Ubuntu
macOS
Windows
```

### 9.2 Workflow path

```text
.github/workflows/ci.yml
```

### 9.3 Initial workflow

```yaml
name: ci

on:
  pull_request:
  push:
    branches:
      - main

jobs:
  build-test:
    name: ${{ matrix.os }} / ${{ matrix.build_type }}
    runs-on: ${{ matrix.os }}

    strategy:
      fail-fast: false
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]
        build_type: [Debug]

    steps:
      - name: Checkout
        uses: actions/checkout@v4

      - name: Configure
        run: cmake -S . -B build -DCMAKE_BUILD_TYPE=${{ matrix.build_type }} -DSCRIVI_BUILD_TESTS=ON

      - name: Build
        run: cmake --build build --config ${{ matrix.build_type }}

      - name: Test
        run: ctest --test-dir build -C ${{ matrix.build_type }} --output-on-failure
```

### 9.4 Notes

This workflow intentionally starts simple.

Later improvements may include:

```text
Ninja explicitly
Release builds
sanitizer builds
coverage
static analysis
macOS Xcode generator check
Windows MSVC version pinning
dependency caching
```

### 9.5 Required CI checks

Once branch protection is enabled, require the CI checks from this workflow before merging.

---

## 10. Dependency Strategy

### 10.1 Approved dependencies

```text
nlohmann/json
Catch2
```

### 10.2 Acquisition strategy

Recommended initial approach:

```text
CMake FetchContent
```

### 10.3 Why FetchContent

Pros:

```text
simple bootstrap
works across macOS/Linux/Windows
avoids requiring system package managers
keeps CI simple
fine for small dependency set
```

Cons:

```text
downloads dependencies during configure
requires network during first configure
version pinning must be explicit
```

### 10.4 Guardrail

Third-party APIs must not leak through ScriviCore public headers.

---

## 11. Proposed CMakePresets.json

Initial concept:

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "macos-debug",
      "displayName": "macOS Debug",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/macos-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "SCRIVI_BUILD_TESTS": "ON"
      }
    },
    {
      "name": "linux-debug",
      "displayName": "Linux Debug",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/linux-debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "SCRIVI_BUILD_TESTS": "ON"
      }
    },
    {
      "name": "xcode-debug",
      "displayName": "Xcode Debug",
      "generator": "Xcode",
      "binaryDir": "${sourceDir}/build/xcode",
      "cacheVariables": {
        "CMAKE_XCODE_GENERATE_SCHEME": "ON",
        "SCRIVI_BUILD_TESTS": "ON"
      }
    }
  ]
}
```

Windows Visual Studio presets can be added after confirming the Windows toolchain details.

---

## 12. Developer Scripts

Scripts are optional but useful.

Recommended paths:

```text
scripts/
  build-macos.sh
  test-macos.sh
  build-xcode.sh
  test-linux-docker.sh
  build-windows.ps1
```

### 12.1 `scripts/build-macos.sh`

```bash
#!/usr/bin/env bash
set -euo pipefail

cmake -S . -B build/macos-debug \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSCRIVI_BUILD_TESTS=ON

cmake --build build/macos-debug
```

### 12.2 `scripts/test-macos.sh`

```bash
#!/usr/bin/env bash
set -euo pipefail

ctest --test-dir build/macos-debug --output-on-failure
```

### 12.3 `scripts/test-linux-docker.sh`

```bash
#!/usr/bin/env bash
set -euo pipefail

docker build -f devops/docker/linux/Dockerfile -t scrivi-core-linux .

docker run --rm \
  -v "$PWD:/work" \
  scrivi-core-linux
```

### 12.4 `scripts/build-windows.ps1`

```powershell
$ErrorActionPreference = "Stop"

cmake -S . -B build\windows-debug -G "Visual Studio 17 2022" -A x64 -DSCRIVI_BUILD_TESTS=ON
cmake --build build\windows-debug --config Debug
ctest --test-dir build\windows-debug -C Debug --output-on-failure
```

---

## 13. Recommended Bootstrap Sequence

Before coding ScriviCore behavior:

1. Create GitHub repository.
2. Commit documentation set.
3. Add top-level `.gitignore`.
4. Add root `CMakeLists.txt`.
5. Add `ScriviCore/CMakeLists.txt`.
6. Add minimal public headers.
7. Add Catch2 test target.
8. Add one trivial test.
9. Add GitHub Actions workflow.
10. Verify CI on Ubuntu/macOS/Windows.
11. Add Linux Dockerfile.
12. Verify Docker build/test locally on MacBook.
13. Verify native Windows build/test on Windows 11 machine.
14. Generate Xcode project from CMake.
15. Verify Xcode can build ScriviCore.
16. Enable branch protection after CI check names are stable.

---

## 14. Environment Verification Checklist

### 14.1 macOS

```bash
xcodebuild -version
xcode-select -p
git --version
cmake --version
ninja --version
docker --version
```

### 14.2 Linux Docker

```bash
docker build -f devops/docker/linux/Dockerfile -t scrivi-core-linux .
docker run --rm -v "$PWD:/work" scrivi-core-linux
```

### 14.3 Windows 11

In Developer PowerShell:

```powershell
cl
cmake --version
git --version
```

Then:

```powershell
cmake -S . -B build\windows-debug -G "Visual Studio 17 2022" -A x64 -DSCRIVI_BUILD_TESTS=ON
cmake --build build\windows-debug --config Debug
ctest --test-dir build\windows-debug -C Debug --output-on-failure
```

### 14.4 GitHub Actions

Verify:

```text
ubuntu-latest passes
macos-latest passes
windows-latest passes
```

---

## 15. Success Criteria

The DevOps/build environment phase is successful if:

1. GitHub repository exists.
2. Documentation set is committed.
3. Local macOS command-line build works.
4. CMake-generated Xcode project builds ScriviCore.
5. Linux Docker build/test works on the MacBook.
6. Native Windows 11 build/test works.
7. GitHub Actions builds/tests on Ubuntu, macOS, and Windows.
8. Branch protection can require CI before merge.
9. Catch2 is test-only.
10. nlohmann/json and Catch2 are acquired consistently.
11. No platform-specific UI code is required to build ScriviCore.
12. No substantive backend coding begins before the build lanes work.

---

## 16. Open Decisions

Before generating the starter files, confirm:

1. GitHub repository owner/name.
2. Repository visibility: private first or public immediately.
3. License choice.
4. Whether to use CMake FetchContent for both nlohmann/json and Catch2.
5. Whether to add GitHub Actions immediately with the initial skeleton.
6. Whether to use Ninja as the default local macOS generator.
7. Whether Docker base should be `ubuntu:24.04` or Debian stable.
8. Whether CI should include Release builds immediately or Debug only.

---

## 17. Recommended Next Artifact

After this plan is approved, generate the initial DevOps starter file set:

```text
.github/workflows/ci.yml
devops/docker/linux/Dockerfile
scripts/build-macos.sh
scripts/test-macos.sh
scripts/test-linux-docker.sh
scripts/build-windows.ps1
CMakePresets.json
```

These starter files should be generated before ScriviCore implementation code.
