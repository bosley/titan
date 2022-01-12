#include "compiler/lexer.hpp"
#include <iostream>
#include <vector>

#include <CppUTest/TestHarness.h>

namespace {
struct TestCase {
  std::string file;
  std::vector<compiler::TD_Pair> tdp;
};

std::vector<TestCase> tcs = {
    {"test_files/lexer_test.txt",
     {
         {compiler::Token::L_BRACE, {}, 0},
         {compiler::Token::R_BRACE, {}, 0},
         {compiler::Token::L_BRACKET, {}, 0},
         {compiler::Token::R_BRACKET, {}, 0},
         {compiler::Token::L_PAREN, {}, 0},
         {compiler::Token::R_PAREN, {}, 0},
         {compiler::Token::LT, {}, 0},
         {compiler::Token::GT, {}, 0},
         {compiler::Token::COMMA, {}, 0},
         {compiler::Token::PERIOD, {}, 0},
         {compiler::Token::QUESTION_MARK, {}, 0},
         {compiler::Token::SEMICOLON, {}, 0},
         {compiler::Token::COLON, {}, 0},
         {compiler::Token::SINGLE_QUOTE, {}, 0},
         {compiler::Token::ADD, {}, 0},
         {compiler::Token::SUB, {}, 0},
         {compiler::Token::MUL, {}, 0},
         {compiler::Token::DIV, {}, 0},
         {compiler::Token::OCTOTHORPE, {}, 0},
         {compiler::Token::AT, {}, 0},
         {compiler::Token::DOLLAR, {}, 0},
         {compiler::Token::AMPERSAND, {}, 0},
         {compiler::Token::PIPE, {}, 0},
         {compiler::Token::TILDE, {}, 0},
         {compiler::Token::HAT, {}, 0},
         {compiler::Token::MOD, {}, 0},
         {compiler::Token::ADD_EQ, {}, 0},
         {compiler::Token::SUB_EQ, {}, 0},
         {compiler::Token::DIV_EQ, {}, 0},
         {compiler::Token::MUL_EQ, {}, 0},
         {compiler::Token::MOD_EQ, {}, 0},
         {compiler::Token::EQ, {}, 0},
         {compiler::Token::EQ_EQ, {}, 0},
         {compiler::Token::POW, {}, 0},
         {compiler::Token::POW_EQ, {}, 0},
         {compiler::Token::ARROW, {}, 0},
         {compiler::Token::OR, {}, 0},
         {compiler::Token::AND, {}, 0},
         {compiler::Token::EXCLAMATION, {}, 0},
         {compiler::Token::EXCLAMATION_EQ, {}, 0},
         {compiler::Token::IDENTIFIER, "variable", 0},
         {compiler::Token::LITERAL_NUMBER, "43", 0},
         {compiler::Token::LITERAL_FLOAT, "3.14159", 0},
         {compiler::Token::FN, {}, 0},
         {compiler::Token::IDENTIFIER, "variable_name", 0},
         {compiler::Token::IDENTIFIER, "variable_name_something_422", 0},
         {compiler::Token::FN, {}, 0},
         {compiler::Token::IDENTIFIER, "main", 0},
         {compiler::Token::L_PAREN, {}, 0},
         {compiler::Token::R_PAREN, {}, 0},
         {compiler::Token::ARROW, {}, 0},
         {compiler::Token::IDENTIFIER, "some_return_type", 0},
         {compiler::Token::L_BRACE, {}, 0},
         {compiler::Token::R_BRACE, {}, 0},
         {compiler::Token::IF, {}, 0},
         {compiler::Token::ELSE, {}, 0},
         {compiler::Token::FOR, {}, 0},
         {compiler::Token::WHILE, {}, 0},
         {compiler::Token::RETURN, {}, 0},
         {compiler::Token::BREAK, {}, 0},
         {compiler::Token::LET, {}, 0},
         {compiler::Token::IDENTIFIER, "x", 0},
         {compiler::Token::EQ, {}, 0},
         {compiler::Token::LITERAL_NUMBER, "3", 0},
         {compiler::Token::SEMICOLON, {}, 0},
         {compiler::Token::IMPORT, {}, 0},
         {compiler::Token::STRING, "This is a string", 0},
         {compiler::Token::STRING, "This \\\"is a string\\\"", 0},
         {compiler::Token::STRING,
          "This \\\"is a string\\\" as well as \\\"this\\\", see?", 0},
     }}};

} // namespace

TEST_GROUP(lexer_tests){};

//  Load the text files and ensure the expected tokens match the input
//
TEST(lexer_tests, all_tokens)
{
  compiler::lexer lexer;

  for (auto &tc : tcs) {
    std::vector<compiler::TD_Pair> tokens;
    CHECK_TRUE(lexer.load_file(tc.file));
    CHECK_TRUE(lexer.lex(tokens));
    CHECK_TRUE(tokens.size() == tc.tdp.size());
    for (size_t i = 0; i < tokens.size(); i++) {
      CHECK_EQUAL(static_cast<int>(tokens[i].token),
                  static_cast<int>(tc.tdp[i].token));
      if (!tc.tdp[i].data.empty()) {
        CHECK_EQUAL(tc.tdp[i].data, tokens[i].data);
      }
    }
  }
}
