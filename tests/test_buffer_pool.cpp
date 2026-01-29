#include "common/page.h"
#include "storage/buffer_pool.h"
#include "storage/disk_manager.h"
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

using namespace mini;

class BufferPoolTest : public ::testing::Test {
protected:
  std::filesystem::path db_file_;

  std::unique_ptr<DiskManager> dm_;
  std::unique_ptr<BufferPool> bp_;

  void SetUp() override {
    db_file_ = "test.db";
    std::filesystem::remove(db_file_);

    dm_ = std::make_unique<DiskManager>(db_file_.string());
  }

  void TearDown() override { std::filesystem::remove(db_file_); }
};

// 当pool大小为2时，fetch然后标记脏，然后再加载另一页，释放，然后再加载原来的页应该能看见数据修改了
TEST_F(BufferPoolTest, DirtyPageIsFlushed) {
  bp_.reset();
  bp_ = std::make_unique<BufferPool>(1, dm_.get());

  page_id_t pid = 0;
  Page *page = bp_->FetchPage(pid);
  ASSERT_NE(page, nullptr);

  const char *msg = "hello buffer pool";
  std::memcpy(page->GetData(), msg, std::strlen(msg) + 1);
  EXPECT_TRUE(bp_->UnpinPage(pid, true));

  // 加载另一页
  pid = 1;
  Page *page2 = bp_->FetchPage(pid);
  ASSERT_NE(page2, nullptr);
  msg = "page2";
  std::memcpy(page2->GetData(), msg, std::strlen(msg) + 1);
  EXPECT_TRUE(bp_->UnpinPage(pid, true));

  pid = 0;
  page2 = bp_->FetchPage(pid);
  ASSERT_NE(page, nullptr);
  EXPECT_STREQ("hello buffer pool", page->GetData());
}

// fetch两次同一页，应该地址一样
TEST_F(BufferPoolTest, FetchPagePinsPage) {
  bp_.reset();
  bp_ = std::make_unique<BufferPool>(1, dm_.get());

  page_id_t pid = 0;
  Page *page = bp_->FetchPage(pid);
  ASSERT_NE(page, nullptr);

  Page *page2 = bp_->FetchPage(pid);
  ASSERT_NE(page2, nullptr);

  EXPECT_EQ(page, page2);
}

// 当bufferpool占满的时候，再加载页应该失败
TEST_F(BufferPoolTest, PoolIsFull) {
  bp_.reset();
  bp_ = std::make_unique<BufferPool>(1, dm_.get());

  page_id_t pid = 0;
  Page *page = bp_->FetchPage(pid);
  ASSERT_NE(page, nullptr);

  pid = 1;
  Page *page2 = bp_->FetchPage(pid);
  EXPECT_EQ(page2, nullptr);
}

// flushallpage后再全部重新读内容应该不变
TEST_F(BufferPoolTest, FlushAllPageReread) {
  bp_.reset();
  bp_ = std::make_unique<BufferPool>(2, dm_.get());

  page_id_t pid = 0;
  Page *page = bp_->FetchPage(pid);
  ASSERT_NE(page, nullptr);
  const char *msg = "hello buffer pool page0";
  std::memcpy(page->GetData(), msg, std::strlen(msg) + 1);
  EXPECT_TRUE(bp_->UnpinPage(pid, true));

  pid = 1;
  Page *page1 = bp_->FetchPage(pid);
  ASSERT_NE(page1, nullptr);
  msg = "hello buffer pool page1";
  std::memcpy(page1->GetData(), msg, std::strlen(msg) + 1);
  EXPECT_TRUE(bp_->UnpinPage(pid, true));

  bp_->FlushAllPages();

  pid = 0;
  page = bp_->FetchPage(pid);
  EXPECT_STREQ(page->GetData(), "hello buffer pool page0");
  pid = 1;
  page1 = bp_->FetchPage(pid);
  EXPECT_STREQ(page1->GetData(), "hello buffer pool page1");
  EXPECT_TRUE(bp_->UnpinPage(1, false));
  EXPECT_TRUE(bp_->UnpinPage(0, false));
}

// upin不存在的页应该失败
TEST_F(BufferPoolTest, UnpinNonExistingPageFails) {
  bp_.reset();
  bp_ = std::make_unique<BufferPool>(1, dm_.get());

  EXPECT_FALSE(bp_->UnpinPage(0, false));
}
