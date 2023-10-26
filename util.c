#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "util.h"

void mi_str_del(mi_str *in) {
  free(in->ptr);
  in->len = 0;
}

mi_str mi_suffix(mi_str in, usize begin) {
  assert(in.len > begin);
  assert(begin >= 0);

  return (mi_str) {
    .len = in.len - begin,
    .ptr = in.ptr + begin
  };
}

mi_str mi_prefix(mi_str in, usize end) {
  assert(end <= in.len);

  return (mi_str) {
    .len = end,
    .ptr = in.ptr
  };
}

mi_str mi_slice(mi_str in, usize begin, usize end) {
  assert(begin >= 0);
  assert(end <= in.len);

  return (mi_str) {
    .len = end - begin,
    .ptr = in.ptr + begin
  };
}

mi_str mi_dup_slice(mi_str in, usize begin, usize end) {
  assert(begin >= 0);
  assert(end <= in.len);

  str out = str_new(end - begin);
  memcpy(out, in.ptr + begin, end - begin);

  return (mi_str) {
    .len = end - begin,
    .ptr = out
  };
}

mi_str mi_read_file(c_str path) {
  FILE *file = fopen(path, "r");

  if (file) {
    fseek(file, 0, SEEK_END);
    usize length = ftell(file);
    fseek(file, 0, SEEK_SET);
    str buffer = str_new(length);
    if (buffer) {
      fread(buffer, 1, length, file);
    }
    fclose(file);

    return (mi_str) {
      .len = length,
      .ptr = buffer
    };
  }

  assert(false && "failed to read file!");
}

c_str mi_c_str(mi_str in) {
  str out = str_new(in.len + 1);
  memcpy(out, in.ptr, in.len);
  out[in.len] = '\0';
  return out;
}

c_str c_str_new(usize size) {
  return malloc(size);
}

str str_new(usize size) {
  return malloc(size);
}

mi_pos mi_pos_new(mi_str path, usize idx, usize col, usize row) {
  return (mi_pos) {
    .path = path,
    .col = col,
    .row = row,
    .idx = idx
  };
}

void mi_pos_adv(mi_pos *in, mi_str file) {
  if (in->idx >= file.len - 1) {
    in->idx = UINT64_MAX;
    return;
  }

  if (file.ptr[in->idx] == '\n') {
    in->col = 1;
    in->row++;
  } else {
    in->col++;
  }

  in->idx++;
}

mi_str mi_str_new(str in) {
  return (mi_str) {
    .len = strlen(in),
    .ptr = in
  };
}

mi_str mi_str_new_size(str in, usize size) {
  return (mi_str) {
    .len = size,
    .ptr = in
  };
}

static const mi_str mi_str_empty = (mi_str) {
  .len = 0,
  .ptr = NULL
};

bool mi_str_eq(mi_str a, mi_str b) {
  if (a.len != b.len) {
    return false;
  }

  for (usize i = 0; i < a.len; i++) {
    if (a.ptr[i] != b.ptr[i]) {
      return false;
    }
  }

  return true;
}

str mi_sprintf(c_str fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buf[256];
  vsnprintf(buf, sizeof(buf), fmt, args);
  return strdup(buf);
}

str mi_pos_str(mi_pos in) {
  return mi_sprintf("%.*s:%d:%d", (int) in.path.len, in.path.ptr, in.row, in.col);
}

mi_str_arr mi_str_arr_new(usize size) {
  return (mi_str_arr) {
    .ptr = malloc(size * sizeof(mi_str)),
    .len = size,
    .count = 0
  };
}

void mi_str_arr_del(mi_str_arr* arr) {
  free(arr->ptr);
  arr->len = 0;
  arr->count = 0;
}

void mi_str_arr_resize(mi_str_arr *arr, usize new_size) {
  mi_str *new_ptr = realloc(arr->ptr, new_size * sizeof(mi_str));
  if (!new_ptr) {
    assert(false && "failed to resize a mi_str_arr");
  }

  arr->ptr = new_ptr;
}

void mi_str_arr_add(mi_str_arr *arr, mi_str str) {
  if (arr->len == arr->count) {
    arr->len *= 2;
    mi_str_arr_resize(arr, arr->len);
  }

  arr->ptr[arr->count++] = str;
}

void mi_indent(FILE* out, int indent_level) {
  for (int i = 0; i < indent_level; i++) {
    fprintf(out, "  ");
  }
}

bool mi_str_starts_with(mi_str this, mi_str path) {
  if (path.len > this.len) {
    return false;
  }

  for (int i = 0; i < path.len; i++) {
    if (this.ptr[i] != path.ptr[i]) {
      return false;
    }
  }

  return true;
}

bool mi_str_starts_with_c(mi_str this, c_str path) {
  size_t len = strlen(path);

  if (len > this.len) {
    return false;
  }

  for (int i = 0; i < len; i++) {
    if (this.ptr[i] != path[i]) {
      return false;
    }
  }

  return true;
}

mi_str mi_str_add(mi_str a, mi_str b) {
  u64 new_size = a.len + b.len;
  str result = str_new(new_size);
  memcpy(result, a.ptr, a.len);
  memcpy(result + a.len, b.ptr, b.len);
  return mi_str_new(result);
}
