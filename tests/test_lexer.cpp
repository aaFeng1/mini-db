#include "parser/lexer.h"
#include <gtest/gtest.h>

using namespace mini;

// Basic test for the lexer to tokenize a simple SQL query
TEST(LexerTest, BasicTokens) {
  std::string input = "SELECT * FROM table1 WHERE id = 10;";
  Lexer lexer(input);
  EXPECT_EQ(lexer.PeekToken().GetType(), TokenType::TOKEN_SELECT);
  EXPECT_EQ(lexer.PeekToken().GetType(), TokenType::TOKEN_SELECT);
  EXPECT_EQ(lexer.PeekToken().GetType(), TokenType::TOKEN_SELECT);
  std::vector<Token> tokens = lexer.Tokenize();

  std::vector<TokenType> expected_types = {
      TokenType::TOKEN_SELECT,    TokenType::TOKEN_STAR,
      TokenType::TOKEN_FROM,      TokenType::TOKEN_IDENTIFIER,
      TokenType::TOKEN_WHERE,     TokenType::TOKEN_IDENTIFIER,
      TokenType::TOKEN_EQUAL,     TokenType::TOKEN_NUMBER,
      TokenType::TOKEN_SEMICOLON, TokenType::TOKEN_EOF};

  ASSERT_EQ(tokens.size(), expected_types.size());
  for (size_t i = 0; i < tokens.size(); ++i) {
    EXPECT_EQ(tokens[i].GetType(), expected_types[i]);
  }
}

// Test for invalid character handling
TEST(LexerTest, InvalidCharacter) {
  std::string input = "SELECT @ FROM table1;";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.Tokenize();

  // Expect an invalid token for the '@' character
  bool found_invalid = false;
  for (const auto &token : tokens) {
    if (token.GetType() == TokenType::TOKEN_INVALID) {
      found_invalid = true;
      break;
    }
  }
  EXPECT_TRUE(found_invalid);
}

// Test for string literal tokenization
TEST(LexerTest, StringLiteral) {
  std::string input = "INSERT INTO table1 VALUES ('hello');";
  Lexer lexer(input);
  std::vector<Token> tokens = lexer.Tokenize();

  std::vector<TokenType> expected_types = {
      TokenType::TOKEN_INSERT,      TokenType::TOKEN_INTO,
      TokenType::TOKEN_IDENTIFIER,  TokenType::TOKEN_VALUES,
      TokenType::TOKEN_LEFT_PAREN,  TokenType::TOKEN_STRING,
      TokenType::TOKEN_RIGHT_PAREN, TokenType::TOKEN_SEMICOLON,
      TokenType::TOKEN_EOF};

  ASSERT_EQ(tokens.size(), expected_types.size());
  for (size_t i = 0; i < tokens.size(); ++i) {
    EXPECT_EQ(tokens[i].GetType(), expected_types[i]);
  }

  // Check that the string literal token is present
  bool found_string = false;
  for (const auto &token : tokens) {
    if (token.GetType() == TokenType::TOKEN_STRING) {
      found_string = true;
      EXPECT_EQ(token.GetLexeme(), "hello");
      break;
    }
  }
  EXPECT_TRUE(found_string);
}
