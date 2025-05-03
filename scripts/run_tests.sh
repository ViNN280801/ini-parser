#!/bin/bash

show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "Options:"
    echo "  -h, --help   Show this help message"
    echo
    echo "Description:"
    echo "  This script builds and runs tests."
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
        echo "Unknown option: $1"
        exit 1
        ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory does not exist. Run cmake first." >&2
    exit 1
fi

echo "Building in Release configuration..."
mkdir -pv "$BUILD_DIR" && cd "$BUILD_DIR" || exit 1
cmake .. -DCMAKE_BUILD_TYPE=Release -DINIPARSER_TESTS=ON
make -j2
if [ $? -ne 0 ]; then
    echo "Build failed" >&2
    exit 1
fi

cd "$BUILD_DIR" || exit 1
ctest -C Release --output-on-failure -VV
echo "Tests completed."
cd "$PROJECT_ROOT" || exit 1
