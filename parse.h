#pragma once

#include "util.h"
#include "lex.h"
#include "dyn_arr.h"
#include <stdlib.h>

typedef enum mi_node_type mi_node_type;

enum mi_node_type {
  NT_CLASS,
  NT_FUNC,
  NT_FIELD,
  NT_MOD,
  NT_PREPROC,
  NT_CALL,
  NT_BIN_OP,
  NT_UN_OP,
  NT_TYPE,
  NT_FOR,
  NT_IF,
  NT_MATCH,
  NT_USE,
  NT_INT,
  NT_FLOAT,
  NT_STRING,
  NT_BLOCK,
  NT_CONST_DECL,
  NT_PROGRAM,
  NT_PTR_TYPE,
  NT_N,
};

typedef struct mi_node_arr mi_node_arr;

typedef struct mi_node mi_node;

struct mi_node_arr {
  mi_node** ptr;
  usize count;
  usize len;
};

mi_node_arr mi_node_arr_new(usize size);

mi_node_arr mi_node_arr_resize(mi_node_arr arr, usize new_size);

void mi_node_arr_add(mi_node_arr* arr, mi_node* node);

struct mi_node {
  mi_node_type type;
  mi_pos begin;
  mi_pos end;
  union {
    struct {
      mi_node* node;
      mi_tok_type op;
    } un_op;

    struct {
      mi_node* lhs;
      mi_node* rhs;
      mi_tok_type op;
    } bin_op;

    struct {
      mi_node* fn;
      mi_node* args;
    } call;

    struct {
      mi_str name;
    } field_get;

    struct {
      mi_node* arg_types;
      mi_str* arg_names;
      mi_node* ret_type;
      mi_node* body;
    } func;

    struct {
      mi_str* path;
      mi_node* value;
    } const_decl;

    struct {
      mi_str name;
      mi_node* iterable;
      mi_node* body;
    } for_loop;

    struct {
      mi_node* stmts;
    } block;

    struct {
      mi_node* cond;
      mi_node* body;
      mi_node* otherwise;
    } if_stmt;

    struct {
      mi_str name;
    } type_spec;

    struct {
      mi_node* base;
    } ptr_type_spec;

    struct {
      mi_node* begin;
      mi_node* end;
      bool begin_inclusive;
      bool end_inclusive;
    } range;

    struct {
      mi_node* cond;
      mi_node* match_vals;
      mi_node* match_bodies;
      mi_node* otherwise;
    } match;

    struct {
      mi_node* types;
      mi_str* names;
    } class;

    mi_str* path;
    i64 int_val;
    double float_val;
    mi_str str_val;
  };
};

typedef struct mi_parser mi_parser;

struct mi_parser {
  mi_tok* last;
  mi_tok* tok;
};

#define mi_parse_if(p, ty) \
  if (p->tok->type == ty) { \
    p->tok++; \
  } else \
  assert(false && "mi_parse_if failed: expected " # ty) \

mi_node* mi_parse(mi_tok* toks);

mi_node* mi_node_dup(mi_node p);

mi_node* mi_parse_file(mi_parser* p);

mi_node* mi_parse_stmt(mi_parser* p);

mi_node* mi_parse_expr(mi_parser* p);

mi_node* mi_parse_var_decl(mi_parser* p);

mi_node* mi_parse_assign(mi_parser* p);

mi_node* mi_parse_compare(mi_parser* p);

mi_node* mi_parse_or(mi_parser* p);

mi_node* mi_parse_and(mi_parser* p);

mi_node* mi_parse_add(mi_parser* p);

mi_node* mi_parse_mul(mi_parser* p);

mi_node* mi_parse_cast(mi_parser* p);

mi_node* mi_parse_atom(mi_parser* p);

mi_node* mi_parse_call(mi_parser* p);

typedef mi_node*(*mi_parse_fn)(mi_parser* p);

mi_node* mi_parse_bin_op(mi_parser* p, mi_parse_fn lhs_fn, mi_parse_fn rhs_fn, mi_tok_type const* types, u64 n_types);