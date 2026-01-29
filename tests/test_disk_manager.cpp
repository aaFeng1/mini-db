#include "storage/disk_manager.h"
#include <cstring>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

using namespace mini;

class DiskManagerTest : public ::testing::Test {
protected:
  std::filesystem::path db_file_;

  std::unique_ptr<DiskManager> dm_;

  void SetUp() override {
    db_file_ = "test.db";
    std::filesystem::remove(db_file_);

    dm_ = std::make_unique<DiskManager>(db_file_.string());
  }

  void TearDown() override { std::filesystem::remove(db_file_); }
};

TEST_F(DiskManagerTest, Simple) { EXPECT_EQ(1 + 1, 2); }
// TEST_F(DiskManagerTest, SimpleFail) { EXPECT_EQ(1 + 1, 3); }

TEST_F(DiskManagerTest, SimpleReadWrite) {
  Page out;
  const char *msg = "page-0";
  std::memcpy(out.data.data(), msg, std::strlen(msg));
  dm_->WritePage(0, out);
  Page in;
  dm_->ReadPage(0, in);
  EXPECT_STREQ(in.GetData(), msg);
}

TEST_F(DiskManagerTest, InvalidPageThrows) {
  Page buf;
  EXPECT_THROW(dm_->WritePage(-1, buf), std::runtime_error);
  EXPECT_THROW(dm_->ReadPage(-1, buf), std::runtime_error);
}

TEST_F(DiskManagerTest, DISABLED_ReWrite) {
  Page out;
  const char *msg = "page-0...";
  std::memcpy(out.data.data(), msg, std::strlen(msg));
  dm_->WritePage(0, out);
  // write again
  const char *msg1 = "page-0";
  std::memcpy(out.data.data(), msg1, std::strlen(msg1));
  dm_->WritePage(0, out);
  Page in;
  dm_->ReadPage(0, in);
  EXPECT_STREQ(in.GetData(), msg1);
  EXPECT_STRNE(in.GetData(), msg);
}
