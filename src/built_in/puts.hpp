#ifndef BI_PUTS_HPP
#define BI_PUTS_HPP

#include "exec/env.hpp"
#include "types/types.hpp"

namespace built_in
{

class puts : public titan::env::xfunc {
public:
  

  bool put_item(titan::object* item) 
  {
    switch(item->type) {
    case titan::obj_type::VAR:
    {
      auto i = reinterpret_cast<titan::object_var*>(item);
      return put_item(i->value.get());
    }
    case titan::obj_type::U8:
    {
      auto i = reinterpret_cast<titan::object_u8*>(item);
      std::cout << static_cast<int>(i->value);
      return true;
    }
    case titan::obj_type::U16:
    {
      auto i = reinterpret_cast<titan::object_u16*>(item);
      std::cout << i->value;
      return true;
    }
    case titan::obj_type::U32:
    {
      auto i = reinterpret_cast<titan::object_u32*>(item);
      std::cout << i->value;
      return true;
    }
    case titan::obj_type::U64:
    {
      auto i = reinterpret_cast<titan::object_u64*>(item);
      std::cout << i->value;
      return true;
    }
    case titan::obj_type::I8:
    {
      auto i = reinterpret_cast<titan::object_i8*>(item);
      std::cout << static_cast<int>(i->value);
      return true;
    }
    case titan::obj_type::I16:
    {
      auto i = reinterpret_cast<titan::object_i16*>(item);
      std::cout << i->value;
      return true;
    }
    case titan::obj_type::I32:
    {
      auto i = reinterpret_cast<titan::object_i32*>(item);
      std::cout << i->value;
      return true;
    }
    case titan::obj_type::I64:
    {
      auto i = reinterpret_cast<titan::object_i64*>(item);
      std::cout << i->value;
      return true;
    }
    case titan::obj_type::FLOAT:
    {
      auto i = reinterpret_cast<titan::object_float*>(item);
      std::cout << i->value;
      return true;
    }

    case titan::obj_type::STRING:
    {
      auto i = reinterpret_cast<titan::object_str*>(item);
      std::cout << i->value;
      return true;
    }
    case titan::obj_type::ARRAY:
    {
      //  TODO:
      //    object_array doesn't have access to segment size information
      //    so [][][][][] ... N deeo items can't be dislayed in a way that
      //    the user will easily understand
      //
      //    - This needs to be updated once the object has sufficient information
      //
      //
      //
      auto i = reinterpret_cast<titan::object_array*>(item);
      std::cout << "[";
      for(size_t x = 0; x < i->value.size(); x++) {
        if(!put_item(i->value[x])) {
          return false;
        }
        if(x != i->value.size()-1) {
          std::cout << ", ";
        }
      }
      std::cout << "]";
      return true;
    }
    case titan::obj_type::USER_DEF: {
      std::cout
          << "built_in function PUTS can not currently display a USER_DEF object"
          << std::endl;
      return false;
    }
    case titan::obj_type::NIL:
    {
      std::cout << "nil";
      return true;
    }
    default:
      return false;
    };
  }

  // Returns 1 if the print was a success, 0 otherwise
  virtual void execute() override
  {
    for(auto& param : parameters) {
      if(!put_item(param)) {
       result = new titan::object_i8(0); 
      }
    }

    result = new titan::object_i8(1);
  }
};

}

#endif