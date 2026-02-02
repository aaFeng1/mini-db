#!/usr/bin/env bash
set -e

GDB_BUILD_DIR=build-debug

mkdir -p "$GDB_BUILD_DIR"

cmake -S . -B "$GDB_BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$GDB_BUILD_DIR" -j2


