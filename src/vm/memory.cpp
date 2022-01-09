#include "memory.hpp"

namespace vm
{

memory::~memory() 
{
  for(auto& e : _frames) {
    delete e;
  }
}

void memory::new_frame() 
{
  std::unique_lock<std::mutex> lock(_mtx);
  _frames.push_back(new frame());

}

void memory::new_frame(std::unordered_map<std::string, data::object*> references)
{
  std::unique_lock<std::mutex> lock(_mtx);
  _frames.push_back(new frame(references));
}

void memory::del_frame()
{
  std::unique_lock<std::mutex> lock(_mtx);
  if(_frames.empty()) {
    return;
  }
  delete _frames.back();
  _frames.pop_back();
}

std::optional<data::object*> memory::get(const std::string& name) 
{
  /*
      Optimization suggestion : cache
  */
  std::unique_lock<std::mutex> lock(_mtx);
  if(_frames.back()->references.find(name) != _frames.back()->references.end()) {
    return { _frames.back()->references.at(name) };
  }
  if(_frames.back()->members.find(name) != _frames.back()->members.end()) {
    return { _frames.back()->members.at(name).get() };
  }
  return std::nullopt;
}

void memory::store(const std::string& name, data::object_ptr object) 
{
  std::unique_lock<std::mutex> lock(_mtx);
  if(_frames.empty()) {
    _frames.push_back(new frame());
  }
  _frames.back()->members.emplace(name, std::move(object));
}

}
