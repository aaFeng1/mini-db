#pragma once
#include <cstdint>

namespace mini {

struct IntComparator {
  int operator()(const int32_t &a, const int32_t &b) const {
    if (a < b)
      return -1;
    if (a > b)
      return 1;
    return 0;
  }
};

} // namespace mini
