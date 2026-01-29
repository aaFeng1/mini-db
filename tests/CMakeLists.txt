add_executable(db_tests
  test_buffer_pool.cpp
  test_disk_manager.cpp
)

target_link_libraries(db_tests
  PRIVATE
    db_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(db_tests)
