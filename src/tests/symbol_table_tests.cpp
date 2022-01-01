#include "compiler/symbols.hpp"
#include "compiler/parsetree.hpp"

#include <iostream>
#include <vector>

#include <CppUTest/TestHarness.h>

TEST_GROUP(symbol_tests){};

TEST(symbol_tests, all)
{
  compiler::symbol::table table;

  std::string function_main  = "main";
  std::string function_add   = "add";
  std::string function_print = "print";

  compiler::parse_tree::function *dummy_func =
      new compiler::parse_tree::function();
  compiler::parse_tree::assignment_statement *dummy_assign =
      new compiler::parse_tree::assignment_statement(0, {}, nullptr);


  // Will create symbols in global table for functions,
  // and will generate global sub scopes for each function
  CHECK_TRUE(table.add_symbol(function_main,  dummy_func));
  CHECK_TRUE(table.add_symbol(function_add,   dummy_func));
  CHECK_TRUE(table.add_symbol(function_print, dummy_func));

  // Enter function
  CHECK_TRUE(table.activate_top_level_scope(function_main));

  // Add some vars to the thing

  // Add another scope
  
  // Add some more vars 

  // Ensure first added vars reachable along with the new ones

  // Leave the scope 

  // Ensure the first ones still reachable and the later 
  // can no longer be reached

  // Attempt shenanigans

  // Ensure shenanigans aren't allowed

  // ??? 

  // Profit

  delete dummy_func;
  delete dummy_assign;
}
