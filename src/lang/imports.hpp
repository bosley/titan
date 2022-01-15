#ifndef IMPORTS_HPP
#define IMPORTS_HPP

#include "tokens.hpp"

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <functional>

namespace titan {
class imports {
public:
  
  imports(std::function<std::vector<TD_Pair>(std::string)> importer,
      std::vector<std::string> include_dirs) : 
    import_file(importer), include_directories(include_dirs){}

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

  std::function<std::vector<TD_Pair>(std::string)> import_file;
  std::vector<std::string> include_directories;

private:
  std::map<std::string, std::string> _imported;
};

} // namespace titan

#endif
