#pragma once
#include "parser/lexer.h"
#include "parser/literal.h"
#include "parser/statement.h"
#include <memory>
#include <optional>
#include <string>

namespace mini {

class ParserError {
public:
  ParserError(ErrorKind kind, SourceSpan span, const std::string &message)
      : kind(kind), span(span), message(message) {}
  ~ParserError() = default;

private:
  ErrorKind kind;
  SourceSpan span;
  std::string message;
};

class Parser {
public:
  explicit Parser(std::unique_ptr<Lexer> lexer) : lexer_(std::move(lexer)) {}
  ~Parser() = default;

  // 解析入口函数
  std::unique_ptr<Statement> ParseStatement();
  std::unique_ptr<Statement> ParseInsertStatement();
  std::unique_ptr<Statement> ParseSelectStatement();

  bool HasError() const { return error_.has_value(); }
  ParserError GetError() const { return error_.value(); }

private:
  //如果下一个token类型匹配expected则消费它并返回，否则记录错误
  Token Expect(TokenType expected);

  std::unique_ptr<Lexer> lexer_;
  std::optional<ParserError> error_;
};

} // namespace mini
