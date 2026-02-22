#pragma once
#include "type/data_type.h"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace mini {

class Value {
public:
  Value() = default;
  virtual ~Value() = default;
  virtual DataType Type() const = 0;
  virtual std::string ToString() const = 0;
};

class IntValue : public Value {
public:
  // TODO: 将构造函数参数换为literal类型
  explicit IntValue(int32_t v) : value_(v) {}
  explicit IntValue(std::string value) : value_(std::stoi(value)) {}
  explicit IntValue(std::string_view value)
      : value_(std::stoi(std::string(value))) {}
  ~IntValue() override = default;
  DataType Type() const override { return DataType::INTEGER; }
  int32_t GetValue() const { return value_; }
  std::string ToString() const override { return std::to_string(value_); }

private:
  int32_t value_;
};

class StringValue : public Value {
public:
  explicit StringValue(std::string value) : value_(std::move(value)) {}
  explicit StringValue(std::string_view value) : value_(value) {}
  ~StringValue() override = default;
  DataType Type() const override { return DataType::VARCHAR; }
  const std::string &GetValue() const { return value_; }
  std::string ToString() const override { return value_; }

private:
  std::string value_;
};

} // namespace mini
