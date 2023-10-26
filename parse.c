#include <string.h>
#include "parse.h"
#include <stdbool.h>

mi_node* mi_parse_type_spec(mi_parser* p) {
  if (p->tok->type != TT_ID) {
    assert(false && "called mi_parse_type_spec w/o TT_ID as current token");
  }

  mi_pos begin = p->tok->begin;

  mi_node* type = mi_node_dup((mi_node) {
    .type = NT_TYPE,
    .type_spec = p->tok->text
  });

  p->tok++;

  while (p->tok->type == TT_MUL) {
    type = mi_node_dup((mi_node) {
      .type = NT_PTR_TYPE,
      .ptr_type_spec.base = type
    });

    p->tok++;
  }

  mi_pos end = (p->tok - 1)->end;

  type->begin = begin;
  type->end = end;

  return type;
}

mi_node* mi_parse_atom(mi_parser* p) {
  mi_tok tok = *p->tok;
  switch_no_default(tok.type) {
    case TT_PREPROC: {
      p->tok++;
      return mi_node_dup((mi_node) {
        .type = NT_PREPROC,
        .begin = tok.begin,
        .end = tok.end,
        .str_val = tok.text
      });
    }
    case TT_INT: {
      p->tok++;
      return mi_node_dup((mi_node) {
        .type = NT_INT,
        .begin = tok.begin,
        .end = tok.end,
        .str_val = tok.text
      });
    }
    case TT_FLOAT: {
      p->tok++;
      return mi_node_dup((mi_node) {
        .type = NT_FLOAT,
        .begin = tok.begin,
        .end = tok.end,
        .str_val = tok.text
      });
    }
    case TT_STRING: {
      p->tok++;
      return mi_node_dup((mi_node) {
        .type = NT_STRING,
        .begin = tok.begin,
        .end = tok.end,
        .str_val = tok.text
      });
    }
    case TT_CLASS: {
      p->tok++;

      mi_parse_if(p, TT_LCURLY);

      mi_node* types = mi_dyn_arr(mi_node, 1);
      mi_str* names = mi_dyn_arr(mi_str, 1);

      while (p->tok->type != TT_RCURLY) {
        mi_dyn_arr_add(names, &p->tok->text);
        p->tok++;
        mi_parse_if(p, TT_DOUBLE_COLON);
        mi_dyn_arr_add(types, mi_parse_type_spec(p));
        mi_parse_if(p, TT_SEMICOLON);
      }

      mi_pos end = p->tok->end;

      p->tok++;

      return mi_node_dup((mi_node) {
        .type = NT_CLASS,
        .class = {
          .names = names,
          .types = types
        },
        .begin = tok.begin,
        .end = end
      });
    }
    case TT_ID: {
      mi_parser fail = *p;

      mi_node* type_spec = mi_parse_type_spec(p);
      if (p->tok->type != TT_LPAREN) {
        *p = fail;
        free(type_spec);
        goto NotAFunctionDefinition;
      }

      p->tok++;

      /*-- test for ) { --*/ {
        mi_parser pre_test = *p;
        while (p->tok->type != TT_RPAREN) {
          if (p->tok->type == TT_LPAREN) {
            *p = fail;
            free(type_spec);
            goto NotAFunctionDefinition;
          }

          p->tok++;
        }

        mi_parse_if(p, TT_RPAREN);

        if (p->tok->type != TT_LCURLY) {
          *p = fail;
          free(type_spec);
          goto NotAFunctionDefinition;
        }

        *p = pre_test;

        mi_str* arg_names = mi_dyn_arr(mi_str, 1);
        mi_node* arg_types = mi_dyn_arr(mi_node, 1);

        if (p->tok->type != TT_RPAREN) {
          for (;;) {
            mi_dyn_arr_add(arg_types, mi_parse_type_spec(p));
            if (p->tok->type == TT_ID) {
              mi_dyn_arr_add(arg_names, &p->tok->text);
              p->tok++;
            } else {
              mi_dyn_arr_add(arg_names, &mi_str_empty);
            }

            if (p->tok->type == TT_COMMA) {
              p->tok++;
            } else if (p->tok->type == TT_RPAREN) {
              p->tok++;
              break;
            } else {
              assert(false && "expected comma or rparen in mi_parse_atom>TT_ID>");
            }
          }
        } else {
          p->tok++;
        }

        mi_parse_if(p, TT_LCURLY);
        mi_node* body = mi_dyn_arr(mi_node, 1);

        while (p->tok->type != TT_RCURLY) {
          mi_dyn_arr_add(body, mi_parse_stmt(p));
        }

        mi_pos end = p->tok->end;

        p->tok++;

        return mi_node_dup((mi_node) {
          .type = NT_FUNC,
          .func = {
            .arg_types = arg_types,
            .arg_names = arg_names,
            .ret_type = type_spec,
            .body = mi_node_dup((mi_node) {
              .type = NT_BLOCK,
              .block.stmts = body
            })
          },
          .begin = tok.begin,
          .end = end,
        });
      }

      NotAFunctionDefinition:;
      mi_str name = p->tok->text;
      p->tok++;

      return mi_node_dup((mi_node) {
        .type = NT_FIELD,
        .field_get.name = name
      });
    }
  }
}

mi_node* mi_parse(mi_tok* toks) {
  mi_parser p = (mi_parser) {
    .tok = toks,
    .last = mi_dyn_arr_last(toks),
  };

  return mi_parse_file(&p);
}

mi_node* mi_parse_file(mi_parser* p) {
  mi_node* arr = mi_dyn_arr(mi_node, 1);
  while (p->tok->type != TT_EOF) {
    mi_dyn_arr_add(arr, mi_parse_stmt(p));
  }

  return mi_node_dup((mi_node) {
    .type = NT_PROGRAM,
    .begin = mi_pos_new(mi_str_empty, 0, 0, 0),
    .end = mi_pos_new(mi_str_empty, 0, 0, 0),
    .block.stmts = arr
  });
}

mi_node_arr mi_node_arr_new(usize size) {
  return (mi_node_arr) {
    .ptr = malloc(size * sizeof(mi_node*)),
    .len = size,
    .count = 0
  };
}

mi_node_arr mi_node_arr_resize(mi_node_arr arr, usize new_size) {
  mi_node** new_ptr = realloc(arr.ptr, new_size * sizeof(mi_node*));
  if (!new_ptr) {
    assert(false && "failed to resize a mi_node_arr");
  }

  arr.ptr = new_ptr;
  return arr;
}

void mi_node_arr_add(mi_node_arr* arr, mi_node* node) {
  if (arr->len == arr->count) {
    arr->len *= 2;
    *arr = mi_node_arr_resize(*arr, arr->len);
  }

  arr->ptr[arr->count++] = node;
}

mi_node* mi_parse_stmt(mi_parser* p) {
  switch (p->tok->type) {
    case TT_USE:
    case TT_MODULE: {
      mi_tok_type type = p->tok->type;

      p->tok++;
      mi_str* path = mi_dyn_arr(mi_str, 1);
      while (p->tok->type == TT_ID) {
        mi_dyn_arr_add(path, &p->tok->text);
        p->tok++;

        if (p->tok->type != TT_DOT) {
          break;
        }

        p->tok++;
      }

      mi_parse_if(p, TT_SEMICOLON);

      return mi_node_dup((mi_node) {
        .type = type == TT_MODULE ? NT_MOD : NT_USE,
        .path = path
      });
    }
    default:;
  }

  if (p->tok->type == TT_ID) {
    mi_parser fail = *p;

    mi_str* path = mi_dyn_arr(mi_str, 1);
    mi_dyn_arr_add(path, &p->tok->text);
    p->tok++;

    while (p->tok->type == TT_DOT) {
      p->tok++;

      if (p->tok->type != TT_ID) {
        *p = fail;
        mi_dyn_arr_del(path);
        goto NotADecl;
      }

      mi_dyn_arr_add(path, &p->tok->text);
      p->tok++;
    }

    if (p->tok->type != TT_DOUBLE_COLON) {
      *p = fail;
      mi_dyn_arr_del(path);
      goto NotADecl;
    }

    p->tok++;

    mi_node* initializer = mi_parse_expr(p);

    if ((p->tok - 1)->type != TT_RCURLY) {
      mi_parse_if(p, TT_SEMICOLON);
    }

    return mi_node_dup((mi_node) {
      .type = NT_CONST_DECL,
      .const_decl = {
        .path = path,
        .value = initializer
      }
    });
  }

  NotADecl:;
  mi_node* expr = mi_parse_expr(p);
  if ((p->tok - 1)->type != TT_RCURLY && (p->tok - 1)->type != TT_PREPROC) {
    mi_parse_if(p, TT_SEMICOLON);
  }

  return expr;
}

mi_node* mi_node_dup(mi_node p) {
  mi_node* ptr = malloc(sizeof(mi_node));
  memcpy(ptr, &p, sizeof(mi_node));
  return ptr;
}

mi_node* mi_parse_var_decl(mi_parser* p) {
  static const mi_tok_type types[] = {
    TT_VAR_DECL
  };

  return mi_parse_bin_op(p, mi_parse_assign, mi_parse_assign, types, mi_len(types));
}

mi_node* mi_parse_expr(mi_parser* p) {
  return mi_parse_var_decl(p);
}

bool internal_mi_parse_contains_tok_type(mi_tok_type const* types, u64 n_types, mi_tok_type type) {
  for (int i = 0; i < n_types; i++) {
    if (*(types + i) == type) {
      return true;
    }
  }

  return false;
}

mi_node* mi_parse_bin_op(mi_parser* p, mi_parse_fn lhs_fn, mi_parse_fn rhs_fn, mi_tok_type const* types, u64 n_types) {
  mi_pos begin = p->tok->begin;
  mi_node* lhs = lhs_fn(p);

  mi_tok_type type;
  while (internal_mi_parse_contains_tok_type(types, n_types, type = p->tok->type)) {
    p->tok++;

    mi_node* rhs = rhs_fn(p);
    lhs = mi_node_dup((mi_node) {
      .type = NT_BIN_OP,
      .bin_op = {
        .lhs = lhs,
        .rhs = rhs,
        .op = type
      },
      .begin = begin,
      .end = rhs->end
    });
  }

  return lhs;
}

mi_node* mi_parse_assign(mi_parser* p) {
  static const mi_tok_type types[] = {
    TT_EQ
  };

  return mi_parse_bin_op(p, mi_parse_or, mi_parse_or, types, mi_len(types));
}

mi_node* mi_parse_or(mi_parser* p) {
  static const mi_tok_type types[] = {
    TT_PIPE
  };

  return mi_parse_bin_op(p, mi_parse_and, mi_parse_and, types, mi_len(types));
}

mi_node* mi_parse_and(mi_parser* p) {
  static const mi_tok_type types[] = {
    TT_AMPERSAND
  };

  return mi_parse_bin_op(p, mi_parse_compare, mi_parse_compare, types, mi_len(types));
}

mi_node* mi_parse_compare(mi_parser* p) {
  if (p->tok->type == TT_EXCLAMATION) {
    mi_pos begin = p->tok->begin;
    p->tok++;

    mi_node* res = mi_parse_compare(p);
    mi_pos end = res->end;
    return mi_node_dup((mi_node) {
      .type = NT_UN_OP,
      .un_op = {
        .node = res,
        .op = TT_EXCLAMATION
      },
      .begin = begin,
      .end = end
    });
  }

  static const mi_tok_type types[] = {
    TT_LT,
    TT_LTE,
    TT_GT,
    TT_GTE,
    TT_NEQ,
    TT_DOUBLE_EQ
  };

  return mi_parse_bin_op(p, mi_parse_add, mi_parse_add, types, mi_len(types));
}

mi_node* mi_parse_add(mi_parser* p) {
  static const mi_tok_type types[] = {
    TT_ADD,
    TT_SUB
  };

  return mi_parse_bin_op(p, mi_parse_mul, mi_parse_mul, types, mi_len(types));
}

mi_node* mi_parse_mul(mi_parser* p) {
  static const mi_tok_type types[] = {
    TT_MUL,
    TT_DIV,
    TT_MOD
  };

  return mi_parse_bin_op(p, mi_parse_cast, mi_parse_cast, types, mi_len(types));
}

mi_node* mi_parse_cast(mi_parser* p) {
  static const mi_tok_type types[] = {
    TT_CAST
  };

  return mi_parse_bin_op(p, mi_parse_call, mi_parse_type_spec, types, mi_len(types));
}

mi_node* mi_parse_call(mi_parser* p) {
  mi_node* lhs = mi_parse_atom(p);

  while (p->tok->type == TT_LPAREN) {
    p->tok++;
    mi_node* args = mi_dyn_arr(mi_node, 1);
    while (p->tok->type != TT_RPAREN) {
      mi_dyn_arr_add(args, mi_parse_atom(p));

      if (p->tok->type == TT_COMMA) {
        p->tok++;
      } else if (p->tok->type != TT_RPAREN) {
        assert(false && "expected '(' or ','");
      }
    }

    p->tok++;

    lhs = mi_node_dup((mi_node) {
      .type = NT_CALL,
      .call = {
        .fn = lhs,
        .args = args
      }
    });
  }

  return lhs;
}