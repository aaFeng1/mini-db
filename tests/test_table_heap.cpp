#include "common/page_guard.h"
#include "storage/table_heap.h"
#include "storage/tuple.h"
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>

using namespace mini;

class TableHeapTest : public ::testing::Test {
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

// 测试插入和读取单条记录
TEST_F(TableHeapTest, InsertAndGetSingleTuple) {
  Tuple tuple;
  char *buf = tuple.Resize(8);
  int32_t id = 42;
  int32_t value = 84;
  std::memcpy(buf, &id, 4);
  std::memcpy(buf + 4, &value, 4);

  RID rid;
  rid = table_heap_->InsertTuple(tuple);
  EXPECT_EQ(rid.page_id, 0);
  EXPECT_EQ(rid.slot_id, 0);

  Tuple out_tuple;
  EXPECT_TRUE(table_heap_->GetTuple(rid, &out_tuple));
  EXPECT_EQ(out_tuple.Size(), 8);
  int32_t out_id;
  int32_t out_value;
  const char *out_data = out_tuple.Data();
  std::memcpy(&out_id, out_data, 4);
  std::memcpy(&out_value, out_data + 4, 4);
  EXPECT_EQ(out_id, id);
  EXPECT_EQ(out_value, value);
}

// 插入1000条记录，验证数据正确性
TEST_F(TableHeapTest, InsertAndGetMultipleTuples) {
  const int num_tuples = 1000;
  std::vector<RID> rids;
  for (int i = 0; i < num_tuples; ++i) {
    Tuple tuple;
    char *buf = tuple.Resize(8);
    int32_t id = i;
    int32_t value = i * 10;
    std::memcpy(buf, &id, 4);
    std::memcpy(buf + 4, &value, 4);

    RID rid = table_heap_->InsertTuple(tuple);
    rids.push_back(rid);
    // std::cout << "Inserted tuple " << i << " at RID(" << rid.page_id << ", "
    //           << rid.slot_id << ")\n";
  }

  for (int i = 0; i < num_tuples; ++i) {
    Tuple out_tuple;
    EXPECT_TRUE(table_heap_->GetTuple(rids[i], &out_tuple));
    EXPECT_EQ(out_tuple.Size(), 8);
    int32_t out_id;
    int32_t out_value;
    const char *out_data = out_tuple.Data();
    std::memcpy(&out_id, out_data, 4);
    std::memcpy(&out_value, out_data + 4, 4);
    EXPECT_EQ(out_id, i);
    EXPECT_EQ(out_value, i * 10);
  }
}

// 测试逻辑删除功能
