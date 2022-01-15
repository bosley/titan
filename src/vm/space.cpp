#include "space.hpp"

namespace titan
{

space::space()
{
  _global_scope.parent = nullptr;
  _global_scope.sub_scope = nullptr;
  _operating_scope = &_global_scope;
}

void space::push_top_level_scope() 
{
  _top_level_scopes.push({
    {}, &_global_scope, nullptr
      });

  _operating_scope = &_top_level_scopes.top();
}

void space::pop_scope()
{
  // Neither of these 'should' happen which mean they will 
  // one day
  if(_top_level_scopes.empty()){
    return;
  }
  if(!_operating_scope) {
    return;
  }

  auto tmp = &_top_level_scopes.top();
  tmp ->parent = nullptr;
  while(tmp->sub_scope) {
    tmp = tmp->sub_scope;
    auto staged = tmp;
    delete staged;
  }
  delete tmp;

  _top_level_scopes.pop();

  if(_top_level_scopes.empty()){
    _operating_scope = &_global_scope;
  } else {
    _operating_scope = &_top_level_scopes.top();
  }
}

void space::sub_scope()
{
  _operating_scope->sub_scope = new scope(); 
  _operating_scope->parent = _operating_scope;
  _operating_scope = _operating_scope->sub_scope;
  _operating_scope->sub_scope = nullptr;
}

void space::leave_scope()
{
  auto parent = _operating_scope->parent;

  auto tmp = _operating_scope;
  while(tmp->sub_scope) {
    tmp = tmp->sub_scope;
    auto staged = tmp;
    delete staged;
  }

  // Delete the scope if it isn't the global scope
  if(parent) {
    _operating_scope = parent;
    delete tmp;
  } else {
    _operating_scope = &_global_scope;
  }
}

instructions::variable *space::get_variable(const std::string& name) 
{
  // Check current scope
  if(_operating_scope->members.find(name) != _operating_scope->members.end()) {
    return _operating_scope->members.at(name).get();
  }

  // Check all parents
  auto tmp = _operating_scope->parent;
  while(tmp) {
    if(tmp->members.find(name) != tmp->members.end()) {
      return tmp->members.at(name).get();
    }
    tmp = tmp->parent;
  }

  return nullptr;
}

bool space::delete_var(const std::string& name)
{
  // Check current scope
  if(_operating_scope->members.find(name) != _operating_scope->members.end()) {
    _operating_scope->members.erase(name);
    return true;
  }

  // Check all parents
  auto tmp = _operating_scope->parent;
  while(tmp) {
    if(tmp->members.find(name) != tmp->members.end()) {
      tmp->members.erase(name);
      return true;
    }
    tmp = tmp->parent;
  }
  return false;
}

// Takes ownership of the data
bool space::new_var(const std::string& name, instructions::variable *var) 
{
  if(!var) {
    return false;
  }
  _operating_scope->members[name] = instructions::variable_ptr(var);
  return true;
}

}
