#!/bin/bash

show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "Options:"
    echo "  -h, --help   Show this help message"
    echo
    echo "Description:"
    echo "  This script automatically:"
    echo "  1. Creates build directory if needed"
    echo "  2. Configures project with CMake"
    echo "  3. Builds project"
    echo "  4. Runs tests"
    echo
    echo "Example:"
    echo "  $0"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
    -h | --help)
        show_help
        exit 0
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

echo "=== Setting up build environment ==="
mkdir -pv "$BUILD_DIR" || exit 1

echo -e "\n=== Configuring and building project ==="
cd "$BUILD_DIR" || exit 1
cmake .. -DCMAKE_BUILD_TYPE=Release -DINIPARSER_TESTS=ON || {
    echo "CMake configuration failed!" >&2
    exit 1
}

make -j$(nproc) || {
    echo "Build failed!" >&2
    exit 1
}

# Run tests
echo -e "\n=== Running tests ==="
ctest -C Release --output-on-failure -VV || {
    echo "Some tests failed!" >&2
    exit 1
}

echo -e "\n=== All operations completed successfully ==="
cd "$PROJECT_ROOT" || exit 1
