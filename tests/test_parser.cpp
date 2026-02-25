#include "binder/value.h"
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

// CREATE INDEX idx_name ON t(name);
// --gtest_filter=ParserTest.CreateIndex
TEST_F(ParserTest, CreateIndex) {
  std::string query = "CREATE INDEX idx_name ON t(name);";
  lexer_ = std::make_unique<Lexer>(query);
  parser_ = std::make_unique<Parser>(std::move(lexer_));
  auto stmt = parser_->ParseStatement();
  ASSERT_EQ(stmt->Type(), StatementType::CREATE_INDEX);
  auto create_index_stmt = static_cast<CreateIndexStatement *>(stmt.get());
  ASSERT_EQ(create_index_stmt->Index_name(), "idx_name");
  ASSERT_EQ(create_index_stmt->Table_name(), "t");
  const auto &column_names = create_index_stmt->Column_names();
  ASSERT_EQ(column_names.size(), 1);
  ASSERT_EQ(column_names[0], "name");
}

// SELECT * FROM t WHERE id = 1;
TEST_F(ParserTest, SelectWithWhereClause) {
  std::string query = "SELECT * FROM t WHERE id = 1;";
  lexer_ = std::make_unique<Lexer>(query);
  parser_ = std::make_unique<Parser>(std::move(lexer_));
  auto stmt = parser_->ParseStatement();
  ASSERT_EQ(stmt->Type(), StatementType::SELECT);
  auto select_stmt = static_cast<SelectStatement *>(stmt.get());
  ASSERT_EQ(select_stmt->Table_name(), "t");
  ASSERT_TRUE(select_stmt->Select_all());
  ASSERT_TRUE(select_stmt->Has_where());
  ASSERT_EQ(select_stmt->Where_column(), "id");
  auto where_value = select_stmt->Where_value();
  ASSERT_NE(where_value, nullptr);
  auto int_literal = dynamic_cast<const IntValue *>(where_value);
  ASSERT_NE(int_literal, nullptr);
  ASSERT_EQ(int_literal->GetValue(), 1);
}
