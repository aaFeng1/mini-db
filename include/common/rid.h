#pragma once
#include <cstdint>

namespace mini {

struct RID {
  RID() : page_id(-1), slot_id(0) {}
  RID(int32_t page_id, uint16_t slot_id) : page_id(page_id), slot_id(slot_id) {}
  int32_t page_id;
  uint16_t slot_id;
};

} // namespace mini
