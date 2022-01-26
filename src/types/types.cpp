#include "types.hpp"

namespace titan
{
namespace obj_operation
{

bool copy_to(object* lhs, object*rhs)
{
  object * v_lhs = lhs;
  object * v_rhs = rhs;

  if(v_lhs->type == obj_type::VAR) {
    auto v = reinterpret_cast<object_var*>(v_lhs);
    v_lhs = v->value.get();
  }

  switch(v_lhs->type) {

  //
  //  this == int
  //
  case obj_type::INT:
  {
    auto dest = reinterpret_cast<object_int*>(v_lhs);

    switch(v_rhs->type) {
    case obj_type::INT:
    {
      auto source = reinterpret_cast<object_int*>(v_rhs);
      dest->value = source->value;
      return true;
    }
    case obj_type::FLOAT:
    {
      auto source = reinterpret_cast<object_float*>(rhs);
      dest->value = static_cast<int64_t>(source->value);
      return true;
    }
    default:
      return false;
    };
  }
  //
  //  this == float
  //
  case obj_type::FLOAT:
  {
    auto dest = reinterpret_cast<object_float*>(v_lhs);

    switch(v_rhs->type) {
    case obj_type::INT:
    {
      auto source = reinterpret_cast<object_int*>(v_rhs);
      dest->value = static_cast<double>(source->value);
      return true;
    }
    case obj_type::FLOAT:
    {
      auto source = reinterpret_cast<object_int*>(v_rhs);
      dest->value = source->value;
      return true;
    }
    default:
      return false;
    };
  }
  //
  //  this == string
  //
  case obj_type::STRING:
  {
    auto dest = reinterpret_cast<object_str*>(v_lhs);

    switch(v_rhs->type) {
    case obj_type::INT:
    {
      auto source = reinterpret_cast<object_int*>(v_rhs);
      dest->value = std::to_string(source->value);
      return true;
    }
    case obj_type::FLOAT:
    {
      auto source = reinterpret_cast<object_int*>(v_rhs);
      dest->value = std::to_string(source->value);
      return true;
    }
    case obj_type::STRING:
    {
      auto source = reinterpret_cast<object_int*>(v_rhs);
      dest->value = source->value;
      return true;
    }
    case obj_type::VAR:
    {
      auto source = reinterpret_cast<object_var*>(v_rhs);
      return copy_to(lhs, source->value.get());
    }
    default:
      return false;
    };
  }

  default:
   return true;
  };

  return false;
}

}
}
