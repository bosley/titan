#include "space.hpp"

#include <iostream>

namespace titan
{

space::space() : _scope_depth(0)
{
  _global_scope.parent = nullptr;
  _global_scope.sub_scope = nullptr;
  _operating_scope = &_global_scope;
}

space::~space()
{
  while(_scope_depth) {
    leave_scope();
  }
  while(!_top_level_scopes.empty()) {
    pop_scope();
  }
}

void space::push_top_level_scope() 
{
  _top_level_scopes.push({&_global_scope, nullptr, {}});
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

  scope* tmp = &_top_level_scopes.top();
  tmp->parent = nullptr;
  while(tmp->sub_scope) {
    auto staged = tmp;
    tmp = tmp->sub_scope;
    delete staged;
  }

  _top_level_scopes.pop();

  if(_top_level_scopes.empty()){
    _operating_scope = &_global_scope;
  } else {
    _operating_scope = &_top_level_scopes.top();
  }
}

void space::sub_scope()
{
  auto s = new scope();
  s->parent = _operating_scope;
  s->sub_scope = nullptr;

  _operating_scope->sub_scope = s;
  _operating_scope = s;
  ++_scope_depth;
}

void space::leave_scope()
{
  if(_scope_depth != 0) {
    --_scope_depth;
  }

  auto parent = _operating_scope->parent;

  auto tmp = _operating_scope;
  while(tmp->sub_scope) {
    auto staged = tmp;
    tmp = tmp->sub_scope;
    delete staged;
  }
  delete tmp;

  if(parent) {
    _operating_scope = parent;
  } else {
    _operating_scope = &_global_scope;
  }
  _operating_scope->sub_scope = nullptr;
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
bool space::new_var(instructions::variable *var) 
{
  if(!var) {
    return false;
  }

  _operating_scope->members[var->name] = instructions::variable_ptr(var);
  return true;
}

}
