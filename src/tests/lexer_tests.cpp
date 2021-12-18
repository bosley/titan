

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
         {compiler::Token::L_BRACE, {}},
         {compiler::Token::R_BRACE, {}},
         {compiler::Token::L_BRACKET, {}},
         {compiler::Token::R_BRACKET, {}},
         {compiler::Token::L_PAREN, {}},
         {compiler::Token::R_PAREN, {}},
         {compiler::Token::LT, {}},
         {compiler::Token::GT, {}},
         {compiler::Token::COMMA, {}},
         {compiler::Token::PERIOD, {}},
         {compiler::Token::QUESTION_MARK, {}},
         {compiler::Token::SEMICOLON, {}},
         {compiler::Token::COLON, {}},
         {compiler::Token::SINGLE_QUOTE, {}},
         {compiler::Token::DOUBLE_QUOTE, {}},
         {compiler::Token::ADD, {}},
         {compiler::Token::SUB, {}},
         {compiler::Token::MUL, {}},
         {compiler::Token::DIV, {}},
         {compiler::Token::OCTOTHORPE, {}},
         {compiler::Token::AT, {}},
         {compiler::Token::DOLLAR, {}},
         {compiler::Token::AMPERSAND, {}},
         {compiler::Token::PIPE, {}},
         {compiler::Token::TILDE, {}},
         {compiler::Token::HAT, {}},
         {compiler::Token::MOD, {}},
         {compiler::Token::ADD_EQ, {}},
         {compiler::Token::SUB_EQ, {}},
         {compiler::Token::DIV_EQ, {}},
         {compiler::Token::MUL_EQ, {}},
         {compiler::Token::MOD_EQ, {}},
         {compiler::Token::EQ, {}},
         {compiler::Token::EQ_EQ, {}},
         {compiler::Token::POW, {}},
         {compiler::Token::POW_EQ, {}},
         {compiler::Token::ARROW, {}},
         {compiler::Token::OR, {}},
         {compiler::Token::AND, {}},
         {compiler::Token::EXCLAMATION, {}},
         {compiler::Token::EXCLAMATION_EQ, {}},
         {compiler::Token::IDENTIFIER, "variable"},
         {compiler::Token::LITERAL_NUMBER, "43"},
         {compiler::Token::LITERAL_FLOAT, "3.14159"},
         {compiler::Token::FN, {}},
         {compiler::Token::IDENTIFIER, "variable_name"},
         {compiler::Token::IDENTIFIER, "variable_name_something_422"},
         {compiler::Token::FN, {}},
         {compiler::Token::IDENTIFIER, "main"},
         {compiler::Token::L_PAREN, {}},
         {compiler::Token::R_PAREN, {}},
         {compiler::Token::ARROW, {}},
         {compiler::Token::IDENTIFIER, "some_return_type"},
         {compiler::Token::L_BRACE, {}},
         {compiler::Token::R_BRACE, {}},
     }}};

} // namespace

TEST_GROUP(lexer_tests){};

//  Load the text files and ensure the expected tokens match the input
//
TEST(lexer_tests, all_tokens) {
  compiler::lexer lexer;
  for (auto &tc : tcs) {
    std::vector<compiler::TD_Pair> tokens;
    CHECK_TRUE(lexer.load_file(tc.file));
    CHECK_TRUE(lexer.lex(tokens));
    CHECK_TRUE(tokens.size() == tc.tdp.size());
    for (size_t i = 0; i < tokens.size(); i++) {
      CHECK_EQUAL(static_cast<int>(tokens[i].token),
                  static_cast<int>(tc.tdp[i].token));
    }
  }
}