#pragma once
#include <array>
#include <cstdint>

namespace mini {

using page_id_t = int32_t;
constexpr std::size_t PAGE_SIZE = 4096;

struct Page {
  std::array<std::uint8_t, PAGE_SIZE> data{};

  char *GetData() { return reinterpret_cast<char *>(data.data()); }
};

} // namespace mini
