cmake_minimum_required(VERSION 3.20)
project(lightdb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# clangd 用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- Core ----------
add_subdirectory(src)

# ---------- Tests ----------
option(ENABLE_TESTS "Build tests" ON)

if(ENABLE_TESTS)
  add_subdirectory(third_party/googletest)
  enable_testing()
  add_subdirectory(tests)
endif()
