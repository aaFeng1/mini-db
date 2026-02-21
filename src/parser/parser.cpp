#include "parser/parser.h"
#include "parser/lexer.h"
#include "parser/literal.h"
#include "parser/statement.h"
#include <iostream>

namespace mini {

std::unique_ptr<Statement> Parser::ParseStatement() {
  if (error_.has_value())
    return nullptr;

  switch (lexer_->PeekToken().GetType()) {
  case TokenType::TOKEN_INSERT:
    return ParseInsertStatement();
  case TokenType::TOKEN_SELECT:
    return ParseSelectStatement();
  case TokenType::TOKEN_CREATE:
    return ParseCreateTableStatement();
  default:
    error_ = ParserError(ErrorKind::ERROR_UNSUPPORTED_TOKEN,
                         lexer_->PeekToken().GetSpan(), "unkonw statement.");
    return nullptr;
  }
}

std::unique_ptr<Statement> Parser::ParseInsertStatement() {
  Expect(TokenType::TOKEN_INSERT);
  Expect(TokenType::TOKEN_INTO);
  Token table_name = Expect(TokenType::TOKEN_IDENTIFIER);
  Expect(TokenType::TOKEN_VALUES);
  Expect(TokenType::TOKEN_LEFT_PAREN);
  std::vector<std::unique_ptr<Literal>> values;
  do {
    Token next = lexer_->PeekToken();
    if (next.GetType() == TokenType::TOKEN_RIGHT_PAREN) {
      break;
    }
    if (next.GetType() == TokenType::TOKEN_COMMA) {
      Expect(TokenType::TOKEN_COMMA);
      continue;
    }

    if (next.GetType() == TokenType::TOKEN_STRING) {
      Token str_token = Expect(TokenType::TOKEN_STRING);
      values.push_back(std::make_unique<StringLiteral>(str_token.GetLexeme()));
    } else if (next.GetType() == TokenType::TOKEN_NUMBER) {
      Token int_token = Expect(TokenType::TOKEN_NUMBER);
      values.push_back(std::make_unique<IntLiteral>(int_token.GetLexeme()));
    } else {
      error_ = ParserError(ErrorKind::ERROR_UNSUPPORTED_TOKEN, next.GetSpan(),
                           "Expected literal value.");
      return nullptr;
    }
  } while (true);
  Expect(TokenType::TOKEN_RIGHT_PAREN);
  Expect(TokenType::TOKEN_SEMICOLON);
  return std::make_unique<InsertStatement>(std::string(table_name.GetLexeme()),
                                           std::move(values));
}

std::unique_ptr<Statement> Parser::ParseSelectStatement() {
  // TODO: v1 only support "SELECT * FROM table_name;"
  Expect(TokenType::TOKEN_SELECT);
  Expect(TokenType::TOKEN_STAR);
  Expect(TokenType::TOKEN_FROM);
  Token table_name = Expect(TokenType::TOKEN_IDENTIFIER);
  Expect(TokenType::TOKEN_SEMICOLON);
  return std::make_unique<SelectStatement>(std::string(table_name.GetLexeme()));
}

std::unique_ptr<Statement> Parser::ParseCreateTableStatement() {
  // TODO: only support "CREATE TABLE table_name (col_name col_type,...);"
  Expect(TokenType::TOKEN_CREATE);
  Expect(TokenType::TOKEN_TABLE);
  Token table_name = Expect(TokenType::TOKEN_IDENTIFIER);
  Expect(TokenType::TOKEN_LEFT_PAREN);
  std::vector<std::pair<std::string, DataType>> columns;
  do {
    Token next = lexer_->PeekToken();
    if (next.GetType() == TokenType::TOKEN_RIGHT_PAREN) {
      break;
    }
    if (next.GetType() == TokenType::TOKEN_COMMA) {
      Expect(TokenType::TOKEN_COMMA);
      continue;
    }

    Token col_name_token = Expect(TokenType::TOKEN_IDENTIFIER);
    Token col_type_token = Expect(TokenType::TOKEN_IDENTIFIER);
    std::string col_name(col_name_token.GetLexeme());
    std::string col_type_str(col_type_token.GetLexeme());
    DataType col_type;
    if (col_type_str == "INT") {
      col_type = DataType::INTEGER;
    } else if (col_type_str == "STRING") {
      col_type = DataType::VARCHAR;
    } else {
      error_ = ParserError(ErrorKind::ERROR_UNSUPPORTED_TOKEN,
                           col_type_token.GetSpan(), "Unsupported data type.");
      return nullptr;
    }
    columns.emplace_back(col_name, col_type);
  } while (true);
  Expect(TokenType::TOKEN_RIGHT_PAREN);
  Expect(TokenType::TOKEN_SEMICOLON);
  return std::make_unique<CreateTableStatement>(
      std::string(table_name.GetLexeme()), std::move(columns));
}

Token Parser::Expect(TokenType expected) {
  if (lexer_->HasError()) {
    auto lexer_error = lexer_->GetError();
    error_ = ParserError(lexer_error.GetType(), lexer_error.GetSpan(),
                         lexer_error.GetMessage());
  }

  if (error_.has_value()) {
    return Token(TokenType::TOKEN_INVALID, {0, 0, 0, 0}, "");
  }

  Token token = lexer_->NextToken();
  if (token.GetType() != expected) {
    error_ = ParserError(ErrorKind::ERROR_UNSUPPORTED_TOKEN, token.GetSpan(),
                         "Unexpected token.");
  }
  return token;
}

} // namespace mini
