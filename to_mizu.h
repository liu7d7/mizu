#pragma once

#include <stdio.h>
#include "parse.h"

c_str mi_op_mizu_str(mi_tok_type type);

typedef struct mi_to_mizu_next_line_opts mi_to_mizu_next_line_opts;

struct mi_to_mizu_next_line_opts {
  bool semicolon;
};

mi_to_mizu_next_line_opts mi_to_mizu_next_line_opts_new(bool semicolon);

mi_to_mizu_next_line_opts mi_to_mizu(FILE* out, mi_node* ast, int indent_level);