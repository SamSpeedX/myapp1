#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")"

BUILD_DIR="build"
EXECUTABLE="$BUILD_DIR/myspec"

mkdir -p "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --config Release

echo "Built $EXECUTABLE"

echo "To run the app, use: $EXECUTABLE"

echo "Or run with ./deploy.sh --run to launch after build."

if [[ "${1:-}" == "--run" ]]; then
  echo "Running $EXECUTABLE"
  "$EXECUTABLE"
fi
