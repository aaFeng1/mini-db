#pragma once
#include <cstdint>

namespace mini {

struct RID {
  int32_t page_id;
  uint16_t slot_id;
};

} // namespace mini
