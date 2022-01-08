#include "memory.hpp"

namespace vm
{

void memory::new_frame() 
{
  std::unique_lock<std::mutex> lock(_mtx);
  _frames.push_back({});
}

void memory::del_frame()
{
  std::unique_lock<std::mutex> lock(_mtx);
  if(_frames.empty()) {
    return;
  }
  _frames.pop_back();
}

std::optional<data::object*> memory::get(const std::string& name) 
{
  /*
      Optimization suggestion : cache
  */
  std::unique_lock<std::mutex> lock(_mtx);
  for(auto iter = _frames.rbegin(); iter != _frames.rend(); ++i) {
    if(iter->members.find(name) != iter->members.end()) {
      return { iter->members.at(name).get() };
    }
  }
  return std::nullopt;
}

void store(const std::string& name, data::object_ptr object) 
{
  std::unique_lock<std::mutex> lock(_mtx);
  if(_frames.empty()) {
    _frames.push_back({});
  }

  _frames.back().members[name] = std::move(object);
}

}
