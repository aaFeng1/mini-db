#pragma once

#include "type/data_type.h"
#include <cstdint>
#include <string>
namespace mini {

struct Column {
  std::string name;
  DataType type;
  uint32_t offset;
  uint32_t length;
};

} // namespace mini
