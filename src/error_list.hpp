#ifndef ERROR_LIST_HPP
#define ERROR_LIST_HPP

namespace error {

namespace lexer {
  static constexpr uint16_t TARGET_NOT_FILE = 100;
  static constexpr uint16_t TARGET_CANT_OPEN = 101;
} // end lexer

namespace parser {
  static constexpr uint16_t INTERNAL_MARK_UNSET = 200;
  static constexpr uint16_t INTERNAL_NO_FN_FOR_TOK = 201;
  static constexpr uint16_t INTERNAL_NON_NUMERIC_REACHED = 202;
  static constexpr uint16_t UNABLE_TO_LOCATE_IMPORT = 300;
  static constexpr uint16_t INVALID_TL_ITEM = 301;
  static constexpr uint16_t EXPECTED_CONDITIONAL = 302;
  static constexpr uint16_t EXPECTED_ASSIGNMENT = 303;
  static constexpr uint16_t UNEXPECTED_TOKEN = 304;
} // end parser

namespace analyzer {
  static constexpr uint16_t INTERNAL_UNABLE_TO_DETERMINE_INT_VAL = 1000;
  static constexpr uint16_t DUPLICATE_FUNCTION_DEF = 1100;
  static constexpr uint16_t DUPLICATE_VARIABLE_DEF = 1101;
  static constexpr uint16_t ENTRY_NOT_FOUND = 1102;
  static constexpr uint16_t RETURN_EXPECTED_EXPRESSION = 1103;
  static constexpr uint16_t UNKNOWN_ID = 1104;
  static constexpr uint16_t UNMATCHED_CALL = 1105;
  static constexpr uint16_t EXPECTED_VARIABLE = 1106;
  static constexpr uint16_t PARAM_SIZE_MISMATCH = 1107;
  static constexpr uint16_t PARAM_TYPE_MISMATCH = 1108;
  static constexpr uint16_t INCORRECT_ENTRY_RETURN = 1109;
  static constexpr uint16_t INVALID_EXPRESSION = 1110;
  static constexpr uint16_t IMPLICIT_CAST_FAIL = 1111;
  static constexpr uint16_t INVALID_ARRAY_IDX = 1112;
  static constexpr uint16_t DUPLICATE_PARAMETER = 1113;
} // end analyzer

}

#endif
