#include "compiler/parsetree.hpp"
#include "compiler/symbols.hpp"

#include <random>
#include <vector>

#include <CppUTest/TestHarness.h>

TEST_GROUP(symbol_tests){};

TEST(symbol_tests, all)
{
  compiler::symbol::table table;

  std::string function_main = "main";
  std::string function_add = "add";
  std::string function_print = "print";

  compiler::parse_tree::function *dummy_func =
      new compiler::parse_tree::function(0, 0);
  compiler::parse_tree::assignment_statement *dummy_assign =
      new compiler::parse_tree::assignment_statement(0, {}, nullptr);

  // Will create symbols in global table for functions,
  // and will generate global sub scopes for each function
  CHECK_TRUE(table.add_symbol(function_main, dummy_func));
  CHECK_TRUE(table.add_symbol(function_add, dummy_func));
  CHECK_TRUE(table.add_symbol(function_print, dummy_func));

  // Enter function
  CHECK_TRUE(table.activate_top_level_scope(function_main));

  // Add some vars to the thing
  CHECK_TRUE(table.add_symbol("x", dummy_assign));
  CHECK_TRUE(table.add_symbol("y", dummy_assign));
  CHECK_TRUE(table.add_symbol("z", dummy_assign));

  CHECK_TRUE(table.exists("x"));
  CHECK_TRUE(table.exists("y"));
  CHECK_TRUE(table.exists("z"));

  // Add another scope
  table.add_scope_and_enter("if_statement");

  // Add some more vars
  CHECK_TRUE(table.add_symbol("a", dummy_assign));
  CHECK_TRUE(table.add_symbol("b", dummy_assign));
  CHECK_TRUE(table.add_symbol("c", dummy_assign));

  CHECK_TRUE(table.exists("x"));
  CHECK_TRUE(table.exists("y"));
  CHECK_TRUE(table.exists("z"));

  CHECK_TRUE(table.exists("a"));
  CHECK_TRUE(table.exists("b"));
  CHECK_TRUE(table.exists("c"));

  // Ensure local scan works properly
  CHECK_TRUE(table.exists("a", true));
  CHECK_TRUE(table.exists("b", true));
  CHECK_TRUE(table.exists("c", true));

  // Ensure that the previous aren't located in the current scope
  CHECK_FALSE(table.exists("x", true));
  CHECK_FALSE(table.exists("y", true));
  CHECK_FALSE(table.exists("z", true));

  // Ensure functions reachable from inner
  CHECK_TRUE(table.exists("main"));
  CHECK_TRUE(table.exists("add"));
  CHECK_TRUE(table.exists("print"));

  // Leave the scope
  table.pop_scope();

  // Ensure the first ones still reachable and the later
  // can no longer be reached
  CHECK_TRUE(table.exists("x"));
  CHECK_TRUE(table.exists("y"));
  CHECK_TRUE(table.exists("z"));
  CHECK_FALSE(table.exists("a"));
  CHECK_FALSE(table.exists("b"));
  CHECK_FALSE(table.exists("c"));

  // Ensure functions reachable
  CHECK_TRUE(table.exists("main"));
  CHECK_TRUE(table.exists("add"));
  CHECK_TRUE(table.exists("print"));

  delete dummy_func;
  delete dummy_assign;
}

TEST(symbol_tests, expanded)
{
  compiler::parse_tree::function *dummy_func =
      new compiler::parse_tree::function(0,0);
  compiler::parse_tree::assignment_statement *dummy_assign =
      new compiler::parse_tree::assignment_statement(0, {}, nullptr);

  compiler::symbol::table table;

  constexpr auto get_rand = [](uint64_t min, uint64_t max) -> uint64_t {
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<uint64_t> dist(min, max);
    return dist(eng);
  };

  uint64_t num_functions = get_rand(10, 50);

  // Add functions
  //
  for (uint64_t i = 0; i < num_functions; i++) {
    std::string fn_name = "function_" + std::to_string(i);
    CHECK_TRUE(table.add_symbol(fn_name, dummy_func));
  }

  // Operate on them
  //
  for (uint64_t i = 0; i < num_functions; i++) {
    std::string fn_name = "function_" + std::to_string(i);

    // Activate function
    //
    CHECK_TRUE(table.activate_top_level_scope(fn_name));

    // Ensure that all other functions are reachable from it
    //
    for (uint64_t j = 0; j < num_functions; j++) {

      std::string search_fn = "function_" + std::to_string(j);
      CHECK_TRUE(table.exists(search_fn));
    }

    uint64_t top_level_vars = get_rand(1, 100);

    for (uint64_t j = 0; j < top_level_vars; j++) {
      std::string var_name = "var_" + std::to_string(j);
      CHECK_TRUE(table.add_symbol(var_name, dummy_assign));
    }

    // Ensure reachable
    for (uint64_t j = 0; j < top_level_vars; j++) {
      std::string var_name = "var_" + std::to_string(j);
      CHECK_TRUE(table.exists(var_name));
    }

    // Go deep into scope
    uint64_t scope_depth = get_rand(1, 5);
    for (uint64_t k = 0; k < scope_depth; k++) {
      std::string scope_name = "scope_" + std::to_string(k);
      table.add_scope_and_enter(scope_name);
    }

    // Ensure still reachable
    for (uint64_t j = 0; j < top_level_vars; j++) {
      std::string var_name = "var_" + std::to_string(j);
      CHECK_TRUE(table.exists(var_name));

      if (std::nullopt == table.lookup(var_name)) {
        FAIL("Failed to get variable");
      }
    }

    // Add vars deep in scopes
    for (uint64_t j = 0; j < top_level_vars; j++) {
      std::string var_name = "scoped_var_" + std::to_string(j);
      CHECK_TRUE(table.add_symbol(var_name, dummy_assign));
    }

    // Sanity check
    for (uint64_t j = 0; j < top_level_vars; j++) {
      std::string var_name = "scoped_var_" + std::to_string(j);
      CHECK_TRUE(table.exists(var_name));

      if (std::nullopt == table.lookup(var_name)) {
        FAIL("Failed to get variable");
      }

      // Ensure that top level items are not in current scope, but
      // in an upper scope
      std::string top_var_name = "var_" + std::to_string(j);
      CHECK_FALSE(table.exists(top_var_name, true));

      if (std::nullopt != table.lookup(top_var_name, true)) {
        FAIL("Failed to _not_ get variable");
      }
    }

    // Leave scopes
    for (uint64_t k = 0; k < scope_depth; k++) {
      table.pop_scope();
    }

    for (uint64_t j = 0; j < top_level_vars; j++) {

      // Ensure scoped vars are gone
      std::string scoped_var_name = "scoped_var_" + std::to_string(j);
      CHECK_FALSE(table.exists(scoped_var_name));

      // Check top level values
      std::string var_name = "var_" + std::to_string(j);
      auto var_item = table.lookup(var_name);
      if (std::nullopt == var_item) {
        FAIL("Failed to get variable");
      }
    }
  }

  delete dummy_func;
  delete dummy_assign;
}
