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
  USER_DEF
};

// Interface for user defined objects to be stored in an object class
class object_if {
public:
  object_if() : object_id(0) {}
  object_if(uint64_t id) : object_id(id) {}
  virtual ~object_if() = default;
  uint64_t object_id;
};
using object_if_ptr = std::unique_ptr<object_if>;

class object {
public:
  object(obj_type type) : type(type) {}
  obj_type type;
};
using object_ptr = std::unique_ptr<object>;

class object_u8 : public object {
public:
  object_u8() : object(obj_type::U8), value(0) {}
  object_u8(uint8_t value) : object(obj_type::U8), value(value) {}
  uint8_t value;
};
using object_u8_ptr = std::unique_ptr<object_u8>;

class object_u16 : public object {
public:
  object_u16() : object(obj_type::U16), value(0) {}
  object_u16(uint8_t value) : object(obj_type::U16), value(value) {}
  uint64_t value;
};
using object_u16_ptr = std::unique_ptr<object_u16>;

class object_u32 : public object {
public:
  object_u32() : object(obj_type::U32), value(0) {}
  object_u32(uint8_t value) : object(obj_type::U32), value(value) {}
  uint32_t value;
};
using object_u32_ptr = std::unique_ptr<object_u32>;

class object_u64 : public object {
public:
  object_u64() : object(obj_type::U64), value(0) {}
  object_u64(uint8_t value) : object(obj_type::U64), value(value) {}
  uint64_t value;
};
using object_u64_ptr = std::unique_ptr<object_u64>;

class object_i8 : public object {
public:
  object_i8() : object(obj_type::I8), value(0) {}
  object_i8(uint8_t value) : object(obj_type::I8), value(value) {}
  int8_t value;
};
using object_i8_ptr = std::unique_ptr<object_i8>;

class object_i16 : public object {
public:
  object_i16() : object(obj_type::I16), value(0) {}
  object_i16(uint8_t value) : object(obj_type::I16), value(value) {}
  int16_t value;
};
using object_i16_ptr = std::unique_ptr<object_i16>;

class object_i32 : public object {
public:
  object_i32() : object(obj_type::I32), value(0) {}
  object_i32(uint8_t value) : object(obj_type::I32), value(value) {}
  int32_t value;
};
using object_i32_ptr = std::unique_ptr<object_i32>;

class object_i64 : public object {
public:
  object_i64() : object(obj_type::I64), value(0) {}
  object_i64(uint8_t value) : object(obj_type::I64), value(value) {}
  int64_t value;
};
using object_i64_ptr = std::unique_ptr<object_i64>;

class object_float : public object {
public:
  object_float() : object(obj_type::FLOAT), value(0.00) {}
  object_float(double value) : object(obj_type::FLOAT), value(value) {}
  double value;
};
using object_float_ptr = std::unique_ptr<object_float>;

class object_str : public object {
public:
  object_str() : object(obj_type::STRING), value("") {}
  object_str(std::string value) : object(obj_type::STRING), value(value) {}
  std::string value;
};
using object_str_ptr = std::unique_ptr<object_str>;

class object_array : public object {
public:
  object_array() : object(obj_type::ARRAY), value({}) {}
  object_array(std::vector<object*> value) : object(obj_type::ARRAY), value(value) {}
  std::vector<object*> value;
};
using object_array_ptr = std::unique_ptr<object_array>;

class object_user_defined : public object {
public:
  object_user_defined() : object(obj_type::USER_DEF) {}
  object_user_defined(object_if_ptr value) : object(obj_type::USER_DEF), value(std::move(value)) {}

  object_if_ptr value;
};
using object_user_defined_ptr = std::unique_ptr<object_user_defined>;
}

#endif
