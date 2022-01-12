#include <vm/memory.hpp>
#include <iostream>
#include <vector>

#include <CppUTest/TestHarness.h>

/*
 *    TODO : Make these tests better
 */
TEST_GROUP(memory_tests){};

TEST(memory_tests, all)
{
  /*
   *  Create some test datas
   * */
  std::vector<vm::data::object_ptr> tcs;
  tcs.emplace_back(new vm::data::integer_u8(21));
  tcs.emplace_back(new vm::data::integer_u16(22));
  tcs.emplace_back(new vm::data::integer_u32(23));
  tcs.emplace_back(new vm::data::integer_u64(24));
  tcs.emplace_back(new vm::data::integer_i8(25));
  tcs.emplace_back(new vm::data::integer_i16(26));
  tcs.emplace_back(new vm::data::integer_i32(27));
  tcs.emplace_back(new vm::data::integer_i64(28));
  tcs.emplace_back(new vm::data::real(3.14159));
  tcs.emplace_back(new vm::data::str("AHHHHHHHHHH"));
  tcs.emplace_back(new vm::data::array(vm::data::object_type::U8));
  auto arr = reinterpret_cast<vm::data::array*>(tcs.back().get());
  arr->data.emplace_back(new vm::data::integer_u8(40));
  arr->data.emplace_back(new vm::data::integer_u8(41));
  arr->data.emplace_back(new vm::data::integer_u8(42));
  arr->data.emplace_back(new vm::data::integer_u8(43));

  vm::memory memory;

  memory.new_frame();

  uint64_t item = 1;
  for(auto &tc : tcs) {
    memory.store(std::to_string(item++), std::move(tc));
  }

  while(item > 1) {
    --item;
    auto res = memory.get(std::to_string(item));
    CHECK_TRUE(res != std::nullopt);
  }

  // "Trick" the array into deallocation :
  // https://stackoverflow.com/questions/10812306/gcc-linux-cpputest-shows-memory-leak-using-static-vectors-false-positive
  std::vector<vm::data::object_ptr>().swap(arr->data);
  std::vector<vm::data::object_ptr>().swap(tcs);
}
