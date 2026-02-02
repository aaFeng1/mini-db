#pragma once

#include <string>
#include <string_view>
namespace mini {

class Literal {
public:
  Literal() = default;
  virtual ~Literal() = default;

private:
};

class StringLiteral : public Literal {
public:
  explicit StringLiteral(std::string value) : value_(std::move(value)) {}
  explicit StringLiteral(std::string_view value) : value_(value) {}
  ~StringLiteral() override = default;

  std::string value() const { return value_; }

private:
  std::string value_;
};

class IntLiteral : public Literal {
public:
  explicit IntLiteral(int value) : value_(value) {}
  ~IntLiteral() override = default;

  int value() const { return value_; }

private:
  int value_;
};

} // namespace mini
