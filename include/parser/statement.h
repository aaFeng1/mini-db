#pragma once
#include "parser/literal.h"
#include <memory>
#include <string>
#include <vector>

namespace mini {

enum class StatementType { INSERT, SELECT };

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
  SelectStatement(std::string table_name)
      : table_name_(std::move(table_name)), is_select_all_(true) {}
  ~SelectStatement() override = default;

  StatementType Type() const override { return StatementType::SELECT; }
  std::string Table_name() const { return table_name_; }
  bool Select_all() const { return is_select_all_; }

private:
  std::string table_name_;
  // TODO: support * in v1
  // std::vector<std::string> columns_;
  bool is_select_all_;
};

} // namespace mini
