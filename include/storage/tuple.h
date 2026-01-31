#pragma once
#include "common/rid.h"
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace mini {

class Tuple {
public:
  Tuple() = default;
  explicit Tuple(std::vector<char> data) : data_(std::move(data)) {}

  Tuple(const char *data, size_t size) : data_(data, data + size) {}

  const char *Data() const { return data_.data(); }
  char *Data() { return data_.data(); }
  uint32_t Size() const { return static_cast<uint32_t>(data_.size()); }
  bool Empty() const { return data_.empty(); }

  const RID &GetRid() const { return rid_; }
  void SetRid(const RID &rid) { rid_ = rid; }

  // 1) 从指针+长度复制（最常用：从 page 里读出来）
  void SetData(const char *src, uint32_t len) {
    if (len == 0) {
      data_.clear();
      return;
    }
    // 允许 src==nullptr 但 len==0；若 len>0 则 src 必须有效
    if (src == nullptr) {
      throw std::invalid_argument("Tuple::SetData: src is null but len > 0");
    }
    data_.assign(src, src + len);
  }

  // 2) 接管一个 vector（避免拷贝：你自己组装 tuple bytes 时用）
  void SetData(std::vector<char> &&data) { data_ = std::move(data); }

  // 3) 预分配并返回可写指针（如果你想 memcpy 写入）
  char *Resize(uint32_t len) {
    data_.resize(len);
    return data_.empty() ? nullptr : data_.data();
  }

private:
  std::vector<char> data_;
  RID rid_{};
};

} // namespace mini
