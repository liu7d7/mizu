#pragma once

#include <stdint.h>
#include <stddef.h>
#include <malloc.h>
#include <assert.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef int8_t i8;
typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;
typedef size_t usize;
typedef const char* c_str;
typedef char* str;

#define for_each(name, arr) for (typeof(arr) name = arr; name < arr + mi_dyn_arr_count(arr); name++)
#define for_each_zip2(name, arr1, arr2) { assert(mi_dyn_arr_count(arr1) == mi_dyn_arr_count(arr2) && "incompatible lengths in for_each_zip2"); } for (struct { typeof(arr1) a; typeof(arr2) b; bool last; } name = { .a = arr1, .b = arr2, .last = false }; name.a < arr1 + mi_dyn_arr_count(arr1); name.a++, name.b++, name.last = (mi_dyn_arr_last(arr1) == name.a))
#define for_each_zip3(name, arr1, arr2, arr3) for (struct { typeof(arr1) a; typeof(arr2) b; typeof(arr3) c } name = { .a = arr1, .b = arr2, .c = arr3 }; name.a < arr1 + mi_dyn_arr_count(arr1); name.a++, name.b++, name.c++)

#define auto(name, ...) typeof(__VA_ARGS__) name = __VA_ARGS__;

#define mi_len(arr) (sizeof(arr) / sizeof(arr[0]))

#define switch_no_default(...) \
  switch (__VA_ARGS__)         \
    default:                   \
      if (true) assert(false && "unhandled switch case"); \
      else \

typedef struct mi_str mi_str;

struct mi_str {
  str ptr;
  usize len;
};

typedef struct mi_str_arr mi_str_arr;

struct mi_str_arr {
  mi_str* ptr;
  usize len;
  usize count;
};

void mi_indent(FILE* out, int indent_level);

mi_str_arr mi_str_arr_new(usize size);

void mi_str_arr_del(mi_str_arr* arr);

void mi_str_arr_resize(mi_str_arr* arr, usize new_size);

void mi_str_arr_add(mi_str_arr* arr, mi_str str);

void mi_str_del(mi_str* in);

static const mi_str mi_str_empty;

mi_str mi_suffix(mi_str in, usize begin);

mi_str mi_prefix(mi_str in, usize end);

mi_str mi_str_new_size(str in, usize size);

mi_str mi_slice(mi_str in, usize begin, usize end);

mi_str mi_dup_slice(mi_str in, usize begin, usize end);

mi_str mi_read_file(c_str path);

bool mi_str_starts_with(mi_str this, mi_str path);
bool mi_str_starts_with_c(mi_str this, c_str path);

bool mi_str_eq(mi_str a, mi_str b);

mi_str mi_str_add(mi_str a, mi_str b);

c_str mi_c_str(mi_str in);

c_str c_str_new(usize size);

str str_new(usize size);

mi_str mi_str_new(str in);

typedef struct mi_pos mi_pos;

struct mi_pos {
  usize idx, col, row;
  mi_str path;
};

str mi_pos_str(mi_pos in);

mi_pos mi_pos_new(mi_str path, usize idx, usize col, usize row);

void mi_pos_adv(mi_pos* in, mi_str file);

str mi_sprintf(c_str fmt, ...);

#define mi_arr_last(arr) (arr.ptr + arr.count - 1)