#pragma once

#include <string.h>
#include "util.h"

typedef struct mi_dyn_arr_metadata mi_dyn_arr_metadata;

struct mi_dyn_arr_metadata {
  usize len;
  usize count;
  usize elem_size;
};

typedef uint8_t byte;

mi_dyn_arr_metadata* internal_mi_dyn_arr_get_metadata(byte* memory);

byte* internal_mi_dyn_arr_new(usize len, usize elem_size);

byte* internal_mi_dyn_arr_base_ptr(byte* memory);

#define mi_dyn_arr(type, initial_size) (type*) internal_mi_dyn_arr_new(initial_size, sizeof(type))

void internal_mi_dyn_arr_add(byte** memory, byte* item, usize size);

#define mi_dyn_arr_add(this, ...) internal_mi_dyn_arr_add((byte**) &(this), (byte*) (__VA_ARGS__), sizeof(*(__VA_ARGS__)))
#define mi_dyn_arr_add_i(this, type, ...) internal_mi_dyn_arr_add((byte**) &(this), (byte*) &(type) {__VA_ARGS__}, sizeof(type))

#define mi_dyn_arr_len(this) internal_mi_dyn_arr_get_metadata((byte*) this)->count

bool internal_mi_dyn_arr_has(byte* memory, byte* element, usize elem_size);

#define mi_dyn_arr_has(this, element) internal_mi_dyn_arr_has((byte*) this, (byte*) &element, sizeof(element))
#define mi_dyn_arr_has_i(this, type, ...) internal_mi_dyn_arr_has((byte*) this, (byte*) &(type) {__VA_ARGS__}, sizeof(type))

#define mi_dyn_arr_last(this) &(this)[mi_dyn_arr_count(this) - 1]

#define mi_dyn_arr_count(this) internal_mi_dyn_arr_get_metadata((byte*) this)->count

void internal_mi_dyn_arr_del(byte* memory);

#define mi_dyn_arr_del(this) internal_mi_dyn_arr_del((byte*) this)