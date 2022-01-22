#include "exec/space.hpp"
#include "exec/memory.hpp"
#include "lang/instructions.hpp"

#include "generator.hpp"

#include <CppUTest/TestHarness.h>

namespace
{
  bool vars_equal(titan::object* lhs,
                  titan::object* rhs)
  {
    if(!lhs || !rhs) { return false; }
    if(lhs->type != rhs->type) { return false; }

    switch(lhs->type) {
    case titan::obj_type::U8: 
    {
      auto l = reinterpret_cast<titan::object_u8*>(lhs);
      auto r = reinterpret_cast<titan::object_u8*>(rhs);
      return l->value == r->value;
    }
    case titan::obj_type::U16:
    {
      auto l = reinterpret_cast<titan::object_u16*>(lhs);
      auto r = reinterpret_cast<titan::object_u16*>(rhs);
      return l->value == r->value;
    }
    case titan::obj_type::U32:   
    {
      auto l = reinterpret_cast<titan::object_u32*>(lhs);
      auto r = reinterpret_cast<titan::object_u32*>(rhs);
      return l->value == r->value;
    }
    case titan::obj_type::U64:   
    {
      auto l = reinterpret_cast<titan::object_u64*>(lhs);
      auto r = reinterpret_cast<titan::object_u64*>(rhs);
      return l->value == r->value;
    }
    case titan::obj_type::I8:
    {
      auto l = reinterpret_cast<titan::object_i8*>(lhs);
      auto r = reinterpret_cast<titan::object_i8*>(rhs);
      return l->value == r->value;
    }
    case titan::obj_type::I16: 
    {
      auto l = reinterpret_cast<titan::object_i16*>(lhs);
      auto r = reinterpret_cast<titan::object_i16*>(rhs);
      return l->value == r->value;
    }
    case titan::obj_type::I32: 
    {
      auto l = reinterpret_cast<titan::object_i32*>(lhs);
      auto r = reinterpret_cast<titan::object_i32*>(rhs);
      return l->value == r->value;
    }
    case titan::obj_type::I64: 
    {
      auto l = reinterpret_cast<titan::object_i64*>(lhs);
      auto r = reinterpret_cast<titan::object_i64*>(rhs);
      return l->value == r->value;
    }
    case titan::obj_type::FLOAT:
    {
      auto l = reinterpret_cast<titan::object_float*>(lhs);
      auto r = reinterpret_cast<titan::object_float*>(rhs);
      return l->value == r->value;
    }
    default: return false;
    }


    return true;
  }
}

TEST_GROUP(exec_memory_tests){};

TEST(exec_memory_tests, space)
{
  // Place within a scope so the smart pointers free
  // and dont cause ASAN and CPPUTest to think there is a leak
  {
    titan::space s;
    s.push_top_level_scope();

    auto x = gen::random_built_in_variable("x");
    auto y = gen::random_built_in_variable("y");

    CHECK_TRUE(s.new_var("x", x));
    CHECK_TRUE(s.new_var("y", y));

    {
      auto get_x = s.get_variable("x");
      auto get_y = s.get_variable("y");
      CHECK_TRUE(get_x != nullptr);
      CHECK_TRUE(get_y != nullptr);
      CHECK_TRUE(vars_equal(x, get_x));
      CHECK_TRUE(vars_equal(y, get_y));
    }

    s.sub_scope();

    {
      auto get_x = s.get_variable("x");
      auto get_y = s.get_variable("y");
      CHECK_TRUE(get_x != nullptr);
      CHECK_TRUE(get_y != nullptr);
      CHECK_TRUE(vars_equal(x, get_x));
      CHECK_TRUE(vars_equal(y, get_y));

      auto a = gen::random_built_in_variable("a");
      auto b = gen::random_built_in_variable("b");

      CHECK_TRUE(s.new_var("a", a));
      CHECK_TRUE(s.new_var("b", b));

      auto get_a = s.get_variable("a");
      auto get_b = s.get_variable("b");
      CHECK_TRUE(get_a != nullptr);
      CHECK_TRUE(get_b != nullptr);
      CHECK_TRUE(vars_equal(a, get_a));
      CHECK_TRUE(vars_equal(b, get_b));
    }

    s.sub_scope();

    {
      auto get_x = s.get_variable("x");
      auto get_y = s.get_variable("y");
      CHECK_TRUE(get_x != nullptr);
      CHECK_TRUE(get_y != nullptr);
      CHECK_TRUE(vars_equal(x, get_x));
      CHECK_TRUE(vars_equal(y, get_y));

      auto get_a = s.get_variable("a");
      auto get_b = s.get_variable("b");
      CHECK_TRUE(get_a != nullptr);
      CHECK_TRUE(get_b != nullptr);
    }

    s.leave_scope();

    s.leave_scope();

    {
      // Should no longer be reachable
      auto get_a = s.get_variable("a");
      auto get_b = s.get_variable("b");
      CHECK_TRUE(nullptr == get_a);
      CHECK_TRUE(nullptr == get_b);
    }

    {
      auto get_x = s.get_variable("x");
      auto get_y = s.get_variable("y");
      CHECK_TRUE(get_x != nullptr);
      CHECK_TRUE(get_y != nullptr);
    }

    s.delete_var("x");

    {
      auto get_x = s.get_variable("x");
      CHECK_TRUE(get_x == nullptr);
    }

    s.pop_scope();
  }
}

TEST(exec_memory_tests, memory)
{
  titan::memory m;
  CHECK_TRUE(m.new_space("delta_quadrant"));
  CHECK_TRUE(m.associate_space_with_name("delta_quadrant", "space::delta"));
  CHECK_TRUE(m.associate_space_with_name("delta_quadrant", "modules::space::delta"));

  auto a = gen::random_built_in_variable("a");
  auto b = gen::random_built_in_variable("b");
  CHECK_TRUE(m.new_variable("delta_quadrant", "a", a));
  CHECK_TRUE(m.new_variable("space::delta", "b", b));

  {
    auto get_a = m.get_variable("space::delta", "a");
    auto get_b = m.get_variable("modules::space::delta", "b");
    CHECK_TRUE(nullptr != get_a);
    CHECK_TRUE(nullptr != get_b);
    CHECK_TRUE(vars_equal(a, get_a));
    CHECK_TRUE(vars_equal(b, get_b));
  }

  {
    auto get_a = m.get_variable("delta_quadrant", "a");
    auto get_b = m.get_variable("modules::space::delta", "b");
    CHECK_TRUE(nullptr != get_a);
    CHECK_TRUE(nullptr != get_b);
    CHECK_TRUE(vars_equal(a, get_a));
    CHECK_TRUE(vars_equal(b, get_b));
  }

  {
    auto get_bad = m.get_variable("delta_quadrant", "i_dont_exist");
    CHECK_TRUE(nullptr == get_bad);
  }

  CHECK_TRUE(m.delete_variable("space::delta", "a"));
  CHECK_FALSE(m.delete_variable("space::delta", "no_exist"));
  CHECK_FALSE(m.delete_variable("not::mapped", "b"));
  
  // Ensure its deleted
  {
    auto get_a = m.get_variable("space::delta", "a");
    CHECK_TRUE(nullptr == get_a);
  }
}

