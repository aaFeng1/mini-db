#include "parser/lexer.h"
#include <cctype>
#include <string_view>

namespace mini {

Token Lexer::PeekToken() {
  if (!lookahead_.has_value()) {
    lookahead_ = ScanToken();
  }
  return lookahead_.value();
}

Token Lexer::NextToken() {
  if (lookahead_.has_value()) {
    Token token = lookahead_.value();
    lookahead_.reset();
    return token;
  }
  return ScanToken();
}

bool Lexer::IsAtEnd() const { return IsAtEndInternal(); }

// TODO: 应该考虑没有调用isatend的情况
bool Lexer::Consume(TokenType expected) {
  Token token = PeekToken();
  if (token.GetType() == expected) {
    NextToken();
    return true;
  }
  return false;
}

std::vector<Token> Lexer::Tokenize() {
  std::vector<Token> tokens;
  while (1) {
    Token token = NextToken();
    tokens.push_back(token);
    if (token.GetType() == TokenType::TOKEN_EOF ||
        token.GetType() == TokenType::TOKEN_INVALID) {
      break;
    }
  }
  return tokens;
}

char Lexer::CurrentChar() const {
  if (IsAtEndInternal()) {
    return '\0';
  }
  return input_[position_];
}

char Lexer::PeekNextChar() const {
  if (position_ + 1 >= input_.size()) {
    return '\0';
  }
  return input_[position_ + 1];
}

void Lexer::AdvanceChar() {
  position_++;
  column_++;
}

bool Lexer::IsAtEndInternal() const { return position_ >= input_.size(); }

Token Lexer::ScanToken() {
  SkipWhitespace();

  if (IsAtEndInternal()) {
    return Token(TokenType::TOKEN_EOF, SourceSpan{position_, 0, line_, column_},
                 "");
  }

  char c = CurrentChar();

  if (isalpha(c) || c == '_') {
    return ScanIdentifierOrKeyword();
  } else if (std::isdigit(c)) {
    return ScanNumber();
  } else if (c == '\'') {
    return ScanString();
  } else {
    return ScanSymbol();
  }
}

void Lexer::SkipWhitespace() {
  if (CurrentChar() == ' ' || CurrentChar() == '\r' || CurrentChar() == '\t') {
    AdvanceChar();
  } else if (CurrentChar() == '\n') {
    line_++;
    column_ = 0;
    AdvanceChar();
  }
}

Token Lexer::ScanIdentifierOrKeyword() {
  SourceSpan span{position_, 0, line_, column_};
  size_t start_pos = position_;
  while (!IsAtEndInternal() &&
         (std::isalnum(CurrentChar()) || CurrentChar() == '_')) {
    AdvanceChar();
  }
  size_t length = position_ - start_pos;
  std::string_view lexeme(&input_[start_pos], length);
  span.length = position_ - span.start;
  TokenType type = MatchKeyword(lexeme);
  return Token(type, span, lexeme);
}

Token Lexer::ScanNumber() {
  // TODO: 不用管是否正确，只要遇到非数字就是结束，且v1不支持小数
  SourceSpan span{position_, 0, line_, column_};
  size_t start_pos = position_;
  while (!IsAtEndInternal() && std::isdigit(CurrentChar())) {
    AdvanceChar();
  }
  size_t length = position_ - start_pos;
  std::string_view number(&input_[start_pos], length);
  span.length = position_ - span.start;
  return Token(TokenType::TOKEN_NUMBER, span, number);
}

Token Lexer::ScanString() {
  SourceSpan span{position_, 0, line_, column_};
  AdvanceChar(); // Skip opening '

  size_t start_pos = position_;
  while (!IsAtEndInternal() && CurrentChar() != '\'') {
    AdvanceChar();
  }

  if (IsAtEndInternal()) {
    return MakeErrorToken(ErrorKind::ERROR_UNTERMINATED_STRING, span,
                          "Unterminated string literal");
  }

  size_t length = position_ - start_pos;
  std::string_view str(&input_[start_pos], length);

  AdvanceChar(); // Skip closing '

  span.length = position_ - span.start;
  return Token(TokenType::TOKEN_STRING, span, str);
}

Token Lexer::ScanSymbol() {
  char c = CurrentChar();
  size_t start_pos = position_;
  SourceSpan span{position_, 1, line_, column_};
  switch (c) {
  case '*':
    AdvanceChar();
    return Token(TokenType::TOKEN_STAR, span,
                 std::string_view(&input_[start_pos], 1));
  case ',':
    AdvanceChar();
    return Token(TokenType::TOKEN_COMMA, span,
                 std::string_view(&input_[start_pos], 1));
  case ';':
    AdvanceChar();
    return Token(TokenType::TOKEN_SEMICOLON, span,
                 std::string_view(&input_[start_pos], 1));
  case '(':
    AdvanceChar();
    return Token(TokenType::TOKEN_LEFT_PAREN, span,
                 std::string_view(&input_[start_pos], 1));
  case ')':
    AdvanceChar();
    return Token(TokenType::TOKEN_RIGHT_PAREN, span,
                 std::string_view(&input_[start_pos], 1));
  case '.':
    AdvanceChar();
    return Token(TokenType::TOKEN_DOT, span,
                 std::string_view(&input_[start_pos], 1));
  case '=':
    AdvanceChar();
    return Token(TokenType::TOKEN_EQUAL, span,
                 std::string_view(&input_[start_pos], 1));
  default:
    return MakeErrorToken(ErrorKind::ERROR_INVALID_CHARACTER, span,
                          "Invalid character");
  }
}

Token Lexer::MakeErrorToken(ErrorKind kind, SourceSpan span,
                            const std::string &message) {
  error_ = LexerError(kind, span, message);
  return Token(TokenType::TOKEN_INVALID, span, "");
}

TokenType Lexer::MatchKeyword(std::string_view lexeme) const {
  if (lexeme == "SELECT") {
    return TokenType::TOKEN_SELECT;
  } else if (lexeme == "FROM") {
    return TokenType::TOKEN_FROM;
  } else if (lexeme == "WHERE") {
    return TokenType::TOKEN_WHERE;
  } else if (lexeme == "INSERT") {
    return TokenType::TOKEN_INSERT;
  } else if (lexeme == "INTO") {
    return TokenType::TOKEN_INTO;
  } else if (lexeme == "VALUES") {
    return TokenType::TOKEN_VALUES;
  } else if (lexeme == "UPDATE") {
    return TokenType::TOKEN_UPDATE;
  } else if (lexeme == "SET") {
    return TokenType::TOKEN_SET;
  } else if (lexeme == "DELETE") {
    return TokenType::TOKEN_DELETE;
  }
  return TokenType::TOKEN_IDENTIFIER;
}

} // namespace mini
