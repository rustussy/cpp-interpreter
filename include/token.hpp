#ifndef __TOKEN_HPP
#define __TOKEN_HPP
#include <string>

// INTERNAL (LEXER-ONLY) VALUES ARE PREFIXED WITH __ AND HAS
// A NEGATIVE INT ASSIGNED.
enum class TokenType {
  INT,
  CHAR,
  PLUS,
  MINUS,
  MUL,
  DIV,
  MOD,

  ASSIGN,
  VAR,
  VAR_TYPE,

  RET,

  FOR,
  WHILE,

  IF,
  ELSE,

  EOF_TOKEN,

  PAREN_OPEN,
  PAREN_CLOSE,
  BRACE_OPEN,
  BRACE_CLOSE,
  BRKET_OPEN,
  BRKET_CLOSE,

  SEMI,
  COMMA,

  CMP_GTE,
  CMP_GRT,
  CMP_LTE,
  CMP_LES,
  CMP_EQU,
  CMP_NEQ,

  IO,
  BW_SHIFTL,
  BW_SHIFTR,
  BW_XOR,

  NEGATE,
  AND,
  OR,

  // strings, cmp operators, etc...
  OTHERS = -1,
};

struct Token {
  TokenType type;
  std::string value;
};
#endif