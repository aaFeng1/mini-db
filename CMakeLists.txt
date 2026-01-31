cmake_minimum_required(VERSION 3.20)
project(lightdb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 生成 compile_commands.json（clangd 用）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- Core library ----------
add_library(db_core
  src/common/page_guard.cpp
  src/storage/buffer_pool.cpp
  src/storage/disk_manager.cpp
  src/storage/table_heap.cpp
  src/catalog/catalog.cpp
)

target_include_directories(db_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 你可以逐步加一些更严格的编译选项（先别一口气拉满，避免你被 warning 淹死）
target_compile_options(db_core PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# ---------- Main executable ----------
add_executable(minidb src/main.cpp)
target_link_libraries(minidb PRIVATE db_core)

# ---------- Tests ----------
option(ENABLE_TESTS "Build tests" ON)

if(ENABLE_TESTS)
  add_subdirectory(third_party/googletest)
  enable_testing()
  add_subdirectory(tests)
endif()

