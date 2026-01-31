#pragma once
#include "catalog/column.h"
#include <stdexcept>
#include <vector>

namespace mini {

class Schema {
public:
  Schema() = default;
  explicit Schema(std::vector<Column> columns) : columns_(std::move(columns)) {
    tuple_length_ = 0;
    for (auto col : columns_) {
      tuple_length_ += col.length;
    }
  }

  const std::vector<Column> &GetColumns() const { return columns_; }

  uint32_t GetTupleLength() const { return tuple_length_; }

  void AddColumn(const std::string &name, DataType type, uint32_t length = 0) {
    if (type == DataType::INTEGER) {
      length = 4;
    } else if (type == DataType::VARCHAR && length == 0) {
      throw std::invalid_argument("VARCHAR column must have a positive length");
    }
    columns_.push_back({name, type, tuple_length_, length});
    tuple_length_ += length;
  }

  uint32_t GetColumnCount() const { return columns_.size(); }

private:
  std::vector<Column> columns_;
  uint32_t tuple_length_{0};
};

} // namespace mini
