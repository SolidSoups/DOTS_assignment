#!/bin/bash

set -e # Exit on error

BUILD_DIR="./build"

mkdir -p $BUILD_DIR

cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -S . -B $BUILD_DIR

cmake --build $BUILD_DIR

# Copy all font files
mkdir -p $BUILD_DIR/bin/fonts
cp ./DotEngine/fonts/* ./build/bin/fonts 2>/dev/null
echo "Copied all font files"

echo "Build completed successfully"

