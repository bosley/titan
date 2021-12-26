

#include "compiler/lexer.hpp"
#include "compiler/parser.hpp"
#include <iostream>
#include <vector>

#include <CppUTest/TestHarness.h>

namespace 
{

  bool exprs_are_equal(compiler::parse_tree::expression* a, compiler::parse_tree::expression* b) {
    if(a->type != b->type) {
      std::cout << "Expression type mismatch" << std::endl;
      return false;
    }

    switch(a->type) {
      case compiler::parse_tree::node_type::ID:
      case compiler::parse_tree::node_type::RAW_FLOAT:
      case compiler::parse_tree::node_type::RAW_NUMBER:
      case compiler::parse_tree::node_type::RAW_STRING:
      case compiler::parse_tree::node_type::CALL:
        if(a->value != b->value) {
          std::cout << "Expression value mismatch (" << a->value << " != " << b->value << ")" << std::endl;
          return false;
        }
        return true;

      case compiler::parse_tree::node_type::INFIX:
      {
        auto a_infix_expr = reinterpret_cast<compiler::parse_tree::infix_expr*>(a);
        auto b_infix_expr = reinterpret_cast<compiler::parse_tree::infix_expr*>(b);
        
        if(!exprs_are_equal(a_infix_expr->left, b_infix_expr->left)) {
          return false;
        }
        if(!exprs_are_equal(a_infix_expr->right, b_infix_expr->right)) {
          return false;
        }
        return true;
      }
      case compiler::parse_tree::node_type::PREFIX:
      {
        auto a_prefix_expr = reinterpret_cast<compiler::parse_tree::prefix_expr*>(a);
        auto b_prefix_expr = reinterpret_cast<compiler::parse_tree::prefix_expr*>(b);
        if(!exprs_are_equal(a_prefix_expr->right, b_prefix_expr->right)) {
          return false;
        }
        break;
      }
      case compiler::parse_tree::node_type::ARRAY_IDX:
      {
        auto a_array_idx_expr = reinterpret_cast<compiler::parse_tree::array_index_expr*>(a);
        auto b_array_idx_expr = reinterpret_cast<compiler::parse_tree::array_index_expr*>(b);
         
        if(!exprs_are_equal(a_array_idx_expr->arr, b_array_idx_expr->arr)) {
          return false;
        }
        if(!exprs_are_equal(a_array_idx_expr->index, b_array_idx_expr->index)) {
          return false;
        }
        return true;
      }
      default:
        std::cout << "Unhandled expression node type" << std::endl;
        return false;
    }
    return false;
  }


}



TEST_GROUP(parser_tests){};

//  Load the text files and ensure the expected tokens match the input
//
TEST(parser_tests, basic_function) {

  struct TestCase {
    compiler::parse_tree::toplevel::tl_type type;
    std::string name;
    std::vector<compiler::parse_tree::variable> parameters;
    std::vector<compiler::parse_tree::element *> element_list;
    compiler::parse_tree::variable_types return_type;
  };

  std::vector<TestCase> tcs = {
      {compiler::parse_tree::toplevel::tl_type::FUNCTION,
       "main",
       {
           {"argc", compiler::parse_tree::variable_types::U8, 0},
           {"argv", compiler::parse_tree::variable_types::STRING,
            std::numeric_limits<uint64_t>::max()},
       },
       {/* No elements */},
       compiler::parse_tree::variable_types::I64},
      {compiler::parse_tree::toplevel::tl_type::FUNCTION,
       "test",
       {/* No parameters */},
       {/* No elements */},
       compiler::parse_tree::variable_types::U16},
      {compiler::parse_tree::toplevel::tl_type::FUNCTION,
       "test1",
       {
           {"a", compiler::parse_tree::variable_types::U32,
            std::numeric_limits<uint64_t>::max()},
           {"b", compiler::parse_tree::variable_types::I8,
            std::numeric_limits<uint64_t>::max()},
           {"c", compiler::parse_tree::variable_types::STRING, 0},

       },
       {/* No elements */},
       compiler::parse_tree::variable_types::I16},
  };

  //  Lex the file(s)
  //
  std::string file = "test_files/parser_basic_function.tl";
  compiler::lexer lexer;
  std::vector<compiler::TD_Pair> tokens;
  CHECK_TRUE(lexer.load_file(file));
  CHECK_TRUE(lexer.lex(tokens));

  //  Parse the Token Data pairs
  //
  constexpr auto import_file =
      [](std::string file) -> std::vector<compiler::TD_Pair> {
    //  Test file will not be importing
    //
    return {};
  };

  compiler::parser parser;
  std::vector<std::string> include_directories;
  std::vector<compiler::parse_tree::toplevel *> functions =
      parser.parse(file, include_directories, import_file, tokens);

  CHECK_EQUAL(tcs.size(), functions.size());

  for (size_t i = 0; i < tcs.size(); i++) {

    CHECK_EQUAL((int)tcs[i].type, (int)functions[i]->type);

    compiler::parse_tree::function *f =
        static_cast<compiler::parse_tree::function *>(functions[i]);

    CHECK_EQUAL(tcs[i].name, f->name);
    CHECK_EQUAL((int)tcs[i].return_type, (int)f->return_type);
    CHECK_EQUAL(tcs[i].parameters.size(), f->parameters.size());
    CHECK_EQUAL(tcs[i].element_list.size(), f->element_list.size());
    CHECK_EQUAL(0, f->element_list.size());

    for (size_t p = 0; p < tcs[i].parameters.size(); p++) {

      CHECK_EQUAL(tcs[i].parameters[p].name, f->parameters[p].name);
      CHECK_EQUAL((int)tcs[i].parameters[p].type, (int)f->parameters[p].type);
      CHECK_EQUAL(tcs[i].parameters[p].depth, f->parameters[p].depth);
    }
  }
}
TEST(parser_tests, assignments) {

  std::vector<compiler::parse_tree::assignment> expected;

  expected.push_back(compiler::parse_tree::assignment(
      5, {"d", compiler::parse_tree::variable_types::U32, 0}, nullptr));
  expected.push_back(compiler::parse_tree::assignment(
      6, {"e", compiler::parse_tree::variable_types::U16, 12}, nullptr));
  expected.push_back(compiler::parse_tree::assignment(
      7, {"f", compiler::parse_tree::variable_types::U8, 6}, nullptr));
  expected.push_back(compiler::parse_tree::assignment(
      8, {"g", compiler::parse_tree::variable_types::I8, 0}, nullptr));

  //  Lex the file(s)
  //
  std::string file = "test_files/parser_assignments.tl";
  compiler::lexer lexer;
  std::vector<compiler::TD_Pair> tokens;
  CHECK_TRUE(lexer.load_file(file));
  CHECK_TRUE(lexer.lex(tokens));

  //  Parse the Token Data pairs
  //
  constexpr auto import_file =
      [](std::string file) -> std::vector<compiler::TD_Pair> {
    //  Test file will not be importing
    //
    return {};
  };

  compiler::parser parser;
  std::vector<std::string> include_directories;
  std::vector<compiler::parse_tree::toplevel *> functions =
      parser.parse(file, include_directories, import_file, tokens);

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  compiler::parse_tree::function *f =
      static_cast<compiler::parse_tree::function *>(functions[0]);

  CHECK_EQUAL(expected.size(), f->element_list.size());

  for (size_t i = 0; i < expected.size(); i++) {
    compiler::parse_tree::assignment *a =
        static_cast<compiler::parse_tree::assignment *>(f->element_list[i]);
    CHECK_EQUAL(expected[i].line_number, a->line_number);
    CHECK_EQUAL(expected[i].var.name, a->var.name);
    CHECK_EQUAL(expected[i].var.depth, a->var.depth);
  }
}

TEST(parser_tests, expr) 
{
  std::vector<compiler::parse_tree::expression *> expected = 
  {
    new compiler::parse_tree::infix_expr("+", 
        new compiler::parse_tree::expression(
          compiler::parse_tree::node_type::RAW_NUMBER,
          "6"),
        new compiler::parse_tree::expression(
          compiler::parse_tree::node_type::RAW_NUMBER,
          "2")),
    new compiler::parse_tree::infix_expr("*", 
        new compiler::parse_tree::infix_expr("+",
          new compiler::parse_tree::expression(
            compiler::parse_tree::node_type::RAW_NUMBER,
            "4"),
          new compiler::parse_tree::expression(
            compiler::parse_tree::node_type::RAW_NUMBER,
            "2")),
        new compiler::parse_tree::expression(
          compiler::parse_tree::node_type::RAW_NUMBER,
          "3")),
    new compiler::parse_tree::infix_expr("+", 
        new compiler::parse_tree::expression(
          compiler::parse_tree::node_type::RAW_NUMBER,
          "3"),
        new compiler::parse_tree::function_call_expr(
          new compiler::parse_tree::expression(
            compiler::parse_tree::node_type::ID,
            "moot"))),
    new compiler::parse_tree::infix_expr("+", 
        new compiler::parse_tree::expression(
          compiler::parse_tree::node_type::RAW_NUMBER,
          "3"),
        new compiler::parse_tree::array_index_expr(
          new compiler::parse_tree::expression(
            compiler::parse_tree::node_type::ID,
            "x"),
          new compiler::parse_tree::expression(
            compiler::parse_tree::node_type::RAW_NUMBER,
            "0")))
  };


  //  Lex the file(s)
  //
  std::string file = "test_files/exprs.tl";
  compiler::lexer lexer;
  std::vector<compiler::TD_Pair> tokens;
  CHECK_TRUE(lexer.load_file(file));
  CHECK_TRUE(lexer.lex(tokens));

  //  Parse the Token Data pairs
  //
  constexpr auto import_file =
      [](std::string file) -> std::vector<compiler::TD_Pair> {
    //  Test file will not be importing
    //
    return {};
  };

  compiler::parser parser;
  std::vector<std::string> include_directories;
  std::vector<compiler::parse_tree::toplevel *> functions =
      parser.parse(file, include_directories, import_file, tokens);

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto func = reinterpret_cast<compiler::parse_tree::function*>(functions[0]);

  CHECK_EQUAL(expected.size(), func->element_list.size());

  for (size_t i = 0; i < expected.size(); i++) {
    
    auto assign = reinterpret_cast<compiler::parse_tree::assignment*>(func->element_list[i]);
    
    CHECK_EQUAL((int)expected[i]->type, (int)assign->expr->type);

    auto expected_infix_expr = reinterpret_cast<compiler::parse_tree::infix_expr*>(expected[i]);
    auto infix_expr = reinterpret_cast<compiler::parse_tree::infix_expr*>(assign->expr);

    CHECK_TRUE(exprs_are_equal(expected_infix_expr->left, infix_expr->left));
    CHECK_TRUE(exprs_are_equal(expected_infix_expr->right, infix_expr->right));

//    compiler::parse_tree::display_expr_tree("", assign->expr, false);
  }
}

