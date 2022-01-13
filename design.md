


```


  main
    parses input arguments and starts up a "titan" object 

  titan.h/cpp 
    Runs the program to completion either in "repl" mode, or by 
    opening a file and iterpreting the contents.

    This object can be configured to just 'analyze' the input to ensure that its
    valid. If configured with 'analyze' you can also chose to not run the 
    input so execution wont take place. This is helpful in determining if 
    pre-written source would fail at execution time due to invalid semantics

  tokens.hpp
    A list of all tokens that can be found within the language, and some helpful structures
    for grouping them to a location in the source. This file also provides a print function
    that will display all the contents of a TDPair. A TDPair is a structure that ties the 
    token value to some origination data.

  lexer.h/cpp
    Takes a chunk of raw code in the form of string data and generates a list of TDPair (see above)
    objects. Using these lists of tokens we can parse the language together and give detailed error
    reporting (file, line, col, err, suggestions, etc)

  instructions.h/cpp
    List of all language instructions ina a form that can be analyzed and executed. See the file
    instructions.hpp to see more detailed information of the types here

  parser.h/cpp
    Takes in a TDPair list and assumes that the list will become some object(s) found in the above listed
    instructions.h/cpp. 

  // Not yet constructed

  maps.h/cpp
    Storage / owner for inspection-time (exectuon time || analyzation time) program variables and definitions. 
      - Function defintions, user struct definitions
      - Function / member variable instances
      - Provides scoping

  builtins
    containing builtin.h/cpp and various other sources the builtins directory will contain all 
    built-in functions that need to interact with the OS (std::io::print, std::net::udp::send, etc)

  NOTE : Env and analyzer (below) will share some functionality, so we may want to have some shared
         functionality like (run_expression) where when a leaf item is reached a callback is given
         to the object (env, or analyzer) that the could use to either execute action against (env), 
         or to inspect the item (analyzer)

  env.h/cpp
    Owner of the various maps (above) and executor of program instructions (above).


  analyzer.h/cpp
    Owner of the various maps (above) and analyzer of program instructions (above).



  // MUCH LATER BUT AS A REMINDER

  The system (titan.h/cpp) could be used as a model to leverage lexer/parser/env/analyzer in a new obejct
  that could be easily added to external priojects that would allow the insertion of commands, and 
  retrieval of resulting data easilty. Using this object an external object could expose a callable interface
  into the environment so methods local to the external object could be invoked by a titan script


```
