<#
.SYNOPSIS
Build and install the project.

.DESCRIPTION
This script builds the project and optionally installs it to a specified path.

.PARAMETER BuildType
The build configuration (Debug, Release, etc.). Default is "Release".

.PARAMETER LibType
The library type (static, shared). Default is "shared".

.PARAMETER InstallPath
The installation path. If not provided, skips installation.

.PARAMETER Testing
Enable or disable test compilation. Default is OFF.

.PARAMETER Help
Shows help.

.EXAMPLE
.\build_and_install.ps1
Builds in Release configuration with shared libraries and tests disabled.

.EXAMPLE
.\build_and_install.ps1 -BuildType Debug -LibType static -Testing ON
Builds in Debug configuration with static libraries and tests enabled.
#>

param(
    [string]$BuildType = "Release",
    [ValidateSet("static", "shared")]
    [string]$LibType = "shared",
    [string]$InstallPath,
    [ValidateSet("ON", "OFF")]
    [string]$Testing = "OFF",
    [switch]$Help
)

function Main {
    $projectRoot = Join-Path $PSScriptRoot ".." -Resolve
    $buildDir = Join-Path $projectRoot "build"

    # Configure CMake
    Write-Host "Configuring CMake with BuildType=$BuildType, LibType=$LibType, Testing=$Testing..."
    $cmakeArgs = @(
        "-B", $buildDir,
        "-DBUILD_SHARED_LIBS=$($LibType -eq 'shared')",
        "-DINIPARSER_TESTS=$Testing"
    )

    # Only add CMAKE_BUILD_TYPE for single-config generators
    $generator = cmake --help | Select-String "Generators"
    if ($generator -match "Unix Makefiles|Ninja") {
        $cmakeArgs += "-DCMAKE_BUILD_TYPE=$BuildType"
    }

    cmake $projectRoot @cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake configuration failed"
        exit 1
    }

    # Build
    Write-Host "Building..."
    cmake --build $buildDir --config $BuildType --parallel
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed"
        exit 1
    }

    # Install (if path provided)
    if ($InstallPath) {
        Write-Host "Installing to $InstallPath..."
        $installArgs = @(
            "--install", $buildDir,
            "--config", $BuildType,
            "--prefix", $InstallPath
        )
        cmake @installArgs
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Installation failed"
            exit 1
        }
        Write-Host "Installation complete."
    }
    else {
        Write-Host "Skipping installation (no path provided)."
    }
}

if ($Help) {
    Get-Help $MyInvocation.MyCommand.Path -Detailed
    exit 0
}

Main
