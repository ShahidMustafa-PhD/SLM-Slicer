#!/usr/bin/env pwsh
<#
.SYNOPSIS
    MarcSLM Debug Build and Installation Script
.DESCRIPTION
    Automates the complete build and installation process for MarcSLM in Debug mode
    with all dependencies properly deployed to the install-debug folder.
.EXAMPLE
    .\build_debug.ps1
.NOTES
    Requirements:
    - PowerShell 5.0 or later
    - CMake 3.28+
    - Ninja build system
    - Qt6 with development tools
    - Visual Studio 2019+ with MSVC
#>

param(
    [switch]$Clean = $false,
    [switch]$NoInstall = $false,
    [switch]$NoVerify = $false,
    [switch]$LaunchApp = $false
)

# ==========================================
# CONFIGURATION
# ==========================================

$ErrorActionPreference = "Stop"
$VerbosePreference = "Continue"

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommandPath
$BuildDir = Join-Path $ProjectRoot "build"
$InstallDir = Join-Path $ProjectRoot "install-debug"
$CMakeGenerator = "Ninja"
$BuildType = "Debug"
$VcpkgTriplet = "x64-windows-vcpkg"

Write-Host "`n============================================"
Write-Host "  MarcSLM Debug Build System (PowerShell)" -ForegroundColor Green
Write-Host "============================================`n"

Write-Host "Configuration:"
Write-Host "  Generator: $CMakeGenerator"
Write-Host "  Build type: $BuildType"
Write-Host "  Build dir: $BuildDir"
Write-Host "  Install dir: $InstallDir"
Write-Host ""

# ==========================================
# HELPER FUNCTIONS
# ==========================================

function Test-Command {
    param([string]$Command)
    $null = Get-Command $Command -ErrorAction SilentlyContinue
    return $?
}

function Find-Qt {
    Write-Host "Searching for Qt6 installation..."
    $QtDirs = Get-ChildItem "C:\Qt" -Filter "6.*" -Directory -ErrorAction SilentlyContinue
    
    foreach ($QtDir in $QtDirs) {
        $MsvcDir = Join-Path $QtDir "msvc2019_64"
        if (Test-Path $MsvcDir) {
            Write-Host "  ? Found Qt at: $MsvcDir" -ForegroundColor Green
            return $MsvcDir
        }
    }
    
    Write-Host "  ? Qt6 not found in standard location" -ForegroundColor Yellow
    return $null
}

function Check-Prerequisites {
    Write-Host "`nChecking prerequisites..."
    
    $missing = @()
    
    if (-not (Test-Command "cmake")) {
        $missing += "CMake"
    } else {
        Write-Host "  ? CMake found" -ForegroundColor Green
    }
    
    if (-not (Test-Command "ninja")) {
        $missing += "Ninja"
    } else {
        Write-Host "  ? Ninja found" -ForegroundColor Green
    }
    
    if ($missing.Count -gt 0) {
        Write-Host "`n  ? Missing tools: $($missing -join ', ')" -ForegroundColor Red
        Write-Host "  Please install missing tools and add to PATH" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "  ? All prerequisites found" -ForegroundColor Green
}

function Clean-Build {
    Write-Host "`n[1/5] Cleaning old build artifacts..."
    
    if (Test-Path $BuildDir) {
        Write-Host "  Removing $BuildDir"
        Remove-Item $BuildDir -Recurse -Force -ErrorAction SilentlyContinue
    }
    
    if (Test-Path $InstallDir) {
        Write-Host "  Removing $InstallDir"
        Remove-Item $InstallDir -Recurse -Force -ErrorAction SilentlyContinue
    }
    
    New-Item $BuildDir -ItemType Directory -Force | Out-Null
    Write-Host "  ? Clean complete" -ForegroundColor Green
}

function Configure-CMake {
    Write-Host "`n[2/5] Configuring CMake..."
    
    Push-Location $BuildDir
    
    $QtPath = Find-Qt
    
    $CmakeArgs = @(
        "-G", $CMakeGenerator,
        "-DCMAKE_BUILD_TYPE=$BuildType",
        "-DCMAKE_INSTALL_PREFIX=$InstallDir",
        "-DVCPKG_TARGET_TRIPLET=$VcpkgTriplet"
    )
    
    if ($QtPath) {
        $CmakeArgs += @("-DQt6_DIR=$QtPath/lib/cmake/Qt6")
    }
    
    $CmakeArgs += $ProjectRoot
    
    Write-Host "  Running: cmake $($CmakeArgs -join ' ')"
    
    & cmake @CmakeArgs
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  ? CMake configuration failed" -ForegroundColor Red
        exit 1
    }
    
    Pop-Location
    Write-Host "  ? CMake configuration complete" -ForegroundColor Green
}

function Build-Project {
    Write-Host "`n[3/5] Building project..."
    
    Push-Location $BuildDir
    
    & ninja MarcSLM
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  ? Build failed" -ForegroundColor Red
        exit 1
    }
    
    Pop-Location
    Write-Host "  ? Build complete" -ForegroundColor Green
}

function Install-Project {
    Write-Host "`n[4/5] Installing to $InstallDir..."
    
    Push-Location $BuildDir
    
    & cmake --install . --prefix $InstallDir --config $BuildType
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  ? Installation failed" -ForegroundColor Red
        exit 1
    }
    
    Pop-Location
    Write-Host "  ? Installation complete" -ForegroundColor Green
}

function Verify-Installation {
    Write-Host "`n[5/5] Verifying installation..."
    
    $allGood = $true
    
    # Critical files
    $criticalFiles = @(
        "bin\MarcSLM.exe",
        "bin\platforms\qwindows.dll",
        "bin\Qt6Core.dll",
        "bin\Qt6Gui.dll",
        "bin\Qt6Widgets.dll"
    )
    
    foreach ($file in $criticalFiles) {
        $filePath = Join-Path $InstallDir $file
        if (Test-Path $filePath) {
            Write-Host "  ? Found: $file" -ForegroundColor Green
        } else {
            Write-Host "  ? Missing: $file" -ForegroundColor Red
            $allGood = $false
        }
    }
    
    return $allGood
}

function Show-Summary {
    param([bool]$Success)
    
    Write-Host "`n============================================"
    if ($Success) {
        Write-Host "  BUILD AND INSTALLATION SUCCESSFUL!" -ForegroundColor Green
    } else {
        Write-Host "  BUILD AND INSTALLATION COMPLETE WITH WARNINGS" -ForegroundColor Yellow
        Write-Host "  Some dependencies may be missing" -ForegroundColor Yellow
    }
    Write-Host "============================================`n"
    
    Write-Host "Installation location: $InstallDir"
    Write-Host ""
    Write-Host "To run the application:"
    Write-Host "  1. Open Command Prompt or PowerShell"
    Write-Host "  2. Navigate to: $InstallDir\bin"
    Write-Host "  3. Run: MarcSLM.exe"
    Write-Host ""
    Write-Host "Or use PowerShell:"
    Write-Host "  & '$InstallDir\bin\MarcSLM.exe'"
    Write-Host ""
}

function Launch-Application {
    Write-Host "Launching application..."
    Push-Location (Join-Path $InstallDir "bin")
    Start-Process ".\MarcSLM.exe"
    Pop-Location
}

# ==========================================
# MAIN EXECUTION
# ==========================================

try {
    Check-Prerequisites
    
    if ($Clean) {
        Clean-Build
    } elseif (-not (Test-Path $BuildDir)) {
        Clean-Build
    }
    
    Configure-CMake
    Build-Project
    
    if (-not $NoInstall) {
        Install-Project
    }
    
    $verifyOk = $true
    if (-not $NoVerify) {
        $verifyOk = Verify-Installation
    }
    
    Show-Summary $verifyOk
    
    if ($LaunchApp -and (Test-Path (Join-Path $InstallDir "bin\MarcSLM.exe"))) {
        Launch-Application
    }
}
catch {
    Write-Host "`n? Error: $_" -ForegroundColor Red
    Write-Host "Stack trace: $($_.ScriptStackTrace)" -ForegroundColor Red
    exit 1
}
