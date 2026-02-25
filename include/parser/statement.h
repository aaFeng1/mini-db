#pragma once
#include "binder/value.h"
#include "parser/literal.h"
#include "type/data_type.h"
#include <memory>
#include <string>
#include <vector>

namespace mini {

enum class StatementType { INSERT, SELECT, CREATE_TABLE, CREATE_INDEX };

class Statement {
public:
  Statement() = default;
  virtual ~Statement() = default;
  virtual StatementType Type() const = 0;

private:
};

class InsertStatement : public Statement {
public:
  InsertStatement(std::string table_name,
                  std::vector<std::unique_ptr<Literal>> values)
      : table_name_(std::move(table_name)), values_(std::move(values)) {}
  ~InsertStatement() override = default;

  StatementType Type() const override { return StatementType::INSERT; }
  std::string Table_name() const { return table_name_; }
  const std::vector<std::unique_ptr<Literal>> &Values() const {
    return values_;
  } // TODO: ???

private:
  std::string table_name_;
  // TODO: not supported in v1
  // std::vector<std::string> columns_;
  std::vector<std::unique_ptr<Literal>> values_;
};

class SelectStatement : public Statement {
public:
  // SelectStatement(std::string table_name, std::vector<std::string> columns)
  //     : table_name_(std::move(table_name)), columns_(std::move(columns)) {}
  SelectStatement(std::string table_name, bool is_select_all = true,
                  bool has_where = false, std::string where_column = "",
                  std::unique_ptr<Value> where_value = nullptr)
      : table_name_(std::move(table_name)), is_select_all_(is_select_all),
        has_where_(has_where), where_column_(where_column),
        where_value_(std::move(where_value)) {}
  ~SelectStatement() override = default;

  StatementType Type() const override { return StatementType::SELECT; }
  std::string Table_name() const { return table_name_; }
  bool Select_all() const { return is_select_all_; }
  bool Has_where() const { return has_where_; }
  std::string Where_column() const { return where_column_; }
  const Value *Where_value() const { return where_value_.get(); }

private:
  std::string table_name_;
  // TODO: support * in v1
  // std::vector<std::string> columns_;
  bool is_select_all_;

  bool has_where_{false};
  std::string where_column_;
  std::unique_ptr<Value> where_value_;
};

class CreateTableStatement : public Statement {
public:
  CreateTableStatement(std::string table_name,
                       std::vector<std::pair<std::string, ColumnType>> columns)
      : table_name_(std::move(table_name)), columns_(std::move(columns)) {}
  ~CreateTableStatement() override = default;

  StatementType Type() const override { return StatementType::CREATE_TABLE; }
  std::string Table_name() const { return table_name_; }
  const std::vector<std::pair<std::string, ColumnType>> &Columns() const {
    return columns_;
  }

private:
  std::string table_name_;
  std::vector<std::pair<std::string, ColumnType>> columns_;
};

class CreateIndexStatement : public Statement {
public:
  CreateIndexStatement(std::string index_name, std::string table_name,
                       std::vector<std::string> column_names)
      : index_name_(std::move(index_name)), table_name_(std::move(table_name)),
        column_names_(std::move(column_names)) {}
  ~CreateIndexStatement() override = default;

  StatementType Type() const override { return StatementType::CREATE_INDEX; }
  std::string Index_name() const { return index_name_; }
  std::string Table_name() const { return table_name_; }
  const std::vector<std::string> &Column_names() const { return column_names_; }

private:
  std::string index_name_;
  std::string table_name_;
  std::vector<std::string> column_names_;
};

} // namespace mini
