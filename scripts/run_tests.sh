#!/bin/bash

show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "Options:"
    echo "  -h, --help            Show this help message"
    echo "  -a, --arch ARCH       Specify architecture (x86, x64, default: x64)"
    echo
    echo "Description:"
    echo "  This script automatically:"
    echo "  1. Creates build directory if needed"
    echo "  2. Configures project with CMake"
    echo "  3. Builds project"
    echo "  4. Runs tests"
    echo
    echo "Example:"
    echo "  $0 --arch x86"
}

ARCH="x64"

while [[ $# -gt 0 ]]; do
    case "$1" in
    -h | --help)
        show_help
        exit 0
        ;;
    -a | --arch)
        ARCH="$2"
        shift
        ;;
    *)
        echo "Unknown option: $1" >&2
        exit 1
        ;;
    esac
    shift
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

echo "=== Setting up build environment for $ARCH ==="
mkdir -pv "$BUILD_DIR" || exit 1

# Clean build directory
rm -rf "$BUILD_DIR"/*

echo -e "\n=== Configuring and building project ==="
cd "$BUILD_DIR" || exit 1

# Configure CMake based on architecture
CMAKE_OPTS=""
if [ "$ARCH" = "x86" ]; then
    # 32-bit build
    export CFLAGS="-m32"
    export CXXFLAGS="-m32"
    export LDFLAGS="-m32"
    CMAKE_OPTS="-DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32"
fi

cmake .. -DCMAKE_BUILD_TYPE=Debug -DINIPARSER_TESTS=ON $CMAKE_OPTS || {
    echo "CMake configuration failed!" >&2
    exit 1
}

# Determine number of CPU cores available
if command -v nproc >/dev/null 2>&1; then
    # Linux
    NUM_CORES=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    # macOS
    NUM_CORES=$(sysctl -n hw.ncpu)
else
    # Default fallback
    NUM_CORES=4
fi

make -j$NUM_CORES || {
    echo "Build failed!" >&2
    exit 1
}

# Run tests
echo -e "\n=== Running tests ==="
ctest -C Debug --output-on-failure --progress -VV || {
    echo "Some tests failed!" >&2
    exit 1
}

logPath="$BUILD_DIR/test.log"
if [ -f "$logPath" ]; then
    echo -e "\n=== Tests completed. Here is the info from the log file: ==="
    cat "$logPath"
else
    echo -e "\n=== Log file not found at $logPath ==="
fi

echo -e "\n=== All operations completed successfully ==="
cd "$PROJECT_ROOT" || exit 1
