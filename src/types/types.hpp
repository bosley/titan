#ifndef TYPES_HPP
#define TYPES_HPP

#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace titan
{

enum class obj_type {
  U8,
  U16,
  U32,
  U64,
  I8,
  I16,
  I32,
  I64,
  FLOAT,
  STRING,
  ARRAY,
  USER_DEF,
  VAR,
  NIL
};


class object {
public:
  virtual ~object() = default;
  virtual object* clone() const = 0;

  object(obj_type type) : type(type) {}
  obj_type type;
};
using object_ptr = std::unique_ptr<object>;

// Interface for user defined objects to be stored in an object class
class object_if {
public:
  object_if() : object_id(0) {}
  object_if(uint64_t id) : object_id(id) {}
  virtual ~object_if() = default;
  virtual object_if* clone() const = 0;
  uint64_t object_id;
};
using object_if_ptr = std::unique_ptr<object_if>;

class object_nil : public object {
public:
  object_nil() : object(obj_type::NIL) {}

  virtual object* clone() const { return new object_nil(); }
};
using object_nil_ptr = std::unique_ptr<object_nil>;

class object_var : public object {
public:
  object_var() : object(obj_type::VAR), value(new object_nil()) {}
  object_var(object* value) : object(obj_type::VAR), value(value) {}
  object_ptr value;

  virtual object* clone() const { return new object_var(value.get()->clone()); }
};
using object_var_ptr = std::unique_ptr<object_var>;

class object_u8 : public object {
public:
  object_u8() : object(obj_type::U8), value(0) {}
  object_u8(uint8_t value) : object(obj_type::U8), value(value) {}
  uint8_t value;
  virtual object* clone() const { return new object_u8(value); }
};
using object_u8_ptr = std::unique_ptr<object_u8>;

class object_u16 : public object {
public:
  object_u16() : object(obj_type::U16), value(0) {}
  object_u16(uint16_t value) : object(obj_type::U16), value(value) {}
  uint16_t value;
  virtual object* clone() const { return new object_u16(value); }
};
using object_u16_ptr = std::unique_ptr<object_u16>;

class object_u32 : public object {
public:
  object_u32() : object(obj_type::U32), value(0) {}
  object_u32(uint32_t value) : object(obj_type::U32), value(value) {}
  uint32_t value;
  virtual object* clone() const { return new object_u32(value); }
};
using object_u32_ptr = std::unique_ptr<object_u32>;

class object_u64 : public object {
public:
  object_u64() : object(obj_type::U64), value(0) {}
  object_u64(uint64_t value) : object(obj_type::U64), value(value) {}
  uint64_t value;
  virtual object* clone() const { return new object_u64(value); }
};
using object_u64_ptr = std::unique_ptr<object_u64>;

class object_i8 : public object {
public:
  object_i8() : object(obj_type::I8), value(0) {}
  object_i8(int8_t value) : object(obj_type::I8), value(value) {}
  int8_t value;
  virtual object* clone() const { return new object_i8(value); }
};
using object_i8_ptr = std::unique_ptr<object_i8>;

class object_i16 : public object {
public:
  object_i16() : object(obj_type::I16), value(0) {}
  object_i16(int16_t value) : object(obj_type::I16), value(value) {}
  int16_t value;
  virtual object* clone() const { return new object_i16(value); }
};
using object_i16_ptr = std::unique_ptr<object_i16>;

class object_i32 : public object {
public:
  object_i32() : object(obj_type::I32), value(0) {}
  object_i32(int32_t value) : object(obj_type::I32), value(value) {}
  int32_t value;
  virtual object* clone() const { return new object_i32(value); }
};
using object_i32_ptr = std::unique_ptr<object_i32>;

class object_i64 : public object {
public:
  object_i64() : object(obj_type::I64), value(0) {}
  object_i64(int64_t value) : object(obj_type::I64), value(value) {}
  int64_t value;
  virtual object* clone() const { return new object_i64(value); }
};
using object_i64_ptr = std::unique_ptr<object_i64>;

class object_float : public object {
public:
  object_float() : object(obj_type::FLOAT), value(0.00) {}
  object_float(double value) : object(obj_type::FLOAT), value(value) {}
  double value;
  virtual object* clone() const { return new object_float(value); }
};
using object_float_ptr = std::unique_ptr<object_float>;

class object_str : public object {
public:
  object_str() : object(obj_type::STRING), value("") {}
  object_str(std::string value) : object(obj_type::STRING), value(value) {}
  std::string value;
  virtual object* clone() const { return new object_str(value); }
};
using object_str_ptr = std::unique_ptr<object_str>;

// TODO : 
//    The object array will need the segments list and some other 
//    information from the AST array so we can determine how to do things
//    like access specific indexes on a [][][][] ... N deep item
//
//
class object_array : public object {
public:
  object_array() : object(obj_type::ARRAY), value({}) {}
  object_array(std::vector<object*> value) : object(obj_type::ARRAY), value(value) {}
  std::vector<object*> value;
  virtual object* clone() const { return new object_array(value); }
};
using object_array_ptr = std::unique_ptr<object_array>;

class object_user_defined : public object {
public:
  object_user_defined() : object(obj_type::USER_DEF) {}
  object_user_defined(object_if_ptr value) : object(obj_type::USER_DEF), value(std::move(value)) {}
  object_user_defined(object_if* value) : object(obj_type::USER_DEF), value(value) {}

  object_if_ptr value;
  virtual object* clone() const { return new object_user_defined(value->clone()); }
};
using object_user_defined_ptr = std::unique_ptr<object_user_defined>;

}

#endif
