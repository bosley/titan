#ifndef VM_HPP
#define VM_HPP

#include "data/types.hpp"
#include "settings.hpp"

namespace vm {

class machine {
public:

  machine();

private:

  data::object * registers[settings::register_count];

};

}

#endif
