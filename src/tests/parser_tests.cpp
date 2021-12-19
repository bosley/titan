

#include "compiler/lexer.hpp"
#include "compiler/parser.hpp"
#include <iostream>
#include <vector>

#include <CppUTest/TestHarness.h>

namespace {
struct TestCase {
  compiler::parse_tree::toplevel::tl_type type;
  std::string name;
  std::vector<compiler::parse_tree::variable> parameters;
  std::vector<compiler::parse_tree::element *> element_list;
  compiler::parse_tree::variable_types return_type;
};
} // namespace

TEST_GROUP(parser_tests){};

//  Load the text files and ensure the expected tokens match the input
//
TEST(parser_tests, basic_function) {

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