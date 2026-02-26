#include "common/comparator.h"
#include "common/page_guard.h"
#include "index/bplus_tree.h"
#include "index/bplus_tree_page.h"
#include "storage/disk_manager.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <random>

using namespace mini;

class BPlusTreeTest : public ::testing::Test {
protected:
  DiskManager *disk_manager;
  BufferPool *buffer_pool;

  void SetUp() override {
    std::string db_file_ = "data/test.db";
    std::filesystem::remove(db_file_);
    disk_manager = new DiskManager(db_file_);
    buffer_pool = new BufferPool(100, disk_manager);
  }

  void TearDown() override {
    delete buffer_pool;
    delete disk_manager;
  }
};

// 基础的插入，不涉及分裂
TEST_F(BPlusTreeTest, BasicInsert) {
  std::ofstream out("data/BasicInsert.txt");
  BPlusTree<int32_t, RID, mini::IntComparator> tree(buffer_pool);
  for (int i = 0; i < 200; ++i) {
    EXPECT_TRUE(tree.Insert(i, RID{i, static_cast<uint16_t>(i)}));
  }
  for (int i = 0; i < 200; ++i) {
    std::vector<RID> values;
    EXPECT_TRUE(tree.GetValue(i, &values));
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0].page_id, i);
    EXPECT_EQ(values[0].slot_id, static_cast<uint16_t>(i));
  }
  tree.Print(out);
}

// 有一页根，两页数据，数据量500以上
// --gtest_filter=BPlusTreeTest.InsertWithSplit
TEST_F(BPlusTreeTest, InsertWithSplit) {
  std::ofstream out("data/InsertWithSplit.txt");
  BPlusTree<int32_t, RID, mini::IntComparator> tree(buffer_pool);
  for (int i = 0; i < 600; ++i) {
    EXPECT_TRUE(tree.Insert(i, RID{i, static_cast<uint16_t>(i)}));
  }
  tree.Print(out);
  for (int i = 0; i < 600; ++i) {
    // std::cout << i << std::endl;
    std::vector<RID> values;
    EXPECT_TRUE(tree.GetValue(i, &values));
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0].page_id, i);
    EXPECT_EQ(values[0].slot_id, static_cast<uint16_t>(i));
  }
}

// 随机数据
TEST_F(BPlusTreeTest, RandomInsert) {
  std::ofstream out("data/RandomInsert.txt");
  BPlusTree<int32_t, RID, mini::IntComparator> tree(buffer_pool);
  std::vector<int> keys(1000);
  for (int i = 0; i < 1000; ++i) {
    keys[i] = i;
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(keys.begin(), keys.end(), gen);
  for (int key : keys) {
    EXPECT_TRUE(tree.Insert(key, RID{key, static_cast<uint16_t>(key)}));
  }
  tree.Print(out);
  for (int key : keys) {
    std::vector<RID> values;
    EXPECT_TRUE(tree.GetValue(key, &values));
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values[0].page_id, key);
    EXPECT_EQ(values[0].slot_id, static_cast<uint16_t>(key));
  }
}
