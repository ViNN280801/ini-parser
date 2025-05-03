#!/bin/bash

# ==============================================
# build_and_install.sh
#
# Build and install the project with configurable:
# - Build type (Debug/Release)
# - Library type (static/shared)
# - Testing (ON/OFF)
# - Custom install path
# ==============================================

# Default values
BUILD_TYPE="Release"
LIB_TYPE="shared"
TESTING="OFF"
INSTALL_PATH=""
HELP=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
    --build-type)
        BUILD_TYPE="$2"
        shift 2
        ;;
    --lib-type)
        LIB_TYPE="$2"
        shift 2
        ;;
    --install-path)
        INSTALL_PATH="$2"
        shift 2
        ;;
    --testing)
        TESTING="$2"
        shift 2
        ;;
    --help)
        HELP=true
        shift
        ;;
    *)
        echo "Unknown option: $1"
        exit 1
        ;;
    esac
done

# Show help if requested
if [ "$HELP" = true ]; then
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  --build-type <Debug|Release>   Set build type (default: Release)"
    echo "  --lib-type <static|shared>     Set library type (default: shared)"
    echo "  --install-path <path>          Set custom install path"
    echo "  --testing <ON|OFF>             Enable/disable tests (default: OFF)"
    echo "  --help                         Show this help message"
    exit 0
fi

# Validate arguments
if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    echo "Error: Invalid build type. Use Debug or Release."
    exit 1
fi

if [[ "$LIB_TYPE" != "static" && "$LIB_TYPE" != "shared" ]]; then
    echo "Error: Invalid library type. Use static or shared."
    exit 1
fi

if [[ "$TESTING" != "ON" && "$TESTING" != "OFF" ]]; then
    echo "Error: Invalid testing flag. Use ON or OFF."
    exit 1
fi

# Configure and build
echo "Configuring CMake with BuildType=$BUILD_TYPE, LibType=$LIB_TYPE, Testing=$TESTING..."
cmake -B build \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_SHARED_LIBS="$([ "$LIB_TYPE" = "shared" ] && echo "ON" || echo "OFF")" \
    -DINIPARSER_TESTS="$TESTING"

if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed."
    exit 1
fi

echo "Building..."
cmake --build build --config "$BUILD_TYPE" --parallel

if [ $? -ne 0 ]; then
    echo "Error: Build failed."
    exit 1
fi

# Install if path provided
if [ -n "$INSTALL_PATH" ]; then
    echo "Installing to $INSTALL_PATH..."
    cmake --install build --config "$BUILD_TYPE" --prefix "$INSTALL_PATH"

    if [ $? -ne 0 ]; then
        echo "Error: Installation failed."
        exit 1
    fi
    echo "Installation complete."
else
    echo "Skipping installation (no path provided)."
fi
