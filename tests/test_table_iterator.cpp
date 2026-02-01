#include "common/page.h"
#include "common/page_guard.h"
#include "storage/table_heap.h"
#include "storage/table_iterator.h"
#include "storage/tuple.h"
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

using namespace mini;

class TableIteratorTest : public ::testing::Test {
protected:
  std::filesystem::path db_file_;

  std::unique_ptr<DiskManager> dm_;
  std::unique_ptr<BufferPool> bp_;
  std::unique_ptr<TableHeap> table_heap_;

  void SetUp() override {
    db_file_ = "test.db";
    std::filesystem::remove(db_file_);

    dm_ = std::make_unique<DiskManager>(db_file_.string());
    bp_ = std::make_unique<BufferPool>(10, dm_.get());
    table_heap_ = std::make_unique<TableHeap>(bp_.get());
  }

  void TearDown() override { std::filesystem::remove(db_file_); }
};

// 基础插入和读取测试
TEST_F(TableIteratorTest, BasicInsertAndRead) {
  // 插入记录
  for (int i = 0; i < 10; ++i) {
    Tuple tuple;
    char *buf = tuple.Resize(12);
    int32_t val1 = i;
    int32_t val2 = i * 10;
    int32_t val3 = i * 100;
    std::memcpy(buf, &val1, 4);
    std::memcpy(buf + 4, &val2, 4);
    std::memcpy(buf + 8, &val3, 4);

    table_heap_->InsertTuple(tuple);
  }

  // 遍历记录
  TableIterator iter = table_heap_->Begin();
  int count = 0;
  while (iter != table_heap_->End()) {
    Tuple tuple = *iter;
    int32_t val1;
    int32_t val2;
    int32_t val3;
    const char *data = tuple.Data();
    std::memcpy(&val1, data, 4);
    std::memcpy(&val2, data + 4, 4);
    std::memcpy(&val3, data + 8, 4);
    EXPECT_EQ(val1, count);
    EXPECT_EQ(val2, count * 10);
    EXPECT_EQ(val3, count * 100);
    ++iter;
    ++count;
  }
  EXPECT_EQ(count, 10);
}

// 插入跨页
TEST_F(TableIteratorTest, InsertAcrossPages) {
  // 插入记录
  int num_records = (PAGE_SIZE / 12) * 2; // 跨页插入
  for (int i = 0; i < num_records; ++i) {
    Tuple tuple;
    char *buf = tuple.Resize(12);
    int32_t val1 = i;
    int32_t val2 = i * 10;
    int32_t val3 = i * 100;
    std::memcpy(buf, &val1, 4);
    std::memcpy(buf + 4, &val2, 4);
    std::memcpy(buf + 8, &val3, 4);

    table_heap_->InsertTuple(tuple);
  }

  // 遍历记录
  TableIterator iter = table_heap_->Begin();
  int count = 0;
  while (iter != table_heap_->End() && count < num_records + 3) {
    Tuple tuple = *iter;
    int32_t val1;
    int32_t val2;
    int32_t val3;
    const char *data = tuple.Data();
    std::memcpy(&val1, data, 4);
    std::memcpy(&val2, data + 4, 4);
    std::memcpy(&val3, data + 8, 4);
    EXPECT_EQ(val1, count);
    EXPECT_EQ(val2, count * 10);
    EXPECT_EQ(val3, count * 100);
    ++iter;
    ++count;
  }
  EXPECT_EQ(count, num_records);
}

// 删除记录遍历不出现
TEST_F(TableIteratorTest, DeleteAndIterate) {
  std::vector<RID> rids;
  // 插入记录
  for (int i = 0; i < 20; ++i) {
    Tuple tuple;
    char *buf = tuple.Resize(8);
    int32_t val1 = i;
    int32_t val2 = i * 10;
    std::memcpy(buf, &val1, 4);
    std::memcpy(buf + 4, &val2, 4);

    RID rid = table_heap_->InsertTuple(tuple);
    rids.push_back(rid);
  }

  // 删除偶数记录
  for (int i = 0; i < 20; i += 2) {
    table_heap_->DeleteTuple(rids[i]);
  }

  // 遍历记录
  TableIterator iter = table_heap_->Begin();
  int count = 1; // 从1开始，因为0被删除
  while (iter != table_heap_->End()) {
    Tuple tuple = *iter;
    int32_t val1;
    int32_t val2;
    const char *data = tuple.Data();
    std::memcpy(&val1, data, 4);
    std::memcpy(&val2, data + 4, 4);
    EXPECT_EQ(val1, count);
    EXPECT_EQ(val2, count * 10);
    ++iter;
    count += 2; // 跳过被删除的偶数
  }
  EXPECT_EQ(count, 21); // 最后一个是19
}

// 空表
TEST_F(TableIteratorTest, EmptyTable) {
  TableIterator iter = table_heap_->Begin();
  EXPECT_EQ(iter, table_heap_->End());
}
