#include "common/metapage.h"
#include "common/page.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace mini;

class MetaPage : public ::testing::Test {
  void SetUp() override {}
  void TearDown() override {}
};

// --gtest_filter=MetaPage.Static
TEST_F(MetaPage, Static) {
  std::cout << sizeof(DBMetaPage) << std::endl;
  EXPECT_TRUE(sizeof(DBMetaPage) <= PAGE_SIZE);
}
