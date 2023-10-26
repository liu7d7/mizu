#include <stdio.h>
#include "util.h"
#include "lex.h"
#include "parse.h"
#include "to_mizu.h"
#include "dyn_arr.h"
#include "to_c.h"

int main() {
  mi_tok* toks = mi_lex("mod.mizu");
  for_each(tok, toks) {
    printf("%s\n", mi_tok_str(*tok));
  }
  mi_node* ast = mi_parse(toks);
  mi_to_mizu(stdout, ast, 0);
  printf("\n\n");
  mi_module* modules = mi_dyn_arr(mi_module, 1);
  mi_dyn_arr_add(modules, &(mi_module) {
    .name = "",
    .decls = mi_dyn_arr(mi_decl, 1)
  });
  mi_to_c(stdout, ast, (mi_to_c_context) { .indent_level = 0, .cur_module = &modules[0], .type = C_CTX_PROGRAM, .modules = modules });
  return 0;
}