file(GLOB_RECURSE TEST_FILES
    CONFIGURE_DEPENDS
    *.cpp
)

add_executable(db_tests ${TEST_FILES})

target_link_libraries(db_tests
    db_core
    gtest
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(db_tests)
