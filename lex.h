#pragma once

#include "util.h"
#include <stdbool.h>
#include <ctype.h>

typedef enum mi_tok_type mi_tok_type;

enum mi_tok_type {
  // keywords
  TT_MODULE,
  TT_CLASS,
  TT_RET,
  TT_FOR,
  TT_IF,
  TT_ELSE,
  TT_MATCH,
  TT_USE,
  TT_DO,
  TT_N_KEYWORDS,
  // symbols
  TT_DOUBLE_COLON,
  TT_COLON,
  TT_SEMICOLON,
  TT_LCURLY,
  TT_RCURLY,
  TT_DOT,
  TT_COMMA,
  TT_LBRACKET,
  TT_RBRACKET,
  TT_LPAREN,
  TT_RPAREN,
  TT_ADD,
  TT_SUB,
  TT_MUL,
  TT_DIV,
  TT_MOD,
  TT_INC,
  TT_DEC,
  TT_DOUBLE_ADD,
  TT_DOUBLE_SUB,
  TT_VAR_DECL,
  TT_EQ,
  TT_DOUBLE_EQ,
  TT_NEQ,
  TT_RANGE,
  TT_CAST,
  TT_LT,
  TT_LTE,
  TT_GT,
  TT_GTE,
  TT_PIPE,
  TT_AMPERSAND,
  TT_EXCLAMATION,
  // literals
  TT_STRING,
  TT_INT,
  TT_HEX,
  TT_FLOAT,
  // other
  TT_ID,
  TT_PREPROC,
  TT_EOF,
  TT_N,
};

c_str mi_tok_type_str(mi_tok_type type);

typedef struct mi_tok mi_tok;

struct mi_tok {
  mi_tok_type type;
  mi_str text;
  mi_pos begin, end;
};

mi_tok mi_tok_new(mi_tok_type type, mi_str text, mi_pos begin, mi_pos end);

str mi_tok_str(mi_tok tok);

typedef struct mi_lexer mi_lexer;

struct mi_lexer {
  mi_str text;
  mi_pos pos;
  mi_tok* toks;
};

bool mi_lex_is_id_begin(char c);

bool mi_lex_is_id_continue(char c);

mi_tok* mi_lex(c_str path);