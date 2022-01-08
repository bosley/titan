#ifndef VM_DATA_TYPES_HPP
#define VM_DATA_TYPES_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility>

namespace vm {
namespace data {

enum class object_type {
  U8,
  U16,
  U32,
  U64,
  I8,
  I16,
  I32,
  I64,
  REAL,
  STR,
  ARRAY
};

class object {
public:
  object(object_type type) : underlying_type(type) {}
  object_type underlying_type;
};
using object_ptr = std::unique_ptr<object>;

class integer_u8 : public object {
public:
  integer_u8() : object(object_type::U8), value(0) {}
  integer_u8(uint8_t val) : object(object_type::U8), value(val) {}
  uint8_t  value;
};
using integer_u8_ptr = std::unique_ptr<integer_u8>;

class integer_u16 : public object {
public:
  integer_u16() : object(object_type::U16), value(0) {}
  integer_u16(uint16_t val) : object(object_type::U16), value(val) {}
  uint16_t  value;
};
using integer_u16_ptr = std::unique_ptr<integer_u16>;

class integer_u32 : public object {
public:
  integer_u32() : object(object_type::U32), value(0) {}
  integer_u32(uint32_t val) : object(object_type::U32), value(val) {}
  uint32_t  value;
};
using integer_u32_ptr = std::unique_ptr<integer_u32>;

class integer_u64 : public object {
public:
  integer_u64() : object(object_type::U64), value(0) {}
  integer_u64(uint64_t val) : object(object_type::U64), value(val) {}
  uint64_t  value;
};
using integer_u64_ptr = std::unique_ptr<integer_u64>;

class integer_i8 : public object {
public:
  integer_i8() : object(object_type::I8), value(0) {}
  integer_i8(int8_t val) : object(object_type::I8), value(val) {}
  int8_t  value;
};
using integer_i8_ptr = std::unique_ptr<integer_i8>;

class integer_i16 : public object {
public:
  integer_i16() : object(object_type::I16), value(0) {}
  integer_i16(int16_t val) : object(object_type::I16), value(val) {}
  int16_t  value;
};
using integer_i16_ptr = std::unique_ptr<integer_i16>;

class integer_i32 : public object {
public:
  integer_i32() : object(object_type::I32), value(0) {}
  integer_i32(int32_t val) : object(object_type::I32), value(val) {}
  int32_t  value;
};
using integer_i32_ptr = std::unique_ptr<integer_i32>;

class integer_i64 : public object {
public:
  integer_i64() : object(object_type::I64), value(0) {}
  integer_i64(int64_t val) : object(object_type::I64), value(val) {}
  int64_t  value;
};
using integer_i64_ptr = std::unique_ptr<integer_i64>;

class real : public object {
public:
  real() : object(object_type::REAL), value(0) {}
  real(double val) : object(object_type::REAL), value(val) {}
  double  value;
};
using real_ptr = std::unique_ptr<real>;

class str : public object {
public:
  str() : object(object_type::STR), value(0) {}
  str(std::string val) : object(object_type::STR), value(val) {}
  std::string  value;
};
using str_ptr = std::unique_ptr<str>;

class array : public object {
public:
  array(object_type data_type) : object(object_type::ARRAY), data_type(data_type) {}
  object_type data_type;
  std::vector<uint64_t> segments;
  std::vector<object_ptr> data;
};
using str_ptr = std::unique_ptr<str>;
}
}
#endif
