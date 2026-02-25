#include "execution/executor.h"
#include "binder/value.h"
#include "catalog/catalog.h"
#include "catalog/column.h"
#include "parser/statement.h"
#include "storage/tuple.h"
#include "type/data_type.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>

namespace mini {

void InsertExecutor::Init() { done_ = false; }

bool InsertExecutor::Next(Tuple *) {
  if (done_)
    return false;
  // TODO: 支持错误处理,以及非全量数据
  auto schema = bound_insert_stmt_->Table()->schema;
  const auto &columns = schema->GetColumns();

  uint32_t len = schema->GetTupleLength();
  Tuple tuple;
  char *buf = tuple.Resize(len);
  for (size_t i = 0; i < bound_insert_stmt_->ValueCount(); ++i) {
    auto col = columns[i];
    const Value *value = bound_insert_stmt_->ValueAt(i);
    switch (value->Type()) {
    case DataType::INTEGER: {
      const IntValue *intv = dynamic_cast<const IntValue *>(value);
      // TODO: add nullptr check
      if (!intv)
        throw std::runtime_error("bad int value");
      int32_t tmp = intv->GetValue();
      memcpy(buf + col.offset, &tmp, std::min<size_t>(col.length, sizeof(tmp)));
      break;
    }

    case DataType::VARCHAR: {
      const StringValue *strv = dynamic_cast<const StringValue *>(value);
      const auto &str = strv->GetValue();
      memcpy(buf + col.offset, str.data(),
             std::min<size_t>(col.length, str.size()));
      break;
    }

    default: {
      throw std::runtime_error("unsupport value");
    }
    }
  }
  bound_insert_stmt_->Table()->table->InsertTuple(tuple);
  return done_ = true;
}

void SelectExecutor::Init() {
  TableInfo *table = bound_select_stmt_->Table();
  table_iter_ = table->table->Begin();
  end_ = table->table->End();

  inited_ = true;
}

bool SelectExecutor::Next(Tuple *ret) {
  if (!inited_)
    return false;
  if (table_iter_ == end_)
    return false;

  *ret = *table_iter_;
  ++table_iter_;
  return true;
}

void CreateTableExecutor::Init() {
  auto table_name = bound_create_table_stmt_->TableName();
  auto columns = bound_create_table_stmt_->Columns();
  auto &catalog = Context().GetCatalog();

  std::vector<Column> cols;
  uint32_t offset = 0;
  for (const auto &col : columns) {
    if (col.second.type == DataType::INTEGER) {
      cols.push_back({col.first, DataType::INTEGER, offset, sizeof(int32_t)});
      offset += sizeof(int32_t);
    } else if (col.second.type == DataType::VARCHAR) {
      cols.push_back({col.first, DataType::VARCHAR, offset,
                      col.second.length}); // 假设字符串最大长度为255
      offset += col.second.length;
    } else {
      throw std::runtime_error("unsupported column type");
    }
  }

  auto schema = std::make_shared<Schema>(cols);
  catalog.CreateTable(table_name, schema);

  done_ = true;
}

bool CreateTableExecutor::Next(Tuple *) { return !done_; }

void CreateIndexExecutor::Init() {
  Context().GetCatalog().CreateIndex(bound_create_index_stmt_->IndexName(),
                                     bound_create_index_stmt_->TableName(),
                                     bound_create_index_stmt_->ColumnIds()[0]);
  done_ = true;
}

bool CreateIndexExecutor::Next(Tuple *) { return !done_; }

} // namespace mini
