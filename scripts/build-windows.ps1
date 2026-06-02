$ErrorActionPreference = "Stop"

# Guard: this script must run locally on the Win11 development machine.
# It cannot be invoked remotely — the CMake build, MSVC toolchain, and test
# binaries all require local execution. Do not run this when Win11 is
# unreachable (i.e. when off the local development network).
if (-not (Test-Path "C:\Windows\System32\cmd.exe")) {
    Write-Error "This script must run locally on the Win11 machine. Aborting."
    exit 1
}

$RepoRoot = $PSScriptRoot | Split-Path -Parent
$BuildDir = Join-Path $RepoRoot "build\windows-debug"

Write-Host "==> Configuring..." -ForegroundColor Cyan
cmake -S $RepoRoot -B $BuildDir `
    -G "Visual Studio 17 2022" `
    -A x64 `
    -DSCRIVI_BUILD_TESTS=ON
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Building..." -ForegroundColor Cyan
cmake --build $BuildDir --config Debug
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> Testing..." -ForegroundColor Cyan
ctest --test-dir $BuildDir -C Debug --output-on-failure
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "==> All steps passed." -ForegroundColor Green
