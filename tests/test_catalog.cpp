#include "catalog/catalog.h"
#include "storage/buffer_pool.h"
#include "storage/disk_manager.h"
#include "storage/table_heap.h"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>

using namespace mini;

class CatalogTest : public ::testing::Test {
protected:
  std::filesystem::path db_file_{"test_catalog.db"};
  std::unique_ptr<DiskManager> dm_;
  std::unique_ptr<BufferPool> bp_;
  std::shared_ptr<TableHeap> table_heap_;

  void SetUp() override {
    dm_ = std::make_unique<DiskManager>(db_file_.string());
    bp_ = std::make_unique<BufferPool>(10, dm_.get());
  }
  void TearDown() override { std::filesystem::remove(db_file_); }
};

// 一张表的创建与获取
TEST_F(CatalogTest, CreateAndGetTable) {
  Catalog catalog(bp_.get());

  auto schema = std::make_shared<Schema>();
  schema->AddColumn("id", DataType::INTEGER);
  schema->AddColumn("name", DataType::VARCHAR, 10);

  TableInfo *table_info = catalog.CreateTable("users", schema);
  EXPECT_NE(table_info, nullptr);
  EXPECT_EQ(table_info->name, "users");
  EXPECT_EQ(table_info->schema->GetColumnCount(), 2);

  TableInfo *fetched_table = catalog.GetTable("users");
  EXPECT_NE(fetched_table, nullptr);
  EXPECT_EQ(fetched_table->name, "users");
  EXPECT_EQ(fetched_table->schema->GetColumnCount(), 2);

  catalog.ListTables();
}

// 多张表的创建与获取
TEST_F(CatalogTest, CreateAndGetMultipleTables) {
  Catalog catalog(bp_.get());

  auto schema1 = std::make_shared<Schema>();
  schema1->AddColumn("id", DataType::INTEGER);
  schema1->AddColumn("name", DataType::VARCHAR, 10);
  schema1->AddColumn("gender", DataType::VARCHAR, 10);
  catalog.CreateTable("users", schema1);

  auto schema2 = std::make_shared<Schema>();
  schema2->AddColumn("product_id", DataType::INTEGER);
  schema2->AddColumn("price", DataType::INTEGER);
  catalog.CreateTable("products", schema2);

  TableInfo *users_table = catalog.GetTable("users");
  EXPECT_NE(users_table, nullptr);
  EXPECT_EQ(users_table->name, "users");
  EXPECT_EQ(users_table->schema->GetColumnCount(), 3);

  TableInfo *products_table = catalog.GetTable("products");
  EXPECT_NE(products_table, nullptr);
  EXPECT_EQ(products_table->name, "products");
  EXPECT_EQ(products_table->schema->GetColumnCount(), 2);

  catalog.ListTables();
}
