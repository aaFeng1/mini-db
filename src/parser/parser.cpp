#include "parser/parser.h"
#include "parser/lexer.h"
#include "parser/literal.h"
#include "parser/statement.h"
#include "type/data_type.h"
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
  case TokenType::TOKEN_CREATE: {
    Expect(TokenType::TOKEN_CREATE);
    Token next = lexer_->PeekToken();
    if (next.GetType() == TokenType::TOKEN_TABLE) {
      return ParseCreateTableStatement();
    } else if (next.GetType() == TokenType::TOKEN_INDEX) {
      return ParseCreateIndexStatement();
    } else {
      error_ = ParserError(ErrorKind::ERROR_UNSUPPORTED_TOKEN, next.GetSpan(),
                           "Expected TABLE or INDEX after CREATE.");
      return nullptr;
    }
  }
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
  Expect(TokenType::TOKEN_TABLE);
  Token table_name = Expect(TokenType::TOKEN_IDENTIFIER);
  Expect(TokenType::TOKEN_LEFT_PAREN);
  std::vector<std::pair<std::string, ColumnType>> columns;
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
    if (col_type_str == "INT") {
      columns.emplace_back(col_name, ColumnType::Integer());
    } else if (col_type_str == "VARCHAR") {
      // VARCHAR(length)
      Expect(TokenType::TOKEN_LEFT_PAREN);

      Token length_token = Expect(TokenType::TOKEN_NUMBER);
      uint32_t length = std::stoul(std::string(length_token.GetLexeme()));
      columns.emplace_back(col_name, ColumnType::Varchar(length));

      Expect(TokenType::TOKEN_RIGHT_PAREN);
    } else {
      error_ = ParserError(ErrorKind::ERROR_UNSUPPORTED_TOKEN,
                           col_type_token.GetSpan(), "Unsupported data type.");
      return nullptr;
    }
  } while (true);
  Expect(TokenType::TOKEN_RIGHT_PAREN);
  Expect(TokenType::TOKEN_SEMICOLON);
  return std::make_unique<CreateTableStatement>(
      std::string(table_name.GetLexeme()), std::move(columns));
}

std::unique_ptr<Statement> Parser::ParseCreateIndexStatement() {
  // CREATE INDEX idx_stu_id ON stu(id);
  Expect(TokenType::TOKEN_INDEX);
  Token index_name = Expect(TokenType::TOKEN_IDENTIFIER);
  Expect(TokenType::TOKEN_ON);
  Token table_name = Expect(TokenType::TOKEN_IDENTIFIER);
  Expect(TokenType::TOKEN_LEFT_PAREN);
  std::vector<std::string> column_names;
  // TODO: only support single column index in v1
  Token column_name_token = Expect(TokenType::TOKEN_IDENTIFIER);
  column_names.push_back(std::string(column_name_token.GetLexeme()));
  Expect(TokenType::TOKEN_RIGHT_PAREN);
  Expect(TokenType::TOKEN_SEMICOLON);
  return std::make_unique<CreateIndexStatement>(
      std::string(index_name.GetLexeme()), std::string(table_name.GetLexeme()),
      std::move(column_names));
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
