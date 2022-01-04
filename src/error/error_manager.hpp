#ifndef ERROR_MANAGER_HPP
#define ERROR_MANAGER_HPP

#include "alert/alert.hpp"

#include <string>
#include <unordered_map>

namespace error {

class manager {

public:
  manager(std::string reporter);
  void raise(uint16_t error_number, alert::config *cfg = nullptr);
  uint64_t num_errors() const { return _num_errors; }

private:

  std::string _reporter;
  uint16_t _num_errors;
  std::unordered_map<uint16_t, std::string> _error_map;
};

} // End error


#endif
