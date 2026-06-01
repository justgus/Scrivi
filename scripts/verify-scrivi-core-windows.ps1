# Validates ScriviCore (C++) on Windows using Visual Studio 2022.
# Run from Developer PowerShell for VS 2022.
$ErrorActionPreference = "Stop"

$RepoRoot = $PSScriptRoot | Split-Path -Parent
$BuildDir = Join-Path $RepoRoot "build\windows-debug"

Write-Host "==> Configuring ScriviCore..." -ForegroundColor Cyan
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

Write-Host "==> ScriviCore Windows: all tests passed." -ForegroundColor Green
