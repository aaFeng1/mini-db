#include "binder/binder.h"
#include "binder/bound_statement.h"
#include "binder/value.h"
#include "catalog/catalog.h"
#include "parser/literal.h"
#include "type/data_type.h"
#include <memory>

namespace mini {

std::unique_ptr<BoundStatement>
Binder::BindStatement(const Statement &statement) {
  switch (statement.Type()) {
  case StatementType::INSERT:
    return BindInsert(static_cast<const InsertStatement &>(statement));
  case StatementType::SELECT:
    return BindSelect(static_cast<const SelectStatement &>(statement));
  case StatementType::CREATE_TABLE:
    return BindCreateTable(
        static_cast<const CreateTableStatement &>(statement));
  default:
    return nullptr;
  }
}

std::unique_ptr<BoundStatement>
Binder::BindInsert(const InsertStatement &statement) {
  std::string table_name = statement.Table_name();
  TableInfo *table = catalog_.GetTable(table_name);
  if (table == nullptr) {
    error_ =
        BindError("Table not found: " + table_name, SourceSpan{0, 0, 0, 0});
    return nullptr;
  }
  std::vector<std::unique_ptr<Value>> values;
  for (const auto &literal_ptr : statement.Values()) {
    const Literal *literal = literal_ptr.get();
    if (const StringLiteral *str_lit =
            dynamic_cast<const StringLiteral *>(literal)) {
      values.emplace_back(std::make_unique<StringValue>(str_lit->value()));
    } else if (const IntLiteral *int_lit =
                   dynamic_cast<const IntLiteral *>(literal)) {
      values.emplace_back(std::make_unique<IntValue>(int_lit->value()));
    } else {
      error_ = BindError("Unsupported literal type", SourceSpan{0, 0, 0, 0});
      return nullptr;
    }
  }
  return std::make_unique<BoundInsertStatement>(table, std::move(values));
}

std::unique_ptr<BoundStatement>
Binder::BindSelect(const SelectStatement &statement) {
  std::string table_name = statement.Table_name();
  TableInfo *table = catalog_.GetTable(table_name);
  if (table == nullptr) {
    error_ =
        BindError("Table not found: " + table_name, SourceSpan{0, 0, 0, 0});
    return nullptr;
  }
  return std::make_unique<BoundSelectStatement>(table);
}

std::unique_ptr<BoundStatement>
Binder::BindCreateTable(const CreateTableStatement &statement) {
  std::string table_name = statement.Table_name();
  std::vector<std::pair<std::string, ColumnType>> columns = statement.Columns();
  return std::make_unique<BoundCreateTableStatement>(table_name, columns);
}

} // namespace mini
