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
  explicit StringLiteral(std::string_view value) : value_(value) {}
  ~StringLiteral() override = default;

  std::string_view value() const { return value_; }

private:
  std::string_view value_;
};

class IntLiteral : public Literal {
public:
  explicit IntLiteral(std::string_view value) : value_(value) {}
  ~IntLiteral() override = default;

  std::string_view value() const { return value_; }

private:
  std::string_view value_;
};

} // namespace mini
