#ifndef VM_MEMORY_HPP
#define VM_MEMORY_HPP

#include "data/types.hpp"

#include <mutex>
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>

namespace vm
{

//  Object storage
//
class memory
{
public:

  ~memory();

  // Create a new frame for data storage
  void new_frame();

  // Create a new frame with references accessable to this frame
  void new_frame(std::unordered_map<std::string, data::object*> references);

  // Remove the current frame
  void del_frame();

  // Attempt to get an object pointer by name
  std::optional<data::object*> get(const std::string& name);

  // Store (move) object pointer into memory.
  // Once this is done the object will be owned by memory for the
  // rest of its lifetime
  void store(const std::string& name, data::object_ptr object);

private:
  class frame {
  public:
    frame(){}
    frame(std::unordered_map<std::string, data::object*> refs) : references(refs) {}
    std::unordered_map<std::string, data::object*> references;
    std::unordered_map<std::string, data::object_ptr> members;
  };
  std::vector< frame* > _frames;
  std::mutex _mtx;
};

}

#endif
