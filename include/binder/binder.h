#pragma once
#include "binder/bound_statement.h"
#include "binder/value.h"
#include "catalog/catalog.h"
#include "parser/lexer.h"
#include "parser/literal.h"
#include "parser/parser.h"
#include "parser/statement.h"
#include <memory>

namespace mini {

class BindError {
public:
  explicit BindError(std::string message, SourceSpan span)
      : message_(std::move(message)), span_(span) {}
  std::string Message() const { return message_; }
  SourceSpan Span() const { return span_; }

private:
  std::string message_;
  SourceSpan span_;
};

class Binder {
public:
  Binder(Catalog &catalog) : catalog_(catalog) {}
  ~Binder() = default;

  std::unique_ptr<BoundStatement> BindStatement(const Statement &statement);

  std::unique_ptr<BoundStatement> BindInsert(const InsertStatement &);
  std::unique_ptr<BoundStatement> BindSelect(const SelectStatement &);

  bool HasError() const { return error_.has_value(); }
  BindError GetError() const { return error_.value(); }

private:
  Catalog &catalog_;
  std::optional<BindError> error_;
};

} // namespace mini
