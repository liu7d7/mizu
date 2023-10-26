#include "dyn_arr.h"

mi_dyn_arr_metadata* internal_mi_dyn_arr_get_metadata(byte* memory) {
  return ((mi_dyn_arr_metadata*) (memory - sizeof(mi_dyn_arr_metadata)));
}

byte* internal_mi_dyn_arr_new(usize len, usize elem_size) {
  byte* memory = malloc(sizeof(mi_dyn_arr_metadata) + len * elem_size);
  memcpy(memory, &(mi_dyn_arr_metadata) {
    .len = len,
    .count = 0,
    .elem_size = elem_size
  }, sizeof(mi_dyn_arr_metadata));

  return memory + sizeof(mi_dyn_arr_metadata);
}

byte* internal_mi_dyn_arr_base_ptr(byte* memory) {
  return memory - sizeof(mi_dyn_arr_metadata);
}

void internal_mi_dyn_arr_add(byte** memory, byte* item, usize size) {
  mi_dyn_arr_metadata* data = internal_mi_dyn_arr_get_metadata(*memory);
  if (data->elem_size != size) {
    assert(false && "incompatible element sizes!");
  }

  if (data->len == data->count) {
    data->len *= 2;
    usize new_size_in_bytes = sizeof(mi_dyn_arr_metadata) + data->len * data->elem_size;
    byte* new_base_ptr = realloc(internal_mi_dyn_arr_base_ptr(*memory), new_size_in_bytes);
    *memory = new_base_ptr + sizeof(mi_dyn_arr_metadata);

    data = internal_mi_dyn_arr_get_metadata(*memory);
  }

  memcpy(*memory + data->count * data->elem_size, item, data->elem_size);
  data->count++;
}

bool internal_mi_dyn_arr_has(byte* memory, byte* element, usize elem_size) {
  mi_dyn_arr_metadata* data = internal_mi_dyn_arr_get_metadata(memory);

  if (data->elem_size != elem_size) {
    assert(false && "incompatible type");
  }

  for (int i = 0; i < data->count; i++) {
    byte* a = memory + data->elem_size * i;
    if (!memcmp(a, element, data->elem_size)) {
      return true;
    }
  }

  return false;
}

void internal_mi_dyn_arr_del(byte* memory) {
  byte* base = internal_mi_dyn_arr_base_ptr(memory);
  free(base);
}
