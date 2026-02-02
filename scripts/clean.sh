#!/usr/bin/env bash
set -e

BUILD_DIR=build
GDB_BUILD_DIR=build-debug

if [ ! -d "$BUILD_DIR" ]; then
  echo "[clean] build/ not found"
else
  echo "[clean] $BUILD_DIR ..."
  rm -rf "$BUILD_DIR"
  echo "[clean] finished"
fi

if [ ! -d "$GDB_BUILD_DIR" ]; then
  echo "[clean] build-debug/ not found"
else
  echo "[clean] $GDB_BUILD_DIR ..."
  rm -rf "$GDB_BUILD_DIR"
  echo "[clean] finished"
fi

