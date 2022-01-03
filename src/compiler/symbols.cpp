#include "symbols.hpp"

#include <algorithm>

namespace compiler {

namespace symbol {

table::table() : _global_scope("global"), _curr_scope(&_global_scope) {}

void table::set_scope_to_global() { _curr_scope = &_global_scope; }

bool table::activate_top_level_scope(const std::string &name)
{
  auto locate = [&](const auto &item) { return item->name == name; };
  auto iter = std::find_if(_global_scope.sub_scopes.begin(),
                           _global_scope.sub_scopes.end(), locate);

  if (iter == _global_scope.sub_scopes.end()) {
    return false;
  }

  auto tmp = _curr_scope;
  _curr_scope = *iter;
  _curr_scope->prev_scope = tmp;
  return true;
}

void table::add_scope(const std::string &name)
{
  _curr_scope->sub_scopes.push_back(new scope(name));
}

void table::add_scope_and_enter(const std::string &name)
{
  auto new_scope = new scope(name);
  _curr_scope->sub_scopes.push_back(new_scope);

  auto tmp = _curr_scope;
  _curr_scope = new_scope;
  _curr_scope->prev_scope = tmp;
}

void table::pop_scope()
{
  if (!_curr_scope->prev_scope) {
    _curr_scope = &_global_scope;
    return;
  }

  _curr_scope = _curr_scope->prev_scope;
}

bool table::add_symbol(const std::string &name, parse_tree::function *func)
{
  if (exists(name, true)) {
    return false;
  }

  variant_data v_data;
  v_data.type = variant_type::FUNCTION;
  v_data.function = func;

  _curr_scope->entries.push_back({name, v_data});
  add_scope(name);
  return true;
}

bool table::add_symbol(const std::string &name,
                       parse_tree::assignment_statement *var)
{
  if (exists(name, true)) {
    return false;
  }

  variant_data v_data;
  v_data.type = variant_type::ASSIGNMENT;
  v_data.assignment = var;

  _curr_scope->entries.push_back({name, v_data});
  return true;
}

bool table::exists(const std::string &v, bool current_only)
{
  if (current_only) {
    return scope_contains_item(_curr_scope, v);
  }

  scope *locator = _curr_scope;
  while (locator) {
    if (scope_contains_item(locator, v)) {
      return true;
    }
    locator = locator->prev_scope;
  }

  return false;
}

bool table::scope_contains_item(scope *s, const std::string &v)
{
  return s->entries.end() !=
         std::find_if(s->entries.begin(), s->entries.end(),
                      [&](const auto &item) { return item.name == v; });
}

std::optional<variant_data> table::lookup(const std::string &v,
                                          bool current_only)
{
  scope *locator = _curr_scope;
  auto locate = [&](const auto &item) { return item.name == v; };

  auto iter =
      std::find_if(locator->entries.begin(), locator->entries.end(), locate);

  if (iter != locator->entries.end()) {
    return std::optional<variant_data>((*iter).data);
  }

  if (current_only) {
    return std::nullopt;
  }

  locator = locator->prev_scope;
  while (locator) {
    iter =
        std::find_if(locator->entries.begin(), locator->entries.end(), locate);

    if (iter != locator->entries.end()) {
      return std::optional<variant_data>((*iter).data);
    }
    locator = locator->prev_scope;
  }

  return std::nullopt;
}

} // namespace symbol
} // namespace compiler
