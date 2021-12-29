

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

  compiler::parser parser;
  std::vector<std::string> include_directories;
  return parser.parse(file, include_directories, import_file, tokens);
}

bool exprs_are_equal(compiler::parse_tree::expr_ptr a,
                     compiler::parse_tree::expr_ptr b)
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
    auto a_infix_expr = std::reinterpret_pointer_cast<compiler::parse_tree::infix_expr>(a);
    auto b_infix_expr = std::reinterpret_pointer_cast<compiler::parse_tree::infix_expr>(b);

    if (!exprs_are_equal(a_infix_expr->left, b_infix_expr->left)) {
      return false;
    }
    if (!exprs_are_equal(a_infix_expr->right, b_infix_expr->right)) {
      return false;
    }
    return true;
  }
  case compiler::parse_tree::node_type::PREFIX: {
    auto a_prefix_expr =
        std::reinterpret_pointer_cast<compiler::parse_tree::prefix_expr>(a);
    auto b_prefix_expr =
        std::reinterpret_pointer_cast<compiler::parse_tree::prefix_expr>(b);
    if (!exprs_are_equal(a_prefix_expr->right, b_prefix_expr->right)) {
      return false;
    }
    break;
  }
  case compiler::parse_tree::node_type::ARRAY_IDX: {
    auto a_array_idx_expr =
        std::reinterpret_pointer_cast<compiler::parse_tree::array_index_expr>(a);
    auto b_array_idx_expr =
        std::reinterpret_pointer_cast<compiler::parse_tree::array_index_expr>(b);

    if (!exprs_are_equal(a_array_idx_expr->arr, b_array_idx_expr->arr)) {
      return false;
    }
    if (!exprs_are_equal(a_array_idx_expr->index, b_array_idx_expr->index)) {
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

TEST_GROUP(parser_tests){
    void setup(){AixLog::Log::init<AixLog::SinkCout>(AixLog::Severity::fatal);
}

void teardown() {}
}
;

//  Load the text files and ensure the expected tokens match the input
//
TEST(parser_tests, basic_function)
{

  struct TestCase {
    compiler::parse_tree::toplevel::tl_type type;
    std::string name;
    std::vector<compiler::parse_tree::variable> parameters;
    std::vector<compiler::parse_tree::element_ptr> element_list;
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

  auto functions = parse_file("test_files/parser_basic_function.tl");

  CHECK_EQUAL(tcs.size(), functions.size());

  for (size_t i = 0; i < tcs.size(); i++) {

    CHECK_EQUAL((int)tcs[i].type, (int)functions[i]->type);

    std::shared_ptr<compiler::parse_tree::function> f =
        std::static_pointer_cast<compiler::parse_tree::function>(functions[i]);

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

TEST(parser_tests, assignments)
{

  std::vector<compiler::parse_tree::assignment_statement> expected;

  expected.push_back(compiler::parse_tree::assignment_statement(
      5, {"d", compiler::parse_tree::variable_types::U32, 0}, nullptr));
  expected.push_back(compiler::parse_tree::assignment_statement(
      6, {"e", compiler::parse_tree::variable_types::U16, 12}, nullptr));
  expected.push_back(compiler::parse_tree::assignment_statement(
      7, {"f", compiler::parse_tree::variable_types::U8, 6}, nullptr));
  expected.push_back(compiler::parse_tree::assignment_statement(
      8, {"g", compiler::parse_tree::variable_types::I8, 0}, nullptr));

  auto functions = parse_file("test_files/parser_assignments.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  std::shared_ptr<compiler::parse_tree::function> f =
      std::static_pointer_cast<compiler::parse_tree::function>(functions[0]);

  CHECK_EQUAL(expected.size(), f->element_list.size());

  for (size_t i = 0; i < expected.size(); i++) {
    std::shared_ptr<compiler::parse_tree::assignment_statement> a =
        std::static_pointer_cast<compiler::parse_tree::assignment_statement>(f->element_list[i]);
    CHECK_EQUAL(expected[i].line_number, a->line_number);
    CHECK_EQUAL(expected[i].var.name, a->var.name);
    CHECK_EQUAL(expected[i].var.depth, a->var.depth);
  }
}

TEST(parser_tests, expr)
{
  std::vector<compiler::parse_tree::expr_ptr > expected = {
    compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          "+",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "6")),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "2")))),
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          "*",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
              "+",
              compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                  compiler::parse_tree::node_type::RAW_NUMBER, "4")),
              compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                  compiler::parse_tree::node_type::RAW_NUMBER, "2")))),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "3")))),
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          "+",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "3")),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::function_call_expr(
              compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                  compiler::parse_tree::node_type::ID, "moot")))))),
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          "+",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "3")),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::array_index_expr(
              compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                  compiler::parse_tree::node_type::ID, "x")),
              compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                  compiler::parse_tree::node_type::RAW_NUMBER, "0")))))) };

  auto functions = parse_file("test_files/exprs.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto func = std::reinterpret_pointer_cast<compiler::parse_tree::function>(functions[0]);

  CHECK_EQUAL(expected.size(), func->element_list.size());

  for (size_t i = 0; i < expected.size(); i++) {

    auto assign = std::reinterpret_pointer_cast<compiler::parse_tree::assignment_statement>(
        func->element_list[i]);

    CHECK_EQUAL((int)expected[i]->type, (int)assign->expr->type);

    auto expected_infix_expr =
        std::reinterpret_pointer_cast<compiler::parse_tree::infix_expr>(expected[i]);
    auto infix_expr =
        std::reinterpret_pointer_cast<compiler::parse_tree::infix_expr>(assign->expr);

    CHECK_TRUE(exprs_are_equal(expected_infix_expr->left, infix_expr->left));
    CHECK_TRUE(exprs_are_equal(expected_infix_expr->right, infix_expr->right));

    //    compiler::parse_tree::display_expr_tree("", assign->expr, false);
  }
}

TEST(parser_tests, if_statements)
{
  auto functions = parse_file("test_files/ifs.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto func = std::reinterpret_pointer_cast<compiler::parse_tree::function>(functions[0]);

  std::vector<size_t> expected_segments = {5, 3, 2, 1};

  // 4 Assignments, 4 if statements
  CHECK_EQUAL(8, func->element_list.size());

  for (size_t i = 4; i < func->element_list.size(); i++) {

    auto if_stmt = std::reinterpret_pointer_cast<compiler::parse_tree::if_statement>(
        func->element_list[i]);
    CHECK_EQUAL(expected_segments[i - 4], if_stmt->segments.size());

    if (i == 4) {
      auto expr =
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "0"));

      for (auto &e : if_stmt->segments[i].element_list) {
        auto a = std::reinterpret_pointer_cast<compiler::parse_tree::assignment_statement>(e);
        CHECK_TRUE(exprs_are_equal(expr, a->expr));
        CHECK_TRUE(("e" == a->var.name));
      }
    }
  }
}

TEST(parser_tests, while_statements)
{
  std::vector<compiler::parse_tree::while_statement_ptr> expected = {

    // While A
    compiler::parse_tree::while_statement_ptr(new compiler::parse_tree::while_statement(
          
          // Line
          3,

          // Expr
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "1")),
          
          // Body
          {

          // Inner While
          compiler::parse_tree::element_ptr(new compiler::parse_tree::while_statement(
              
              // Line
              4,

              // Expr
              compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                  compiler::parse_tree::node_type::RAW_NUMBER, "0")),

              // Body
              {
              }))
          })),

      // While B
      compiler::parse_tree::while_statement_ptr(new compiler::parse_tree::while_statement(

          // Line
          8,

          // Expr
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "1")),

          // Body
          {

          // Assignment 1
          compiler::parse_tree::element_ptr(new compiler::parse_tree::assignment_statement(

               // Line
               9,

               // Variable
               compiler::parse_tree::variable{
                   "a", compiler::parse_tree::variable_types::U8, 0},

               // Expression
               compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                   compiler::parse_tree::node_type::RAW_NUMBER, "2")))),

           // Assignment 2
           compiler::parse_tree::element_ptr(new compiler::parse_tree::assignment_statement(

               // Line
               10,

               // Variable
               compiler::parse_tree::variable{
                   "b", compiler::parse_tree::variable_types::U16, 0},

               // Expression
               compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                   compiler::parse_tree::node_type::RAW_NUMBER, "4")))),
           
           // Assignment 3
           compiler::parse_tree::element_ptr(new compiler::parse_tree::assignment_statement(

               // Line
               11,

               // variable
               compiler::parse_tree::variable{
                   "c", compiler::parse_tree::variable_types::U32, 0},

               // Expression
               compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                   compiler::parse_tree::node_type::RAW_NUMBER, "6")))),

           // Assignment 4
           compiler::parse_tree::element_ptr(new compiler::parse_tree::assignment_statement(

              // Line
               12,

               // Variable
               compiler::parse_tree::variable{
                   "d", compiler::parse_tree::variable_types::U64, 0},

               // Expression
               compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                   compiler::parse_tree::node_type::RAW_NUMBER, "8"))))
          }

        ))
  };

  auto functions = parse_file("test_files/while.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto func = std::reinterpret_pointer_cast<compiler::parse_tree::function>(functions[0]);

  CHECK_EQUAL(expected.size(), func->element_list.size());

  for (size_t i = 0; i < expected.size(); i++) {

    auto ws = std::reinterpret_pointer_cast<compiler::parse_tree::while_statement>(
        func->element_list[i]);
    CHECK_TRUE(exprs_are_equal(expected[i]->condition, ws->condition));

    CHECK_EQUAL(expected[i]->body.size(), ws->body.size());

    for (size_t j = 0; j < expected[i]->body.size(); j++) {

      if (ws->body[j]->line_number == 4) {

        auto expected_ws =
            std::reinterpret_pointer_cast<compiler::parse_tree::while_statement>(
                expected[i]->body[j]);
        auto inner_ws =
            std::reinterpret_pointer_cast<compiler::parse_tree::while_statement>(
                ws->body[j]);

        CHECK_EQUAL(expected_ws->body.size(), inner_ws->body.size());
        CHECK_TRUE(
            exprs_are_equal(expected_ws->condition, inner_ws->condition));
      }
      else {

        auto expected_a = std::reinterpret_pointer_cast<compiler::parse_tree::assignment_statement>(
            expected[i]->body[j]);
        auto inner_a =
            std::reinterpret_pointer_cast<compiler::parse_tree::assignment_statement>(ws->body[j]);

        CHECK_TRUE(expected_a->var.name == inner_a->var.name);
        CHECK_EQUAL((int)expected_a->var.type, (int)inner_a->var.type);
        CHECK_EQUAL(expected_a->var.depth, inner_a->var.depth);
        CHECK_TRUE(exprs_are_equal(expected_a->expr, inner_a->expr));
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
        std::reinterpret_pointer_cast<compiler::parse_tree::function>(functions[i]);
    CHECK_EQUAL(1, func->element_list.size());

    auto return_stmt =
        std::reinterpret_pointer_cast<compiler::parse_tree::return_statement>(
            func->element_list[0]);

    compiler::parse_tree::expr_ptr expected_expr = nullptr;
    if (i == 0) {
      expected_expr = compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
          compiler::parse_tree::node_type::RAW_NUMBER, "0"));
    }
    else if (i == 2) {
      expected_expr = compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
          compiler::parse_tree::node_type::RAW_NUMBER, "8"));
    }

    CHECK_TRUE(exprs_are_equal(expected_expr, return_stmt->expr));
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

  auto func = std::reinterpret_pointer_cast<compiler::parse_tree::function>(functions[1]);

  CHECK_EQUAL(1, func->element_list.size());

  auto expected =
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::function_call_expr(
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::ID, "new"))));

  auto expr_stmt =
      std::reinterpret_pointer_cast<compiler::parse_tree::expression_statement>(
          func->element_list[0]);

  CHECK_TRUE(exprs_are_equal(expected, expr_stmt->expr));
}

TEST(parser_tests, reassignment_statement)
{
  auto functions = parse_file("test_files/reassign.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto func = std::reinterpret_pointer_cast<compiler::parse_tree::function>(functions[0]);

  CHECK_EQUAL(2, func->element_list.size());

  auto reassign =
      std::reinterpret_pointer_cast<compiler::parse_tree::expression_statement>(
          func->element_list[1]);

  auto expected =
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          "=",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::ID, "x")),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
              "+",
              compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                  compiler::parse_tree::node_type::RAW_NUMBER, "22")),
              compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                  compiler::parse_tree::node_type::ID, "x"))))));

  CHECK_TRUE(exprs_are_equal(expected, reassign->expr));
}

TEST(parser_tests, for_statement)
{
  auto functions = parse_file("test_files/for.tl");

  CHECK_EQUAL(1, functions.size());
  CHECK_EQUAL((int)compiler::parse_tree::toplevel::tl_type::FUNCTION,
              (int)functions[0]->type);

  auto func = std::reinterpret_pointer_cast<compiler::parse_tree::function>(functions[0]);

  CHECK_EQUAL(1, func->element_list.size());

  auto expected_assign =
      
      compiler::parse_tree::assignment_statement_ptr(new compiler::parse_tree::assignment_statement(
          5, {"i", compiler::parse_tree::variable_types::U8, 0},
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "0"))));

  auto expected_condition =
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          "<",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::ID, "i")),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::RAW_NUMBER, "10"))));

  auto expected_modifier =
      compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
          "=",
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
              compiler::parse_tree::node_type::ID, "i")),
          compiler::parse_tree::expr_ptr(new compiler::parse_tree::infix_expr(
              "+",
              compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                  compiler::parse_tree::node_type::ID, "i")),
              compiler::parse_tree::expr_ptr(new compiler::parse_tree::expression(
                  compiler::parse_tree::node_type::RAW_NUMBER, "1"))))));

  auto actual = std::reinterpret_pointer_cast<compiler::parse_tree::for_statement>(
      func->element_list[0]);

  auto actual_assign =
      std::reinterpret_pointer_cast<compiler::parse_tree::assignment_statement>(actual->assign);

  CHECK_TRUE(exprs_are_equal(expected_assign->expr, actual_assign->expr));
  CHECK_TRUE(exprs_are_equal(expected_condition, actual->condition));
  CHECK_TRUE(exprs_are_equal(expected_modifier, actual->modifier));
}

