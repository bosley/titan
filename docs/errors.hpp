# Error Codes

Error codes are broken up into ranges. Each range corresponds to a particular portion of the titan compiler.

## Range [ 0 - 99 ] - Reserved

## Range [ 100 - 199 ] - Compiler::Lexer

```
  100 - Given item is not a file
  101 - Unable to open given file
```

## Range [ 200 - 999 ] - Compiler::Parser

Internal Errors 
```
  200 - Internal error - Attempt to reset with unset mark
  201 - Internal error - No prefix function for given token
  202 - Internal error - Non numerical item reached in parser::number
```

User Errors
```
  300 - Unable to locate import target
  301 - Invalid top level item
  302 - Expected conditional
  303 - Expected assignment
  304 - Unexpected token
```

## Range [ 1000 - 1999 ] - Compiler::Analyzer

Internal errors
```
  1000 - Internal error - Unable to determine integer value data
```

User Errors
```
  1100 - Duplicate function definition
  1101 - Duplicate variable definition
  1102 - No entry function present in program
  1103 - Expected expression for return of non-nil function
  1104 - Unknown identifier 
  1105 - Call to non function type 
  1106 - Expected variable item is of non-variable type
  1107 - Invalid parameter list size
  1108 - Parameter type mismatch
  1109 - Incorrect return type for entry function
  1110 - Invalid expression - Can no assign mismatched types
  1111 - Unable to implicitly cast types
  1112 - Invalid non-integer type for array index
```
