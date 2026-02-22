#include "parser/lexer.h"
#include "parser/literal.h"
#include "parser/parser.h"
#include "parser/statement.h"
#include <gtest/gtest.h>
#include <memory>

using namespace mini;

class ParserTest : public ::testing::Test {

protected:
  std::unique_ptr<Lexer> lexer_;
  std::unique_ptr<Parser> parser_;
  void SetUp() override {}

  void TearDown() override {
    lexer_.reset();
    parser_.reset();
  }
};

// SELECT * FROM t;
TEST_F(ParserTest, SelectAllFromTable) {
  std::string query = "SELECT * FROM t;";
  lexer_ = std::make_unique<Lexer>(query);

  // EXPECT_EQ(lexer_->PeekToken().GetType(), TokenType::TOKEN_SELECT);//

  parser_ = std::make_unique<Parser>(std::move(lexer_));
  auto stmt = parser_->ParseStatement();
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->Type(), StatementType::SELECT);
  auto select_stmt = static_cast<SelectStatement *>(stmt.get());
  ASSERT_EQ(select_stmt->Table_name(), "t");
  ASSERT_TRUE(select_stmt->Select_all());
}

// INSERT INTO temp VALUES (1, 'a');
// INSERT INTO t VALUES (1, 'a');
TEST_F(ParserTest, InsertIntoTable) {
  std::string query = "INSERT INTO t VALUES (1, 'a');";
  lexer_ = std::make_unique<Lexer>(query);
  parser_ = std::make_unique<Parser>(std::move(lexer_));
  auto stmt = parser_->ParseStatement();
  ASSERT_EQ(stmt->Type(), StatementType::INSERT);
  auto insert_stmt = static_cast<InsertStatement *>(stmt.get());
  ASSERT_EQ(insert_stmt->Table_name(), "t");
  const auto &values = insert_stmt->Values();
  ASSERT_EQ(values.size(), 2);
  auto int_literal = static_cast<IntLiteral *>(values[0].get());
  ASSERT_EQ(int_literal->value(), "1");
  auto str_literal = static_cast<StringLiteral *>(values[1].get());
  ASSERT_EQ(str_literal->value(), "a");
}

// CREATE TABLE temp (id INT, name VARCHAR(10));
// CREATE TABLE t (id INT, name VARCHAR(255));
TEST_F(ParserTest, CreateTable) {
  std::string query = "CREATE TABLE t (id INT, name VARCHAR(255));";
  lexer_ = std::make_unique<Lexer>(query);
  parser_ = std::make_unique<Parser>(std::move(lexer_));
  auto stmt = parser_->ParseStatement();
  ASSERT_EQ(stmt->Type(), StatementType::CREATE_TABLE);
  auto create_table_stmt = static_cast<CreateTableStatement *>(stmt.get());
  ASSERT_EQ(create_table_stmt->Table_name(), "t");
  const auto &columns = create_table_stmt->Columns();
  ASSERT_EQ(columns.size(), 2);
  ASSERT_EQ(columns[0].first, "id");
  ASSERT_EQ(columns[0].second.type, DataType::INTEGER);
  ASSERT_EQ(columns[1].first, "name");
  ASSERT_EQ(columns[1].second.type, DataType::VARCHAR);
  ASSERT_EQ(columns[1].second.length, 255);
}
