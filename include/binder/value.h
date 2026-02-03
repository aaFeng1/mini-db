#pragma once
#include <cstdlib>
#include <string>
#include <string_view>
#include <utility>

namespace mini {
enum class ValueType { INT, STRING };

class Value {
public:
  Value() = default;
  virtual ~Value() = default;
  virtual ValueType Type() const = 0;
};

class IntValue : public Value {
public:
  // TODO: 将构造函数参数换为literal类型
  IntValue(std::string value) : value_(std::stoi(value)) {}
  IntValue(std::string_view value) : value_(std::stoi(std::string(value))) {}
  ~IntValue() override = default;
  ValueType Type() const override { return ValueType::INT; }
  int GetValue() const { return value_; }

private:
  int value_;
};

class StringValue : public Value {
public:
  StringValue(std::string value) : value_(std::move(value)) {}
  StringValue(std::string_view value) : value_(value) {}
  ~StringValue() override = default;
  ValueType Type() const override { return ValueType::STRING; }
  const std::string &GetValue() const { return value_; }

private:
  std::string value_;
};

} // namespace mini
