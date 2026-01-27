cmake_minimum_required(VERSION 3.10)
project(mini_db)

set(CMAKE_CXX_STANDARD 17)

add_executable(mini_db 
    src/main.cpp
    src/storage/disk_manager.cpp
)

add_executable(test_disk tests/test_disk_manager.cpp src/storage/disk_manager.cpp)
add_executable(test_bufferpool tests/test_buffer_pool.cpp src/storage/buffer_pool.cpp src/storage/disk_manager.cpp)

enable_testing()

add_test(NAME test_disk COMMAND test_disk)
add_test(NAME test_bufferpool COMMAND test_bufferpool)