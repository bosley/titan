#ifndef IMPORTS_HPP
#define IMPORTS_HPP

#include <map>
#include <memory>
#include <string>
#include <tuple>

namespace compiler {
class imports {
public:
  bool has_been_imported(const std::string target) const
  {
    if (_imported.empty()) {
      return false;
    }
    return _imported.find(target) != _imported.end();
  }

  void store_target_path(const std::string &target, const std::string &path)
  {
    _imported[target] = path;
  }

  std::tuple<bool, std::string> get_target_path(const std::string &target)
  {
    if (!has_been_imported(target)) {
      return {false, {}};
    }
    return {true, _imported.at(target)};
  }

private:
  std::map<std::string, std::string> _imported;
};

} // namespace compiler

#endif
