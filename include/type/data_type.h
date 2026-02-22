#pragma once

#include <cstdint>
namespace mini {
enum class DataType { INTEGER, VARCHAR };

struct ColumnType {
  DataType type{};
  uint32_t length{0}; // only used for VARCHAR

  static ColumnType Integer() { return ColumnType{DataType::INTEGER, 0}; }
  static ColumnType Varchar(uint32_t length) {
    return ColumnType{DataType::VARCHAR, length};
  }
};

} // namespace mini
