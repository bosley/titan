#include "space.hpp"

#include <iostream>

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
  /*
  scope* tmp = &_top_level_scopes.top();
  tmp->parent = nullptr;
  while(tmp->sub_scope) {
    auto staged = tmp;
    tmp = tmp->sub_scope;
    delete staged;
  }
  delete tmp;
  */
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
  _operating_scope->sub_scope->parent = _operating_scope;
  _operating_scope = _operating_scope->sub_scope;
  _operating_scope->sub_scope = nullptr;
}

void space::leave_scope()
{
  auto parent = _operating_scope->parent;

  auto tmp = _operating_scope;
  while(tmp->sub_scope) {
    auto staged = tmp;
    tmp = tmp->sub_scope;
    delete staged;
  }

  // Delete the scope if it isn't the global scope
  if(parent) {
    _operating_scope = parent;
  } else {
    _operating_scope = &_global_scope;
  }
}

instructions::variable *space::get_variable(const std::string& name) 
{
  std::cout << "Get variable : " << name << std::endl;

  // Check current scope
  if(_operating_scope->members.find(name) != _operating_scope->members.end()) {
  
    std::cout << ">> Get from operating scope" << std::endl;
    
    return _operating_scope->members.at(name).get();
  }

  // Check all parents
  auto tmp = _operating_scope->parent;
  while(tmp) {
    
    std::cout << ">> Searching parent" << std::endl;

    if(tmp->members.find(name) != tmp->members.end()) {
    
      std::cout << ">> Get from parent" << std::endl;

      return tmp->members.at(name).get();
    }
    tmp = tmp->parent;
  }

  std::cout << "Couldn't locate variable" << std::endl;

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

  std::cout << "Store variable : " << var->name << std::endl;

  _operating_scope->members[var->name] = instructions::variable_ptr(var);
  return true;
}

}
