#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mini {

enum class TokenType {
  // TOKEN_KEYWORD
  TOKEN_SELECT,
  TOKEN_FROM,
  TOKEN_WHERE,
  TOKEN_INSERT,
  TOKEN_INTO,
  TOKEN_VALUES,
  TOKEN_UPDATE,
  TOKEN_SET,
  TOKEN_DELETE,
  TOKEN_CREATE,
  TOKEN_TABLE,

  // Literals
  TOKEN_IDENTIFIER,
  TOKEN_NUMBER,
  TOKEN_STRING,

  // Symbols
  TOKEN_STAR,        //*
  TOKEN_COMMA,       //,
  TOKEN_SEMICOLON,   //;
  TOKEN_LEFT_PAREN,  //(
  TOKEN_RIGHT_PAREN, //)
  // not in v1
  TOKEN_DOT,   //.
  TOKEN_EQUAL, //=

  // Special tokens
  TOKEN_EOF,
  TOKEN_INVALID,
  // Add more token types as needed
};

enum class ErrorKind {
  ERROR_INVALID_CHARACTER,   //非法字符
  ERROR_UNTERMINATED_STRING, //不完整字符串
  ERROR_INVALID_NUMBER,      //无效数字
  ERROR_UNSUPPORTED_TOKEN,   //不支持的token
  // Add more error kinds as needed
};

struct SourceSpan {
  size_t start;
  size_t length;
  int line;
  int column;
};

class Token {
public:
  Token(TokenType type, SourceSpan span, std::string_view lexeme)
      : type(type), span(span), lexeme(lexeme) {}
  ~Token() = default;

  TokenType GetType() const { return type; }
  SourceSpan GetSpan() const { return span; }
  std::string_view GetLexeme() const { return lexeme; }

private:
  TokenType type;
  SourceSpan span;
  std::string_view lexeme;
  // TODO: add payload for literals (numbers, strings, etc.)
};

class LexerError {
public:
  LexerError(ErrorKind kind, SourceSpan span, const std::string &message)
      : kind(kind), span(span), message(message) {}
  ~LexerError() = default;

  ErrorKind GetType() const { return kind; }
  SourceSpan GetSpan() const { return span; }
  std::string GetMessage() const { return message; }

private:
  ErrorKind kind;
  SourceSpan span;
  std::string message;
};

class Lexer {
public:
  explicit Lexer(std::string input) : input_(std::move(input)), position_(0){};
  ~Lexer() = default;

  Token PeekToken();    //查看下一个token但不消费它
  Token NextToken();    //获取下一个token并消费它
  bool IsAtEnd() const; //是否到达输入末尾

  //如果下一个token类型匹配expected则消费它并返回true，否则返回false
  bool Consume(TokenType expected);

  std::vector<Token> Tokenize(); //将整个输入标记化为token序列
  size_t GetPosition() const { return position_; }

  bool HasError() const { return error_.has_value(); }
  LexerError GetError() const { return error_.value(); }

private:
  // 游标操作
  char CurrentChar() const;     //获取当前字符但不消费它
  char PeekNextChar() const;    //查看当前字符但不消费它
  void AdvanceChar();           //前进游标
  bool IsAtEndInternal() const; //检查是否到达输入末尾

  // 扫描函数
  Token ScanToken(); // 扫描下一个token,跳过空白字符，分流到具体扫描函数
  void SkipWhitespace();
  Token ScanIdentifierOrKeyword();
  Token ScanNumber();
  Token ScanString();
  Token ScanSymbol();
  Token MakeErrorToken(ErrorKind kind, SourceSpan span,
                       const std::string &message);

  TokenType MatchKeyword(std::string_view lexeme) const;

  std::string input_;
  size_t position_;
  int line_{1};
  int column_{1};
  std::optional<Token> lookahead_;  // 缓存下一个token以支持PeekToken
  std::optional<LexerError> error_; // 缓存第一个遇到的错误
};

} // namespace mini
