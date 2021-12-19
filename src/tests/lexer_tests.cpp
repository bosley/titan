

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
         {compiler::Token::L_BRACE, {}, nullptr},
         {compiler::Token::R_BRACE, {}, nullptr},
         {compiler::Token::L_BRACKET, {}, nullptr},
         {compiler::Token::R_BRACKET, {}, nullptr},
         {compiler::Token::L_PAREN, {}, nullptr},
         {compiler::Token::R_PAREN, {}, nullptr},
         {compiler::Token::LT, {}, nullptr},
         {compiler::Token::GT, {}, nullptr},
         {compiler::Token::COMMA, {}, nullptr},
         {compiler::Token::PERIOD, {}, nullptr},
         {compiler::Token::QUESTION_MARK, {}, nullptr},
         {compiler::Token::SEMICOLON, {}, nullptr},
         {compiler::Token::COLON, {}, nullptr},
         {compiler::Token::SINGLE_QUOTE, {}, nullptr},
         {compiler::Token::ADD, {}, nullptr},
         {compiler::Token::SUB, {}, nullptr},
         {compiler::Token::MUL, {}, nullptr},
         {compiler::Token::DIV, {}, nullptr},
         {compiler::Token::OCTOTHORPE, {}, nullptr},
         {compiler::Token::AT, {}, nullptr},
         {compiler::Token::DOLLAR, {}, nullptr},
         {compiler::Token::AMPERSAND, {}, nullptr},
         {compiler::Token::PIPE, {}, nullptr},
         {compiler::Token::TILDE, {}, nullptr},
         {compiler::Token::HAT, {}, nullptr},
         {compiler::Token::MOD, {}, nullptr},
         {compiler::Token::ADD_EQ, {}, nullptr},
         {compiler::Token::SUB_EQ, {}, nullptr},
         {compiler::Token::DIV_EQ, {}, nullptr},
         {compiler::Token::MUL_EQ, {}, nullptr},
         {compiler::Token::MOD_EQ, {}, nullptr},
         {compiler::Token::EQ, {}, nullptr},
         {compiler::Token::EQ_EQ, {}, nullptr},
         {compiler::Token::POW, {}, nullptr},
         {compiler::Token::POW_EQ, {}, nullptr},
         {compiler::Token::ARROW, {}, nullptr},
         {compiler::Token::OR, {}, nullptr},
         {compiler::Token::AND, {}, nullptr},
         {compiler::Token::EXCLAMATION, {}, nullptr},
         {compiler::Token::EXCLAMATION_EQ, {}, nullptr},
         {compiler::Token::IDENTIFIER, "variable", nullptr},
         {compiler::Token::LITERAL_NUMBER, "43", nullptr},
         {compiler::Token::LITERAL_FLOAT, "3.14159", nullptr},
         {compiler::Token::FN, {}, nullptr},
         {compiler::Token::IDENTIFIER, "variable_name", nullptr},
         {compiler::Token::IDENTIFIER, "variable_name_something_422", nullptr},
         {compiler::Token::FN, {}, nullptr},
         {compiler::Token::IDENTIFIER, "main", nullptr},
         {compiler::Token::L_PAREN, {}, nullptr},
         {compiler::Token::R_PAREN, {}, nullptr},
         {compiler::Token::ARROW, {}, nullptr},
         {compiler::Token::IDENTIFIER, "some_return_type", nullptr},
         {compiler::Token::L_BRACE, {}, nullptr},
         {compiler::Token::R_BRACE, {}, nullptr},
         {compiler::Token::IF, {}, nullptr},
         {compiler::Token::ELSE, {}, nullptr},
         {compiler::Token::FOR, {}, nullptr},
         {compiler::Token::WHILE, {}, nullptr},
         {compiler::Token::RETURN, {}, nullptr},
         {compiler::Token::BREAK, {}, nullptr},
         {compiler::Token::LET, {}, nullptr},
         {compiler::Token::IDENTIFIER, "x", nullptr},
         {compiler::Token::EQ, {}, nullptr},
         {compiler::Token::LITERAL_NUMBER, "3", nullptr},
         {compiler::Token::SEMICOLON, {}, nullptr},
         {compiler::Token::IMPORT, {}, nullptr},
         {compiler::Token::STRING, "This is a string", nullptr},
         {compiler::Token::STRING, "This \\\"is a string\\\"", nullptr},
         {compiler::Token::STRING,
          "This \\\"is a string\\\" as well as \\\"this\\\", see?", nullptr},
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
      CHECK_EQUAL(tc.tdp[i].data, tokens[i].data);
    }
  }
}