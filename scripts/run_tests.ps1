<#
.SYNOPSIS
Run tests.

.DESCRIPTION
This script builds and runs tests.

.PARAMETER Help
Shows help

.PARAMETER Architecture
Specifies the architecture to build for

.EXAMPLE
.\run_tests.ps1
Builds and runs tests
#>

param(
    [switch]$Help,
    [string]$Architecture = "x64"  # Default to 64-bit
)

function Main {
    $projectRoot = Join-Path $PSScriptRoot ".." -Resolve
    $buildDir = Join-Path $projectRoot "build"

    # Create build directory if it doesn't exist
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir | Out-Null
    }

    # Clean build directory
    Remove-Item -Recurse -Force $buildDir

    Write-Host "Building in Release configuration for $Architecture..."
    
    # Configure CMake with architecture
    if ($Architecture -eq "x86") {
        cmake $projectRoot -B $buildDir -A Win32
    }
    else {
        cmake $projectRoot -B $buildDir -A x64
    }
    
    cmake --build $buildDir --config Release --parallel
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed"
        exit 1
    }

    Set-Location $buildDir
    ctest -C Release --output-on-failure -VV
    $logPath = Join-Path $buildDir "test.log"
    if (Test-Path $logPath) {
        Write-Host "Tests completed. Here is the info from the log file:"
        Get-Content $logPath
    }
    else {
        Write-Warning "Log file not found at $logPath"
    }
    Set-Location $projectRoot
}

if ($Help) {
    Get-Help $MyInvocation.MyCommand.Path -Detailed
    exit 0
}

Main
