#pragma once
#include <array>
#include <cstdint>

namespace mini {

using page_id_t = int32_t;
constexpr std::size_t PAGE_SIZE = 4096;

class Page {
public:
  template <typename T> T *As() {
    static_assert(std::is_standard_layout_v<T>,
                  "Page cast target must be standard layout");
    return reinterpret_cast<T *>(data_.data());
  }

  char *GetData() { return reinterpret_cast<char *>(data_.data()); }
  const char *GetConstData() const {
    return reinterpret_cast<const char *>(data_.data());
  }

private:
  std::array<std::uint8_t, PAGE_SIZE> data_{};
};

} // namespace mini
