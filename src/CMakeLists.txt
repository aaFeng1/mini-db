file(GLOB_RECURSE SRC_FILES
    CONFIGURE_DEPENDS
    *.cpp
)

list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_library(db_core STATIC ${SRC_FILES})

target_include_directories(db_core
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(db_core PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)


add_executable(minidb main.cpp)
target_link_libraries(minidb PRIVATE db_core)
