

#include "compiler/imports.hpp"
#include "compiler/lexer.hpp"
#include "compiler/parser.hpp"
#include "log/log.hpp"
#include <iostream>
#include <vector>

#include <CppUTest/TestHarness.h>

namespace {

std::vector<compiler::parse_tree::toplevel_ptr> parse_file(std::string file)
{
  compiler::lexer lexer;
  std::vector<compiler::TD_Pair> tokens;
  if (!lexer.load_file(file)) {
    std::cerr << "Lexer failed to load : " << file << std::endl;
    return {};
  }
  if (!lexer.lex(tokens)) {
    std::cerr << "Lexer failed to lex : " << file << std::endl;
    return {};
  }

  constexpr auto import_file =
      [](std::string file) -> std::vector<compiler::TD_Pair> {
    //  Test files will not be importing
    return {};
  };

  compiler::imports file_imports;
  compiler::parser parser(file_imports);
  std::vector<std::string> include_directories;
  return parser.parse(file, include_directories, import_file, tokens);
}

bool exprs_are_equal(compiler::parse_tree::expression *a,
                     compiler::parse_tree::expression *b)
{
  if (a == nullptr && b == nullptr) {
    return true;
  }

  if (!a || !b) {
    return false;
  }

  if (a->type != b->type) {
    std::cout << "Expression type mismatch" << std::endl;
    return false;
  }

  switch (a->type) {
  case compiler::parse_tree::node_type::ID:
  case compiler::parse_tree::node_type::RAW_FLOAT:
  case compiler::parse_tree::node_type::RAW_NUMBER:
  case compiler::parse_tree::node_type::RAW_STRING:
  case compiler::parse_tree::node_type::CALL:
    if (a->value != b->value) {
      std::cout << "Expression value mismatch (" << a->value
                << " != " << b->value << ")" << std::endl;
      return false;
    }
    return true;

  case compiler::parse_tree::node_type::INFIX: {
    auto a_infix_expr = reinterpret_cast<compiler::parse_tree::infix_expr *>(a);
    auto b_infix_expr = reinterpret_cast<compiler::parse_tree::infix_expr *>(b);

    if (!exprs_are_equal(a_infix_expr->left.get(), b_infix_expr->left.get())) {
      return false;
    }
    if (!exprs_are_equal(a_infix_expr->right.get(),
                         b_infix_expr->right.get())) {
      return false;
    }
    return true;
  }
  case compiler::parse_tree::node_type::PREFIX: {
    auto a_prefix_expr =
        reinterpret_cast<compiler::parse_tree::prefix_expr *>(a);
    auto b_prefix_expr =
        reinterpret_cast<compiler::parse_tree::prefix_expr *>(b);
    if (!exprs_are_equal(a_prefix_expr->right.get(),
                         b_prefix_expr->right.get())) {
      return false;
    }
    break;
  }
  case compiler::parse_tree::node_type::ARRAY_IDX: {
    auto a_array_idx_expr =
        reinterpret_cast<compiler::parse_tree::array_index_expr *>(a);
    auto b_array_idx_expr =
        reinterpret_cast<compiler::parse_tree::array_index_expr *>(b);

    if (!exprs_are_equal(a_array_idx_expr->arr.get(),
                         b_array_idx_expr->arr.get())) {
      return false;
    }
    if (!exprs_are_equal(a_array_idx_expr->index.get(),
                         b_array_idx_expr->index.get())) {
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

} // namespace

TEST_GROUP(parser_tests){};

//  Load the text files and ensure the expected tokens match the input
//
TEST(parser_tests, basic_function)
{
  struct TestCase {
    compiler::parse_tree::toplevel::tl_type type;
    std::string name;
    std::vector<compiler::parse_tree::variable> parameters;
    compiler::parse_tree::variable_types return_type;
  };

  std::vector<TestCase> tcs = {
      {compiler::parse_tree::toplevel::tl_type::FUNCTION,
       "main",
       {
           {"argc", compiler::parse_tree::variable_types::U8, "u8", 0},
           {"argv", compiler::parse_tree::variable_types::STRING, "str", 1},
       },
       compiler::parse_tree::variable_types::I64},
      {compiler::parse_tree::toplevel::tl_type::FUNCTION,
       "test",
       {},
       compiler::parse_tree::variable_types::U16},
      {compiler::parse_tree::toplevel::tl_type::FUNCTION,
       "test1",
       {
           {"a", compiler::parse_tree::variable_types::U32, "u32", 10},
           {"b", compiler::parse_tree::variable_types::I8, "i8", 20},
           {"c", compiler::parse_tree::variable_types::STRING, "str", 0},

       },
       compiler::parse_tree::variable_types::I16},
  };

  auto functions = parse_file("test_files/parser_basic_function.tl");

  CHECK_EQUAL(tcs.size(), functions.size());

  for (size_t i = 0; i < tcs.size(); i++) {

    CHECK_EQUAL((int)tcs[i].type, (int)functions[i]->type);

    auto f = static_cast<compiler::parse_tree::function *>(functions[i].get());

    CHECK_EQUAL(tcs[i].name, f->name);
    CHECK_EQUAL((int)tcs[i].return_type, (int)f->return_data.type);
    CHECK_EQUAL(tcs[i].parameters.size(), f->parameters.size());
    CHECK_EQUAL(0, f->element_list.size());

    for (size_t p = 0; p < tcs[i].parameters.size(); p++) {

      CHECK_EQUAL(tcs[i].parameters[p].name, f->parameters[p].name);
      CHECK_EQUAL((int)tcs[i].parameters[p].type, (int)f->parameters[p].type);
      CHECK_EQUAL(tcs[i].parameters[p].depth, f->parameters[p].depth);
    }
  }
}

TEST(parser_tests, assignments)
{

  std::vector<compiler::parse_tree::assignment_statement> expected;

  expected.push_back(compiler::parse_tree::assignment_statement(
      5, 0, {"d", compiler::parse_tree::variable_types::U32, "u32", 0},
      nullptr));
  expected.push_back(compiler::parse_tree::assignment_statement(
      6, 0, {"e", compiler::parse_tree::variable_types::U16, "u16", 12},
      nullptr));
  expected.push_back(compiler::parse_tree::assignment_statement(
      7, 0, {"f", compiler::parse_tree::variable_types::U8, "u8", 6}, nullptr));
  expected.push_back(compiler::parse_tree::assignment_statement(
      8, 0, {"g", compiler::parse_tree::variable_types::I8, "i8", 0}, nullptr));

  auto functions = parse_file("test_files/parser_assignments.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto f = static_cast<compiler::parse_tree::function *>(functions[0].get());

  CHECK_EQUAL(expected.size(), f->element_list.size());

  for (size_t i = 0; i < expected.size(); i++) {
    auto a = static_cast<compiler::parse_tree::assignment_statement *>(
        f->element_list[i].get());
    CHECK_EQUAL(expected[i].line, a->line);
    CHECK_EQUAL(expected[i].var.name, a->var.name);
    CHECK_EQUAL(expected[i].var.depth, a->var.depth);
  }
}

TEST(parser_tests, expr)
{
  std::vector<compiler::parse_tree::expr_ptr> expected;

  expected.emplace_back(
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          0, 0, "+",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "6")),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "2")))));

  expected.emplace_back(
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          0, 0, "*",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
              0, 0, "+",
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "4")),
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "2")))),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "3")))));

  expected.emplace_back(
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          0, 0, "+",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "3")),
          compiler::parse_tree::expr_ptr(
              new compiler::parse_tree::function_call_expr(
                  0, 0,
                  compiler::parse_tree::expr_ptr(
                      new compiler::parse_tree::expression(
                          compiler::parse_tree::node_type::ID, "moot")))))));

  expected.emplace_back(
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          0, 0, "+",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "3")),
          compiler::parse_tree::expr_ptr(
              new compiler::parse_tree::array_index_expr(
                  0, 0,
                  compiler::parse_tree::expr_ptr(
                      new compiler::parse_tree::expression(
                          compiler::parse_tree::node_type::ID, "x")),
                  compiler::parse_tree::expr_ptr(
                      new compiler::parse_tree::expression(
                          compiler::parse_tree::node_type::RAW_NUMBER,
                          "0")))))));

  expected.emplace_back(
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          0, 0, "+",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "3")),
          compiler::parse_tree::expr_ptr(
              new compiler::parse_tree::function_call_expr(
                  0, 0,
                  compiler::parse_tree::expr_ptr(
                      new compiler::parse_tree::expression(
                          compiler::parse_tree::node_type::ID, "add")))))));

  auto functions = parse_file("test_files/exprs.tl");

  CHECK_EQUAL(2, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[1]->type);

  auto func =
      reinterpret_cast<compiler::parse_tree::function *>(functions[1].get());

  CHECK_EQUAL(expected.size(), func->element_list.size());

  for (size_t i = 0; i < expected.size(); i++) {

    auto assign =
        reinterpret_cast<compiler::parse_tree::assignment_statement *>(
            func->element_list[i].get());

    CHECK_EQUAL((int)expected[i]->type, (int)assign->expr->type);

    auto expected_infix_expr =
        reinterpret_cast<compiler::parse_tree::infix_expr *>(expected[i].get());
    auto infix_expr = reinterpret_cast<compiler::parse_tree::infix_expr *>(
        assign->expr.get());

    CHECK_TRUE(exprs_are_equal(expected_infix_expr->left.get(),
                               infix_expr->left.get()));
    CHECK_TRUE(exprs_are_equal(expected_infix_expr->right.get(),
                               infix_expr->right.get()));

    //    compiler::parse_tree::display_expr_tree("", assign->expr, false);
  }
}

TEST(parser_tests, if_statements)
{
  auto functions = parse_file("test_files/ifs.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto func =
      reinterpret_cast<compiler::parse_tree::function *>(functions[0].get());

  std::vector<size_t> expected_segments = {5, 3, 2, 1};

  // 4 Assignments, 4 if statements
  CHECK_EQUAL(8, func->element_list.size());

  for (size_t i = 4; i < func->element_list.size(); i++) {

    auto if_stmt = reinterpret_cast<compiler::parse_tree::if_statement *>(
        func->element_list[i].get());
    CHECK_EQUAL(expected_segments[i - 4], if_stmt->segments.size());

    if (i == 4) {
      auto expr =
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "0"));

      for (auto &e : if_stmt->segments[i].element_list) {
        auto a = reinterpret_cast<compiler::parse_tree::assignment_statement *>(
            e.get());
        CHECK_TRUE(exprs_are_equal(expr.get(), a->expr.get()));
        CHECK_TRUE(("e" == a->var.name));
      }
    }
  }
}

TEST(parser_tests, while_statements)
{
  std::vector<compiler::parse_tree::while_statement_ptr> expected;

  std::vector<compiler::parse_tree::element_ptr> while_a_body;

  while_a_body.emplace_back(

      // Inner While
      compiler::parse_tree::element_ptr(
          new compiler::parse_tree::while_statement(

              // Line
              4, 0,

              // Expr
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "0")),

              // Body
              {})));

  expected.emplace_back(
      // While A
      compiler::parse_tree::while_statement_ptr(
          new compiler::parse_tree::while_statement(

              // Line
              3, 0,

              // Expr
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "1")),

              // Body
              std::move(while_a_body)

                  )));

  std::vector<compiler::parse_tree::element_ptr> while_b_body;

  while_b_body.emplace_back(
      // Assignment 1
      compiler::parse_tree::element_ptr(
          new compiler::parse_tree::assignment_statement(

              // Line
              9, 0,

              // Variable
              compiler::parse_tree::variable{
                  "a", compiler::parse_tree::variable_types::U8, "u8", 0},

              // Expression
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "2")))));

  while_b_body.emplace_back(
      // Assignment 2
      compiler::parse_tree::element_ptr(
          new compiler::parse_tree::assignment_statement(

              // Line
              10, 0,

              // Variable
              compiler::parse_tree::variable{
                  "b", compiler::parse_tree::variable_types::U16, "u16", 0},

              // Expression
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "4")))));

  while_b_body.emplace_back(
      // Assignment 3
      compiler::parse_tree::element_ptr(
          new compiler::parse_tree::assignment_statement(

              // Line
              11, 0,

              // variable
              compiler::parse_tree::variable{
                  "c", compiler::parse_tree::variable_types::U32, "u32", 0},

              // Expression
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "6")))));

  while_b_body.emplace_back(
      // Assignment 4
      compiler::parse_tree::element_ptr(
          new compiler::parse_tree::assignment_statement(

              // Line
              12, 0,

              // Variable
              compiler::parse_tree::variable{
                  "d", compiler::parse_tree::variable_types::U64, "u64", 0},

              // Expression
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "8")))));

  expected.emplace_back(
      // While B
      compiler::parse_tree::while_statement_ptr(
          new compiler::parse_tree::while_statement(

              // Line
              8, 0,

              // Expr
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "1")),

              // Body
              std::move(while_b_body))));

  auto functions = parse_file("test_files/while.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto func =
      reinterpret_cast<compiler::parse_tree::function *>(functions[0].get());

  CHECK_EQUAL(expected.size(), func->element_list.size());

  for (size_t i = 0; i < expected.size(); i++) {

    auto ws = reinterpret_cast<compiler::parse_tree::while_statement *>(
        func->element_list[i].get());
    CHECK_TRUE(
        exprs_are_equal(expected[i]->condition.get(), ws->condition.get()));

    CHECK_EQUAL(expected[i]->body.size(), ws->body.size());

    for (size_t j = 0; j < expected[i]->body.size(); j++) {

      if (ws->body[j]->line == 4) {

        auto expected_ws =
            reinterpret_cast<compiler::parse_tree::while_statement *>(
                expected[i]->body[j].get());
        auto inner_ws =
            reinterpret_cast<compiler::parse_tree::while_statement *>(
                ws->body[j].get());

        CHECK_EQUAL(expected_ws->body.size(), inner_ws->body.size());
        CHECK_TRUE(exprs_are_equal(expected_ws->condition.get(),
                                   inner_ws->condition.get()));
      }
      else {

        auto expected_a =
            reinterpret_cast<compiler::parse_tree::assignment_statement *>(
                expected[i]->body[j].get());
        auto inner_a =
            reinterpret_cast<compiler::parse_tree::assignment_statement *>(
                ws->body[j].get());

        CHECK_TRUE(expected_a->var.name == inner_a->var.name);
        CHECK_EQUAL((int)expected_a->var.type, (int)inner_a->var.type);
        CHECK_EQUAL(expected_a->var.depth, inner_a->var.depth);
        CHECK_TRUE(
            exprs_are_equal(expected_a->expr.get(), inner_a->expr.get()));
      }
    }
  }
}

TEST(parser_tests, return_tests)
{
  auto functions = parse_file("test_files/return.tl");
  CHECK_EQUAL(3, functions.size());

  for (size_t i = 0; i < 3; i++) {
    CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
                (int)functions[i]->type);

    auto func =
        reinterpret_cast<compiler::parse_tree::function *>(functions[i].get());
    CHECK_EQUAL(1, func->element_list.size());

    auto return_stmt =
        reinterpret_cast<compiler::parse_tree::return_statement *>(
            func->element_list[0].get());

    compiler::parse_tree::expr_ptr expected_expr = nullptr;
    if (i == 0) {
      expected_expr =
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "0"));
    }
    else if (i == 2) {
      expected_expr =
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "8"));
    }

    CHECK_TRUE(exprs_are_equal(expected_expr.get(), return_stmt->expr.get()));
  }
}

TEST(parser_tests, expression_statement)
{
  auto functions = parse_file("test_files/expr_stmt.tl");

  CHECK_EQUAL(2, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[1]->type);

  auto func =
      reinterpret_cast<compiler::parse_tree::function *>(functions[1].get());

  CHECK_EQUAL(1, func->element_list.size());

  auto expected = compiler::parse_tree::expr_ptr(
      new compiler::parse_tree::function_call_expr(
          0, 0,
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::ID, "new"))));

  auto expr_stmt =
      reinterpret_cast<compiler::parse_tree::expression_statement *>(
          func->element_list[0].get());

  CHECK_TRUE(exprs_are_equal(expected.get(), expr_stmt->expr.get()));
}

TEST(parser_tests, reassignment_statement)
{
  auto functions = parse_file("test_files/reassign.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto func =
      reinterpret_cast<compiler::parse_tree::function *>(functions[0].get());

  CHECK_EQUAL(2, func->element_list.size());

  auto reassign =
      reinterpret_cast<compiler::parse_tree::expression_statement *>(
          func->element_list[1].get());

  auto expected =
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          0, 0, "=",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::ID, "x")),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
              0, 0, "+",
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "22")),
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::ID, "x"))))));

  CHECK_TRUE(exprs_are_equal(expected.get(), reassign->expr.get()));
}

TEST(parser_tests, for_statement)
{
  auto functions = parse_file("test_files/for.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto func =
      reinterpret_cast<compiler::parse_tree::function *>(functions[0].get());

  CHECK_EQUAL(1, func->element_list.size());

  auto expected_assign = compiler::parse_tree::assignment_statement_ptr(
      new compiler::parse_tree::assignment_statement(
          5, 0, {"i", compiler::parse_tree::variable_types::U8, "u8", 0},
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "0"))));

  auto expected_condition =
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          0, 0, "<",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::ID, "i")),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "10"))));

  auto expected_modifier =
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          0, 0, "=",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::ID, "i")),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
              0, 0, "+",
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::ID, "i")),
              compiler::parse_tree::expr_ptr(
                  new compiler::parse_tree::expression(
                      compiler::parse_tree::node_type::RAW_NUMBER, "1"))))));

  auto actual = reinterpret_cast<compiler::parse_tree::for_statement *>(
      func->element_list[0].get());

  auto actual_assign =
      reinterpret_cast<compiler::parse_tree::assignment_statement *>(
          actual->assign.get());

  CHECK_TRUE(
      exprs_are_equal(expected_assign->expr.get(), actual_assign->expr.get()));
  CHECK_TRUE(
      exprs_are_equal(expected_condition.get(), actual->condition.get()));
  CHECK_TRUE(exprs_are_equal(expected_modifier.get(), actual->modifier.get()));
}

TEST(parser_tests, import_statement)
{
  //  Import needs to handle importing all by its lonesome

  std::string file = "test_files/import.tl";

  compiler::lexer lexer;
  std::vector<compiler::TD_Pair> tokens;
  if (!lexer.load_file(file)) {
    FAIL("Failed to load file");
  }
  if (!lexer.lex(tokens)) {
    FAIL("Failed to lex file");
  }

  constexpr auto import_file =
      [](std::string file) -> std::vector<compiler::TD_Pair> {
    compiler::lexer lexer;
    if (!lexer.load_file(file)) {
      return {};
    }

    std::vector<compiler::TD_Pair> tokens;
    if (!lexer.lex(tokens)) {
      return {};
    }

    return tokens;
  };

  compiler::imports file_imports;
  compiler::parser parser(file_imports);
  std::vector<std::string> include_directories = {"test_files"};

  auto functions = parser.parse(file, include_directories, import_file, tokens);

  CHECK_EQUAL(2, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);
}
