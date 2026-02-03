#include "execution/executor.h"
#include "binder/value.h"
#include "catalog/catalog.h"
#include "parser/statement.h"
#include "storage/tuple.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>

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
    case ValueType::INT: {
      const IntValue *intv = dynamic_cast<const IntValue *>(value);
      // TODO: add nullptr check
      if (!intv)
        throw std::runtime_error("bad int value");
      int32_t tmp = intv->GetValue();
      memcpy(buf + col.offset, &tmp, std::min<size_t>(col.length, sizeof(tmp)));
      break;
    }

    case ValueType::STRING: {
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

} // namespace mini
