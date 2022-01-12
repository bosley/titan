

```

use "std::io"          // Use installed library $TITAN/lib

import "some_lib"      // Checks the local dirs for library


fn add(lhs:u8, rhs:u8) -> u8 {
  return lhs + rhs;
}

fn divide(lhs:i64, rhs::i64) -> i64 {

  if( 0 == lhs ) {
    std::io::print("Divide by 0 err");
    return 0;
  }

  return lhs / rhs;
}


```
