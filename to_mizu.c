#include "to_mizu.h"

c_str mi_op_mizu_str(mi_tok_type type) {
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
    case TT_VAR_DECL: return ":=";
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

mi_to_mizu_next_line_opts mi_to_mizu_next_line_opts_new(bool semicolon) {
  return (mi_to_mizu_next_line_opts) {
    .semicolon = semicolon
  };
}

mi_to_mizu_next_line_opts mi_to_mizu(FILE* out, mi_node* ast, int indent_level) {
  switch_no_default(ast->type) {
    case NT_CALL: {
      mi_to_mizu(out, ast->call.fn, indent_level);
      fprintf(out, "(");
      for_each(it, ast->call.args) {
        mi_to_mizu()
      }
    }
        case NT_PREPROC: {
          mi_indent(out, indent_level);
          fprintf(out, "%.*s", (int) ast->str_val.len, ast->str_val.ptr);

          return mi_to_mizu_next_line_opts_new(false);
        }
        case NT_MOD: {
          mi_indent(out, indent_level);
          fprintf(out, "cur_module ");
          for_each(it, ast->path) {
            fprintf(out, "%.*s", (int) it->len, it->ptr);

            if (it != mi_dyn_arr_last(ast->path)) {
              fprintf(out, ".");
            }
          }

          return mi_to_mizu_next_line_opts_new(true);
        }
        case NT_USE: {
          mi_indent(out, indent_level);
          fprintf(out, "use ");
          for_each(it, ast->path) {
            fprintf(out, "%.*s", (int) it->len, it->ptr);

            if (it != mi_dyn_arr_last(ast->path)) {
              fprintf(out, ".");
            }
          }

          return mi_to_mizu_next_line_opts_new(true);
        }
        case NT_PROGRAM: {
          for_each(it, ast->block.stmts) {
            mi_to_mizu_next_line_opts opts = mi_to_mizu(out, it, indent_level);
            if (opts.semicolon) {
              fprintf(out, ";");
            }

            fprintf(out, "\n");
          }

          return mi_to_mizu_next_line_opts_new(false);
        }
        case NT_BLOCK: {
          for_each(it, ast->block.stmts) {
            mi_indent(out, indent_level);
            mi_to_mizu_next_line_opts opts = mi_to_mizu(out, it, indent_level);
            if (opts.semicolon) {
              fprintf(out, ";");
            }

            fprintf(out, "\n");
          }

          return mi_to_mizu_next_line_opts_new(false);
        }
        case NT_CONST_DECL: {
          for_each(it, ast->const_decl.path) {
            fprintf(out, "%.*s", (int) it->len, it->ptr);

            if (it != mi_dyn_arr_last(ast->path)) {
              fprintf(out, ".");
            }
          }

          fprintf(out, " :: ");

          mi_to_mizu(out, ast->const_decl.value, indent_level);

          return mi_to_mizu_next_line_opts_new(
            ast->const_decl.value->type != NT_CLASS && ast->const_decl.value->type != NT_FUNC);
        }
        case NT_INT: {
          fprintf(out, "%.*s", (int) ast->str_val.len, ast->str_val.ptr);

          return mi_to_mizu_next_line_opts_new(true);
        }
        case NT_STRING: {
          fprintf(out, "\"%.*s\"", (int) ast->str_val.len, ast->str_val.ptr);

          return mi_to_mizu_next_line_opts_new(true);
        }
        case NT_BIN_OP: {
          mi_to_mizu(out, ast->bin_op.lhs, indent_level);
          fprintf(out, " %s ", mi_op_mizu_str(ast->bin_op.op));
          mi_to_mizu(out, ast->bin_op.rhs, indent_level);

          return mi_to_mizu_next_line_opts_new(true);
        }
        case NT_UN_OP: {
          fprintf(out, "%s", mi_op_mizu_str(ast->un_op.op));
          mi_to_mizu(out, ast->un_op.node, indent_level);

          return mi_to_mizu_next_line_opts_new(true);
        }
        case NT_FUNC: {
          mi_to_mizu(out, ast->func.ret_type, indent_level);
          fprintf(out, "(");
          for_each_zip2(it, ast->func.arg_types, ast->func.arg_names) {
            mi_to_mizu(out, it.a, indent_level);
            fprintf(out, " %.*s", (int) it.b->len, it.b->ptr);

            if (!it.last) {
              fprintf(out, ", ");
            }
          }
          fprintf(out, ") {\n");

          mi_to_mizu(out, ast->func.body, indent_level + 1);

          mi_indent(out, indent_level);
          fprintf(out, "}");

          return mi_to_mizu_next_line_opts_new(false);
        }
        case NT_FIELD: {
          fprintf(out, "%.*s", (int) ast->field_get.name.len, ast->field_get.name.ptr);

          return mi_to_mizu_next_line_opts_new(true);
        }
        case NT_TYPE: {
          fprintf(out, "%.*s", (int) ast->type_spec.name.len, ast->type_spec.name.ptr);

          return mi_to_mizu_next_line_opts_new(true);
        }
        case NT_PTR_TYPE: {
          mi_to_mizu(out, ast->ptr_type_spec.base, indent_level);
          fprintf(out, "*");

          return mi_to_mizu_next_line_opts_new(true);
        }
        case NT_CLASS: {
          fprintf(out, "class {\n");
          for_each_zip2(it, ast->class.names, ast->class.types) {
            mi_indent(out, indent_level + 1);
            fprintf(out, "%.*s :: ", (int) it.a->len, it.a->ptr);
            mi_to_mizu(out, it.b, indent_level + 1);
            fprintf(out, ";\n");
          }
          mi_indent(out, indent_level);
          fprintf(out, "}");

          return mi_to_mizu_next_line_opts_new(false);
        }
      }
}
