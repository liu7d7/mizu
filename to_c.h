#pragma once

#include <stdio.h>
#include <string.h>
#include "parse.h"

typedef enum mi_to_c_context_type mi_to_c_context_type;

enum mi_to_c_context_type {
  C_CTX_PROGRAM,
  C_CTX_FUNCTION
};

typedef enum mi_decl_type mi_decl_type;

enum mi_decl_type {
  DECL_FUNC,
  DECL_CLASS,
  DECL_VAR
};

typedef struct mi_decl mi_decl;

struct mi_decl {
  mi_str name;
  mi_decl_type type;

  union {
    struct {
      mi_str return_type;
    } func;

    struct {
      mi_str* fields;
      mi_str* types;
    } class;

    struct {
      mi_str type;
    } var;
  };
};

typedef struct mi_module mi_module;

struct mi_module {
  mi_str name;
  mi_decl* decls;
};

typedef struct mi_to_c_context mi_to_c_context;

struct mi_to_c_context {
  mi_to_c_context* parent;
  mi_module* modules;
  mi_module* cur_module;
  mi_to_c_context_type type;
  int indent_level;
};

typedef struct mi_to_c_next_line_opts mi_to_c_next_line_opts;

struct mi_to_c_next_line_opts {
  bool semicolon;
  mi_to_c_context next_ctx;
};

mi_to_c_next_line_opts mi_to_c_next_line_opts_new(bool semicolon, mi_to_c_context ctx) {
  return (mi_to_c_next_line_opts) {
    .semicolon = semicolon,
    .next_ctx = ctx
  };
}

const char* const mi_sep = "__";
usize const mi_sep_len = 2;

c_str mi_op_c_str(mi_tok_type type) {
  switch_no_default(type) {
    case TT_ADD: return "+";
    case TT_SUB: return "-";
    case TT_MUL: return "*";
    case TT_DIV: return "/";
    case TT_MOD: return "%";
    case TT_INC: return "+=";
    case TT_DEC: return "-=";
    case TT_DOUBLE_ADD: return "++";
    case TT_DOUBLE_SUB: return "--";
    case TT_EQ: return "=";
    case TT_DOUBLE_EQ: return "==";
    case TT_NEQ: return "!=";
    case TT_RANGE: return "..";
    case TT_CAST: return ":>";
    case TT_LT: return "<";
    case TT_LTE: return "<=";
    case TT_GT: return ">";
    case TT_GTE: return ">=";
    case TT_PIPE: return "|";
    case TT_AMPERSAND: return "&";
    case TT_EXCLAMATION: return "!";
  }
}

mi_str mi_path_to_c_id(mi_str* path) {
  usize end_size = 0;
  for_each(it, path) {
    end_size += it->len;

    if (it != mi_dyn_arr_last(path)) {
      end_size += 2;
    }
  }

  str out = str_new(end_size);
  usize off = 0;
  for_each(it, path) {
    memcpy(out + off, it->ptr, it->len);
    off += it->len;

    if (it != mi_dyn_arr_last(path)) {
      memcpy(out + off, mi_sep, mi_sep_len);
      off += mi_sep_len;
    }
  }

  return mi_str_new_size(out, end_size);
}

mi_str mi_type_to_c_str(mi_node* ast) {
  usize num_ptrs = 0;
  while (ast->type == NT_PTR_TYPE) {
    num_ptrs++;
    ast = ast->ptr_type_spec.base;
  }

  str out = str_new(ast->type_spec.name.len + num_ptrs);
  memcpy(out, ast->type_spec.name.ptr, ast->type_spec.name.len);
  for (int i = ast->type_spec.name.len; i < ast->type_spec.name.len + num_ptrs; i++) {
    out[i] = '*';
  }

  return mi_str_new_size(out, ast->type_spec.name.len + num_ptrs);
}

mi_str mi_infer_type(mi_node* ast, mi_to_c_context ctx) {
  switch_no_default(ast->type) {
    case NT_STRING: return mi_str_new("char const*");
    case NT_INT: return mi_str_new("int");
    case NT_FLOAT: return mi_str_new("float");
    case NT_BIN_OP: {
      switch (ast->bin_op.op) {
        case TT_CAST: {
          return mi_type_to_c_str(ast->bin_op.rhs);
        }
      }
    }
  }
}

mi_to_c_next_line_opts mi_to_c(FILE* out, mi_node* ast, mi_to_c_context ctx) {
  switch_no_default(ast->type) {
    case NT_PREPROC: {
      if (ctx.type != C_CTX_PROGRAM && mi_str_starts_with_c(ast->str_val, "#include")) {
        assert(false && "cannot put #include statement inside anything other than program context");
      }

      fprintf(out, "%.*s", (int) ast->str_val.len, ast->str_val.ptr);

      return mi_to_c_next_line_opts_new(false, ctx);
    }
    case NT_INT: {
      fprintf(out, "%.*s", (int) ast->str_val.len, ast->str_val.ptr);

      return mi_to_c_next_line_opts_new(true, ctx);
    }
    case NT_STRING: {
      fprintf(out, "\"%.*s\"", (int) ast->str_val.len, ast->str_val.ptr);

      return mi_to_c_next_line_opts_new(true, ctx);
    }
    case NT_PROGRAM: {
      for_each(it, ast->block.stmts) {
        mi_to_c_next_line_opts opts = mi_to_c(out, it, ctx);
        ctx = opts.next_ctx;
        if (opts.semicolon) {
          fprintf(out, ";");
        }

        fprintf(out, "\n");
      }

      return mi_to_c_next_line_opts_new(false, ctx);
    }
    case NT_BLOCK: {
      for_each(it, ast->block.stmts) {
        mi_indent(out, ctx.indent_level);
        mi_to_c_next_line_opts opts = mi_to_c(out, it, ctx);
        ctx = opts.next_ctx;
        if (opts.semicolon) {
          fprintf(out, ";");
        }

        fprintf(out, "\n");
      }

      return mi_to_c_next_line_opts_new(false, ctx);
    }
    case NT_MOD: {
      mi_to_c_context next_ctx = ctx;
      next_ctx.cur_module = malloc(sizeof(mi_module));
      next_ctx.cur_module->name = mi_path_to_c_id(ast->path);
      next_ctx.cur_module->decls = mi_dyn_arr(mi_decl, 1);

      // is this module already added?
      bool found = false;
      for_each(it, next_ctx.modules) {
        if (mi_str_eq(it->name, next_ctx.cur_module->name)) {
          found = true;
          break;
        }
      }

      if (!found) {
        mi_dyn_arr_add(next_ctx.modules, next_ctx.cur_module);
      }

      return mi_to_c_next_line_opts_new(false, next_ctx);
    }
    case NT_USE: {
      // TODO: figure out importing
      return mi_to_c_next_line_opts_new(false, ctx);
    }
    case NT_BIN_OP: {
      if (ast->bin_op.op == TT_VAR_DECL) {
        mi_str type = mi_infer_type(ast->bin_op.rhs, ctx);
        fprintf(out, "%.*s ", (int) type.len, type.ptr);
        fprintf(out, "%.*s = ", (int) ast->bin_op.lhs->field_get.name.len, ast->bin_op.lhs->field_get.name.ptr);
        mi_to_c(out, ast->bin_op.rhs, ctx);
        return mi_to_c_next_line_opts_new(true, ctx);
      } else if (ast->bin_op.op == TT_CAST) {
        fprintf(out, "((");
        mi_to_c(out, ast->bin_op.rhs, ctx);
        fprintf(out, ") ");
        mi_to_c(out, ast->bin_op.lhs, ctx);
        fprintf(out, ")");

        return mi_to_c_next_line_opts_new(true, ctx);
      }

      mi_to_c(out, ast->bin_op.lhs, ctx);
      fprintf(out, " %s ", mi_op_c_str(ast->bin_op.op));
      mi_to_c(out, ast->bin_op.rhs, ctx);

      return mi_to_c_next_line_opts_new(true, ctx);
    }
    case NT_UN_OP: {
      fprintf(out, "%s", mi_op_c_str(ast->un_op.op));
      mi_to_c(out, ast->un_op.node, ctx);

      return mi_to_c_next_line_opts_new(true, ctx);
    }
    case NT_CONST_DECL: {
      mi_str* path = ast->const_decl.path;
      usize path_len = mi_dyn_arr_len(path);
      mi_node* value = ast->const_decl.value;

      switch (value->type) {
        case NT_FUNC: {
          if (path_len == 1) {
            mi_str id;

            if (ctx.cur_module->name.len != 0) {
              str tmp_id = str_new(ctx.cur_module->name.len + mi_sep_len + path->len);
              memcpy(tmp_id, ctx.cur_module->name.ptr, ctx.cur_module->name.len);
              memcpy(tmp_id + ctx.cur_module->name.len, mi_sep, mi_sep_len);
              memcpy(tmp_id + ctx.cur_module->name.len + mi_sep_len, path->ptr, path->len);
              id = mi_str_new_size(tmp_id, ctx.cur_module->name.len + mi_sep_len + path->len);
            } else {
              id = *path;
            }

            mi_to_c(out, value->func.ret_type, ctx);
            fprintf(out, " %.*s(", (int) id.len, id.ptr);

            for_each_zip2(it, value->func.arg_types, value->func.arg_names) {
              mi_to_c(out, it.a, ctx);
              fprintf(out, " %.*s", (int) it.b->len, it.b->ptr);

              if (!it.last) {
                fprintf(out, ", ");
              }
            }

            fprintf(out, ") {\n");

            mi_dyn_arr_add(ctx.cur_module->decls, &(mi_decl) {
              .name = id,
              .type = DECL_FUNC,
              .func = {
                .return_type = mi_type_to_c_str(ast->func.ret_type)
              }
            });

            mi_to_c_context next_ctx = ctx;
            next_ctx.indent_level++;
            mi_to_c(out, value->func.body, next_ctx);

            mi_indent(out, ctx.indent_level);
            fprintf(out, "}");

            return mi_to_c_next_line_opts_new(false, ctx);
          } else if (path_len == 2) {

          }

          assert(false && "more than one member in function definition");
        }
        case NT_CLASS: {
          if (path_len != 1) {
            assert(false && "cannot define struct as a member");
          }

          // make sure to predeclare it
        }
        default:; // fall through
      }

      // just a normal const variable
      mi_str type = mi_infer_type(value, ctx);

      fprintf(
        out,
        "%.*s const %.*s = ",
        (int) type.len,
        type.ptr,
        (int) path->len,
        path->ptr
      );

      mi_to_c(out, ast->const_decl.value, ctx);
      return mi_to_c_next_line_opts_new(true, ctx);
    }
    case NT_TYPE: {
      fprintf(out, "%.*s", (int) ast->type_spec.name.len, ast->type_spec.name.ptr);

      return mi_to_c_next_line_opts_new(true, ctx);
    }
    case NT_PTR_TYPE: {
      mi_to_c(out, ast->ptr_type_spec.base, ctx);
      fprintf(out, "*");

      return mi_to_c_next_line_opts_new(true, ctx);
    }
  }
}