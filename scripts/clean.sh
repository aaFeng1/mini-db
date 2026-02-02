#!/usr/bin/env bash
set -e

BUILD_DIR=build
GDB_BUILD_DIR=build-debug

if [ ! -d "$BUILD_DIR" ]; then
  echo "[clean] build/ 不存在，看起来很干净"
  exit 0
else
  echo "[clean] 清理 $BUILD_DIR ..."
  rm -rf "$BUILD_DIR"
  echo "[clean] 完成"
fi

if [ ! -d "$GDB_BUILD_DIR" ]; then
  echo "[clean] build-debug/ 不存在，看起来很干净"
  exit 0
else
  echo "[clean] 清理 $GDB_BUILD_DIR ..."
  rm -rf "$GDB_BUILD_DIR"
  echo "[clean] 完成"
fi

