<#
.SYNOPSIS
Run tests.

.DESCRIPTION
This script builds and runs tests.

.PARAMETER Help
Shows help

.EXAMPLE
.\run_tests.ps1
Builds and runs tests
#>

param(
    [switch]$Help
)

function Main {
    $projectRoot = Join-Path $PSScriptRoot ".." -Resolve
    $buildDir = Join-Path $projectRoot "build"

    if (-not (Test-Path $buildDir)) {
        Write-Error "Error: Build directory does not exist. Run cmake first."
        exit 1
    }

    Write-Host "Building in Release configuration..."
    cmake $projectRoot -B $buildDir
    cmake --build $buildDir --config Release --parallel
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed"
        exit 1
    }

    Set-Location $buildDir
    ctest -C Release --output-on-failure -VV
    Write-Host "Tests completed."
    Set-Location $projectRoot
}

if ($Help) {
    Get-Help $MyInvocation.MyCommand.Path -Detailed
    exit 0
}

Main
