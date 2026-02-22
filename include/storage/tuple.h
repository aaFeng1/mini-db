#pragma once
#include "binder/value.h"
#include "catalog/schema.h"
#include "common/rid.h"
#include <cstddef>
#include <cstring>
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

  std::unique_ptr<Value> GetValue(std::shared_ptr<Schema> schema,
                                  size_t col_idx) const {
    if (col_idx >= schema->GetColumnCount()) {
      throw std::out_of_range("Tuple::GetValue: column index out of range");
    }
    const Column &col = schema->GetColumn(col_idx);
    // 这里你需要根据自己的 Tuple 存储格式实现解析逻辑
    // 下面是一个示例，假设 Tuple 的 data_ 是列值的连续字节表示
    const char *ptr = Data();
    ptr += col.offset; // 定位到该列数据的起始位置
    if (col.type == DataType::INTEGER) {
      int32_t int_val;
      std::memcpy(&int_val, ptr, sizeof(int32_t));
      return std::make_unique<IntValue>(int_val);
    } else if (col.type == DataType::VARCHAR) {
      std::string str_val(ptr,
                          col.length); // 假设定长字符串，实际可能需要处理变长
      auto pos = str_val.find('\0'); // 找第一个 0
      if (pos != std::string::npos)
        str_val.resize(pos);
      return std::make_unique<StringValue>(str_val);
    } else {
      throw std::runtime_error("Tuple::GetValue: unsupported data type");
    }
  }

private:
  std::vector<char> data_;
  RID rid_{};
};

} // namespace mini
