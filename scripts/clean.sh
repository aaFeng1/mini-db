#!/usr/bin/env bash
set -e

BUILD_DIR=build

if [ ! -d "$BUILD_DIR" ]; then
  echo "[clean] build/ 不存在，看起来很干净"
  exit 0
fi

echo "[clean] 清理 $BUILD_DIR ..."
rm -rf "$BUILD_DIR"
echo "[clean] 完成"
