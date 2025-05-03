#!/bin/bash

# Help message
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "Options:"
    echo "  -h, --help   Show this help message"
    echo
    echo "Example:"
    echo "  $0"
}

# Parse command line arguments
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

# Validate paths and setup
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory does not exist. Run the build script first."
    exit 1
fi

cd "$BUILD_DIR" || exit 1

# Run tests
echo "Running tests..."
ctest -C Release --output-on-failure -VV
echo "Tests completed."
