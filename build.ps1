<#
.SYNOPSIS
    Build script for Agent-Sentinel (CUDA/Ninja).
    Ensures environment dependencies are met and builds the project from scratch.
#>

$ErrorActionPreference = "Stop"

function Write-Header {
    param($Text)
    Write-Host "`n=== $Text ===" -ForegroundColor Cyan
}

function Check-Command {
    param($Cmd, $Url)
    if (!(Get-Command $Cmd -ErrorAction SilentlyContinue)) {
        Write-Host "[ERROR] $Cmd not found! Please download it from: $Url" -ForegroundColor Red
        return $false
    }
    Write-Host "[OK] $Cmd found." -ForegroundColor Green
    return $true
}

Write-Header "Checking Prerequisites"
$allOk = $true
$allOk = $allOk -and (Check-Command "cmake" "https://cmake.org/download/")
$allOk = $allOk -and (Check-Command "ninja" "https://github.com/ninja-build/ninja/releases")
$allOk = $allOk -and (Check-Command "git" "https://git-scm.com/downloads")
$allOk = $allOk -and (Check-Command "nvcc" "https://developer.nvidia.com/cuda-downloads")

if (!$allOk) {
    Write-Host "`nPlease install missing tools and restart the script." -ForegroundColor Yellow
    exit 1
}

Write-Header "Locating Visual Studio"
$vsPath = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vsPath) {
    Write-Host "[ERROR] Visual Studio C++ tools not found! Please install VS 2022 or 2026." -ForegroundColor Red
    exit 1
}
$vcvars = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
Write-Host "[OK] Found Visual Studio at: $vsPath" -ForegroundColor Green

Write-Header "Checking Dependencies"
$repoRoot = Get-Location
$llamaPath = Join-Path $repoRoot.Path "..\llama.cpp"

if (!(Test-Path $llamaPath)) {
    Write-Host "llama.cpp not found at $llamaPath. Cloning..." -ForegroundColor Yellow
    git clone https://github.com/ggerganov/llama.cpp $llamaPath
}

Write-Header "Building llama.cpp (CUDA enabled)"
Push-Location $llamaPath
if (!(Test-Path "build")) {
    cmd /c "call `"$vcvars`" && cmake -B build -G Ninja -DGGML_CUDA=ON -DCMAKE_BUILD_TYPE=Release"
}
cmd /c "call `"$vcvars`" && cmake --build build --config Release"
Pop-Location

Write-Header "Building Agent-Sentinel"
if (!(Test-Path "build")) {
    cmd /c "call `"$vcvars`" && cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release"
}
cmd /c "call `"$vcvars`" && cmake --build build --config Release"

Write-Header "5. Success Confirmation"
if (Test-Path "build\agentic.exe") {
    Write-Host "`nSUCCESS: agentic.exe built successfully!" -ForegroundColor Green
    Write-Host "Run it with: .\build\agentic.exe"
} else {
    Write-Host "`nERROR: Build failed. agentic.exe not found." -ForegroundColor Red
    exit 1
}
