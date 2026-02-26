#pragma once
#include "binder/value.h"
#include "catalog/catalog.h"
#include "catalog/schema.h"
#include "type/data_type.h"
#include <memory>
#include <string>
#include <vector>

namespace mini {

enum class BoundStatementType {
  BOUND_INSERT,
  BOUND_SELECT,
  BOUND_CREATE_TABLE,
  BOUND_CREATE_INDEX,
};

class BoundStatement {
public:
  BoundStatement() = default;
  virtual ~BoundStatement() = default;
  virtual BoundStatementType Type() const = 0;

private:
};

class BoundInsertStatement : public BoundStatement {
public:
  BoundInsertStatement(TableInfo *table,
                       std::vector<std::unique_ptr<Value>> values)
      : table_(table), values_(std::move(values)) {}
  ~BoundInsertStatement() override = default;

  BoundStatementType Type() const override {
    return BoundStatementType::BOUND_INSERT;
  }
  TableInfo *Table() const { return table_; }
  const std::vector<std::unique_ptr<Value>> &Values() const { return values_; }

  const Value *ValueAt(size_t i) const { return values_[i].get(); }
  size_t ValueCount() const { return values_.size(); }

private:
  TableInfo *table_;
  std::vector<std::unique_ptr<Value>> values_;
};

class BoundSelectStatement : public BoundStatement {
public:
  BoundSelectStatement(TableInfo *table, IndexInfo *index_info = nullptr)
      : table_(table), index_info_(index_info) {}
  ~BoundSelectStatement() override = default;
  BoundStatementType Type() const override {
    return BoundStatementType::BOUND_SELECT;
  }
  TableInfo *Table() const { return table_; }
  std::shared_ptr<Schema> GetSchema() const { return table_->schema; }
  IndexInfo *Index() const { return index_info_; }

private:
  TableInfo *table_;
  IndexInfo *index_info_;
};

class BoundCreateTableStatement : public BoundStatement {
public:
  BoundCreateTableStatement(
      std::string table_name,
      std::vector<std::pair<std::string, ColumnType>> columns)
      : table_name_(std::move(table_name)), columns_(std::move(columns)) {}
  ~BoundCreateTableStatement() override = default;
  BoundStatementType Type() const override {
    return BoundStatementType::BOUND_CREATE_TABLE;
  }
  const std::string &TableName() const { return table_name_; }
  const std::vector<std::pair<std::string, ColumnType>> &Columns() const {
    return columns_;
  }

private:
  std::string table_name_;
  std::vector<std::pair<std::string, ColumnType>> columns_;
};

class BoundCreateIndexStatement : public BoundStatement {
public:
  BoundCreateIndexStatement(std::string index_name, std::string table_name,
                            std::vector<uint32_t> column_names)
      : index_name_(std::move(index_name)), table_name_(std::move(table_name)),
        column_names_(std::move(column_names)) {}
  ~BoundCreateIndexStatement() override = default;
  BoundStatementType Type() const override {
    return BoundStatementType::BOUND_CREATE_INDEX;
  }
  const std::string &IndexName() const { return index_name_; }
  const std::string &TableName() const { return table_name_; }
  const std::vector<uint32_t> &ColumnIds() const { return column_names_; }

private:
  std::string index_name_;
  std::string table_name_;
  // 目前只支持单列索引，所以 column_names_ 里只有一个元素，且是列的 id（在
  // schema 中的索引）
  std::vector<uint32_t> column_names_;
};

} // namespace mini
