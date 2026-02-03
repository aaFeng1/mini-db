#pragma once
#include "binder/value.h"
#include "catalog/catalog.h"
#include <memory>
#include <string>
#include <vector>

namespace mini {

enum class BoundStatementType { BOUND_INSERT, BOUND_SELECT };

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
  BoundSelectStatement(TableInfo *table) : table_(table) {}
  ~BoundSelectStatement() override = default;
  BoundStatementType Type() const override {
    return BoundStatementType::BOUND_SELECT;
  }
  TableInfo *Table() const { return table_; }

private:
  TableInfo *table_;
};

} // namespace mini
