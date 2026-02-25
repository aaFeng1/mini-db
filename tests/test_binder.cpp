#include "binder/binder.h"
#include "binder/bound_statement.h"
#include "binder/value.h"
#include "catalog/catalog.h"
#include "parser/parser.h"
#include "storage/buffer_pool.h"
#include "storage/disk_manager.h"
#include "type/data_type.h"
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <variant>

using namespace mini;

class BinderTest : public ::testing::Test {
protected:
  Catalog *catalog_;
  Binder *binder_;
  DiskManager *disk_manager_;
  BufferPool *bpm_;
  void SetUp() override {
    std::filesystem::remove("data/test.db");
    disk_manager_ = new DiskManager("data/test.db");
    bpm_ = new BufferPool(10, disk_manager_); // 假设 BufferPool 有一个容量参数
    catalog_ = new Catalog(bpm_);
    binder_ = new Binder(*catalog_);
  }
  void TearDown() override {
    delete binder_;
    delete catalog_;
    delete bpm_;
    delete disk_manager_;
  }
};

// INSERT INTO t VALUES (1, 'a');
TEST_F(BinderTest, BindInsertStatement) {
  // 首先在 catalog 中创建表 t
  auto schema = std::make_shared<Schema>();
  schema->AddColumn("col1", DataType::INTEGER);
  schema->AddColumn("col2", DataType::VARCHAR, 10);
  catalog_->CreateTable("t", schema);

  // 创建 InsertStatement
  std::vector<std::unique_ptr<Literal>> values;
  values.push_back(std::make_unique<IntLiteral>("1"));
  values.push_back(std::make_unique<StringLiteral>("a"));
  InsertStatement insert_stmt("t", std::move(values));

  // 绑定语句
  auto bound_stmt = binder_->BindStatement(insert_stmt);
  ASSERT_NE(bound_stmt, nullptr);
  ASSERT_EQ(bound_stmt->Type(), BoundStatementType::BOUND_INSERT);

  auto bound_insert = static_cast<BoundInsertStatement *>(bound_stmt.get());
  ASSERT_EQ(bound_insert->Table()->name, "t");
  const auto &bound_values = bound_insert->Values();
  ASSERT_EQ(bound_values.size(), 2);
  ASSERT_EQ(dynamic_cast<const IntValue *>(bound_values[0].get())->GetValue(),
            1);
  ASSERT_EQ(
      dynamic_cast<const StringValue *>(bound_values[1].get())->GetValue(),
      "a");
}

// SELECT * FROM t;
TEST_F(BinderTest, BindSelectStatement) {
  // 首先在 catalog 中创建表 t
  auto schema = std::make_shared<Schema>();
  schema->AddColumn("col1", DataType::INTEGER);
  schema->AddColumn("col2", DataType::VARCHAR, 10);
  catalog_->CreateTable("t", schema);

  // 创建 SelectStatement
  SelectStatement select_stmt("t");

  // 绑定语句
  auto bound_stmt = binder_->BindStatement(select_stmt);
  ASSERT_NE(bound_stmt, nullptr);
  ASSERT_EQ(bound_stmt->Type(), BoundStatementType::BOUND_SELECT);

  auto bound_select = static_cast<BoundSelectStatement *>(bound_stmt.get());
  ASSERT_EQ(bound_select->Table()->name, "t");
}

// CREATE TABLE t (col1 INTEGER, col2 VARCHAR(10));
TEST_F(BinderTest, BindCreateTableStatement) {
  // 创建 CreateTableStatement
  std::vector<std::pair<std::string, ColumnType>> columns = {
      {"col1", ColumnType{DataType::INTEGER, 0}},
      {"col2", ColumnType{DataType::VARCHAR, 10}}};
  CreateTableStatement create_table_stmt("t", columns);

  // 绑定语句
  auto bound_stmt = binder_->BindStatement(create_table_stmt);
  ASSERT_NE(bound_stmt, nullptr);
  ASSERT_EQ(bound_stmt->Type(), BoundStatementType::BOUND_CREATE_TABLE);

  auto bound_create_table =
      static_cast<BoundCreateTableStatement *>(bound_stmt.get());
  ASSERT_EQ(bound_create_table->TableName(), "t");
  const auto &bound_columns = bound_create_table->Columns();
  ASSERT_EQ(bound_columns.size(), 2);
  ASSERT_EQ(bound_columns[0].first, "col1");
  ASSERT_EQ(bound_columns[0].second.type, DataType::INTEGER);
  ASSERT_EQ(bound_columns[1].first, "col2");
  ASSERT_EQ(bound_columns[1].second.type, DataType::VARCHAR);
  ASSERT_EQ(bound_columns[1].second.length, 10);
}

// CREATE INDEX idx ON t (col1);
TEST_F(BinderTest, BindCreateIndexStatement) {
  // 首先在 catalog 中创建表 t
  auto schema = std::make_shared<Schema>();
  schema->AddColumn("col1", DataType::INTEGER);
  schema->AddColumn("col2", DataType::VARCHAR, 10);
  catalog_->CreateTable("t", schema);

  // 创建 CreateIndexStatement
  std::vector<std::string> column_names = {"col1"};
  CreateIndexStatement create_index_stmt("idx", "t", column_names);

  // 绑定语句
  auto bound_stmt = binder_->BindStatement(create_index_stmt);
  ASSERT_NE(bound_stmt, nullptr);
  ASSERT_EQ(bound_stmt->Type(), BoundStatementType::BOUND_CREATE_INDEX);

  auto bound_create_index =
      static_cast<BoundCreateIndexStatement *>(bound_stmt.get());
  ASSERT_EQ(bound_create_index->IndexName(), "idx");
  ASSERT_EQ(bound_create_index->TableName(), "t");
  const auto &bound_column_ids = bound_create_index->ColumnIds();
  ASSERT_EQ(bound_column_ids.size(), 1);
  ASSERT_EQ(bound_column_ids[0], 0); // col1 在 schema 中的索引是 0
}
