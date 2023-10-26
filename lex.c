#include <string.h>
#include "lex.h"
#include "dyn_arr.h"

c_str mi_tok_type_str(mi_tok_type type) {
  c_str types[TT_N] = {
    "TT_MODULE",
    "TT_CLASS",
    "TT_RET",
    "TT_FOR",
    "TT_IF",
    "TT_ELSE",
    "TT_MATCH",
    "TT_USE",
    "TT_DO",
    "TT_N_KEYWORDS",
    "TT_DOUBLE_COLON",
    "TT_COLON",
    "TT_SEMICOLON",
    "TT_LCURLY",
    "TT_RCURLY",
    "TT_DOT",
    "TT_COMMA",
    "TT_LBRACKET",
    "TT_RBRACKET",
    "TT_LPAREN",
    "TT_RPAREN",
    "TT_ADD",
    "TT_SUB",
    "TT_MUL",
    "TT_DIV",
    "TT_MOD",
    "TT_INC",
    "TT_DEC",
    "TT_DOUBLE_ADD",
    "TT_DOUBLE_SUB",
    "TT_VAR_DECL",
    "TT_EQ",
    "TT_DOUBLE_EQ",
    "TT_NEQ",
    "TT_RANGE",
    "TT_CAST",
    "TT_LT",
    "TT_LTE",
    "TT_GT",
    "TT_GTE",
    "TT_PIPE",
    "TT_AMPERSAND",
    "TT_EXCLAMATION",
    "TT_STRING",
    "TT_INT",
    "TT_HEX",
    "TT_FLOAT",
    "TT_ID",
    "TT_PREPROC",
    "TT_EOF",
  };

  return types[type];
}

str mi_tok_str(mi_tok tok) {
  return mi_sprintf(
    "mi_tok{type=%s, text=\"%.*s\", begin=%s end=%s}",
    mi_tok_type_str(tok.type),
    (int) tok.text.len,
    tok.text.ptr,
    mi_pos_str(tok.begin),
    mi_pos_str(tok.end)
  );
}

bool mi_lex_at_end(mi_lexer *s) {
  return s->pos.idx >= s->text.len;
}

char mi_lex_peek(mi_lexer *s) {
  return s->text.ptr[s->pos.idx];
}

void mi_lex_adv(mi_lexer *s) {
  mi_pos_adv(&s->pos, s->text);
}

void mi_lex_add(mi_lexer *s, mi_tok tok) {
  mi_dyn_arr_add(s->toks, &tok);
}

bool mi_lex_is_id_begin(char c) {
  return isalpha(c) || c == '_';
}

bool mi_lex_is_id_continue(char c) {
  return isalnum(c) || c == '_';
}

mi_tok* mi_lex(c_str path) {
  mi_lexer s = (mi_lexer) {
    .text = mi_read_file(path),
    .pos = mi_pos_new(mi_str_new(strdup(path)), 0, 1, 1),
    .toks = mi_dyn_arr(mi_tok, 1)
  };

  mi_str keywords[TT_N_KEYWORDS] = {
    [TT_MODULE] = mi_str_new("mod"),
    [TT_CLASS] = mi_str_new("class"),
    [TT_RET] = mi_str_new("ret"),
    [TT_FOR] = mi_str_new("for"),
    [TT_IF] = mi_str_new("if"),
    [TT_ELSE] = mi_str_new("else"),
    [TT_MATCH] = mi_str_new("match"),
    [TT_DO] = mi_str_new("do"),
    [TT_USE] = mi_str_new("use"),
  };

  while (!mi_lex_at_end(&s)) {
    mi_pos begin = s.pos;
    switch (mi_lex_peek(&s)) {
      case '#': {
        while (mi_lex_peek(&s) != '\n' && mi_lex_peek(&s) != '\r') {
          mi_lex_adv(&s);
        }
        mi_lex_add(&s, mi_tok_new(TT_PREPROC, mi_dup_slice(s.text, begin.idx, s.pos.idx), begin, s.pos));
        break;
      }
      case '\r':
      case '\n':
      case '\t':
      case ' ': {
        mi_lex_adv(&s);
        break;
      }
      case '>': {
        mi_lex_adv(&s);
        if (mi_lex_peek(&s) == '=') {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_LTE, mi_str_empty, begin, s.pos));
        } else {
          mi_lex_add(&s, mi_tok_new(TT_LT, mi_str_empty, begin, s.pos));
        }
        break;
      }
      case '<': {
        mi_lex_adv(&s);
        if (mi_lex_peek(&s) == '=') {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_GTE, mi_str_empty, begin, s.pos));
        } else {
          mi_lex_add(&s, mi_tok_new(TT_GT, mi_str_empty, begin, s.pos));
        }
        break;
      }
      case ':': {
        mi_lex_adv(&s);
        if (mi_lex_peek(&s) == '=') {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_VAR_DECL, mi_str_empty, begin, s.pos));
        } else if (mi_lex_peek(&s) == '>') {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_CAST, mi_str_empty, begin, s.pos));
        } else if (mi_lex_peek(&s) == ':') {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_DOUBLE_COLON, mi_str_empty, begin, s.pos));
        } else {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_COLON, mi_str_empty, begin, s.pos));
        }
        break;
      }
      case '|': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_PIPE, mi_str_empty, begin, s.pos));
        break;
      }
      case '&': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_AMPERSAND, mi_str_empty, begin, s.pos));
        break;
      }
      case '{': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_LCURLY, mi_str_empty, begin, s.pos));
        break;
      }
      case '}': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_RCURLY, mi_str_empty, begin, s.pos));
        break;
      }
      case '(': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_LPAREN, mi_str_empty, begin, s.pos));
        break;
      }
      case ')': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_RPAREN, mi_str_empty, begin, s.pos));
        break;
      }
      case '[': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_LBRACKET, mi_str_empty, begin, s.pos));
        break;
      }
      case ']': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_RBRACKET, mi_str_empty, begin, s.pos));
        break;
      }
      case ',': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_COMMA, mi_str_empty, begin, s.pos));
        break;
      }
      case '.': {
        mi_lex_adv(&s);
        if (mi_lex_peek(&s) == '.') {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_RANGE, mi_str_empty, begin, s.pos));
        } else {
          mi_lex_add(&s, mi_tok_new(TT_DOT, mi_str_empty, begin, s.pos));
        }
        break;
      }
      case ';': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_SEMICOLON, mi_str_empty, begin, s.pos));
        break;
      }
      case '+': {
        mi_lex_adv(&s);
        if (mi_lex_peek(&s) == '+') {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_DOUBLE_ADD, mi_str_empty, begin, s.pos));
        } else {
          mi_lex_add(&s, mi_tok_new(TT_ADD, mi_str_empty, begin, s.pos));
        }
        break;
      }
      case '-': {
        mi_lex_adv(&s);
        if (mi_lex_peek(&s) == '-') {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_DOUBLE_SUB, mi_str_empty, begin, s.pos));
        } else {
          mi_lex_add(&s, mi_tok_new(TT_SUB, mi_str_empty, begin, s.pos));
        }
        break;
      }
      case '*': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_MUL, mi_str_empty, begin, s.pos));
        break;
      }
      case '/': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_DIV, mi_str_empty, begin, s.pos));
        break;
      }
      case '%': {
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_MOD, mi_str_empty, begin, s.pos));
        break;
      }
      case '=': {
        mi_lex_adv(&s);
        if (mi_lex_peek(&s) == '=') {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_DOUBLE_EQ, mi_str_empty, begin, s.pos));
        } else {
          mi_lex_add(&s, mi_tok_new(TT_EQ, mi_str_empty, begin, s.pos));
        }
        break;
      }
      case '!': {
        mi_lex_adv(&s);
        if (mi_lex_peek(&s) == '=') {
          mi_lex_adv(&s);
          mi_lex_add(&s, mi_tok_new(TT_NEQ, mi_str_empty, begin, s.pos));
        } else {
          mi_lex_add(&s, mi_tok_new(TT_EXCLAMATION, mi_str_empty, begin, s.pos));
        }
        break;
      }
      case '"': {
        mi_lex_adv(&s);
        usize start = s.pos.idx;
        while (mi_lex_peek(&s) != '"') {
          mi_lex_adv(&s);
        }
        usize finish = s.pos.idx;
        mi_lex_adv(&s);
        mi_lex_add(&s, mi_tok_new(TT_STRING, mi_slice(s.text, start, finish), begin, s.pos));
        break;
      }
      default: {
        if (isdigit(mi_lex_peek(&s))) {
          usize start = s.pos.idx;
          while (isdigit(mi_lex_peek(&s))) {
            mi_lex_adv(&s);
          }
          usize finish = s.pos.idx;
          mi_lex_add(&s, mi_tok_new(TT_INT, mi_dup_slice(s.text, start, finish), begin, s.pos));
        } else if (mi_lex_is_id_begin(mi_lex_peek(&s))) {
          usize start = s.pos.idx;
          while (mi_lex_is_id_continue(mi_lex_peek(&s))) {
            mi_lex_adv(&s);
          }
          usize finish = s.pos.idx;
          mi_str text = mi_dup_slice(s.text, start, finish);

          for (int i = 0; i < TT_N_KEYWORDS; i++) {
            if (mi_str_eq(text, keywords[i])) {
              mi_lex_add(&s, mi_tok_new(i, mi_str_empty, begin, s.pos));
              mi_str_del(&text);
              goto End;
            }
          }

          mi_lex_add(&s, mi_tok_new(TT_ID, text, begin, s.pos));
          End:
        }
      }
    }
  }

  mi_lex_add(&s, mi_tok_new(TT_EOF, mi_str_empty, s.pos, s.pos));

  free(s.text.ptr);
  return s.toks;
}

mi_tok mi_tok_new(mi_tok_type type, mi_str text, mi_pos begin, mi_pos end) {
  return (mi_tok) {
    .type = type,
    .text = text,
    .begin = begin,
    .end = end
  };
}
