#include "rccrsv.h"

LVar*
lvar_new(String* name, int offset, Type* type)
{
  LVar* lvar = calloc(1, sizeof(LVar));
  lvar->name = name;
  lvar->offset = offset;
  lvar->type = type;
  return lvar;
}

LVar*
find_lvar(String* name)
{
  return hashmap_get(program->locals, name->data);
}

LVar*
expect_lvar(String* name)
{
  LVar* lvar = find_lvar(name);
  if (!lvar) {
    fprintf(stderr, "undefined variable: %s\n", name->data);
    exit(1);
  }
  return lvar;
}

LVar*
add_lvar(String* name, Type* type)
{
  LVar* lvar = find_lvar(name);
  if (lvar == NULL) {
    if (program->latest_offset % type_sizeof_aligned(type) != 0) {
      program->latest_offset +=
        type_sizeof_aligned(type) -
        program->latest_offset % type_sizeof_aligned(type);
    }
    size_t head = program->latest_offset;
    if (type->kind == TY_ARRAY) {
      head += type_sizeof_aligned(type->ptr_to);
      program->latest_offset += type_sizeof_aligned(type);
    } else {
      program->latest_offset += type_sizeof_aligned(type);
      head = program->latest_offset;
    }
    lvar = lvar_new(name, head, type);
    hashmap_put(program->locals, name->data, lvar);
  } else {
    if (!type_equals(lvar->type, type)) {
      fprintf(stderr,
              "conflicting types for '%s' %s vs %s\n",
              name->data,
              type_to_string(lvar->type)->data,
              type_to_string(type)->data);
      exit(1);
    }
  }

  return lvar;
}

LVar*
find_gvar(String* name)
{
  return hashmap_get(program->globals, name->data);
}

LVar*
expect_gvar(String* name)
{
  LVar* lvar = find_gvar(name);
  if (!lvar) {
    fprintf(stderr, "undefined variable: %s\n", name->data);
    exit(1);
  }
  return lvar;
}

LVar*
add_gvar(String* name, Type* type)
{
  LVar* lvar = find_gvar(name);
  if (lvar == NULL) {
    lvar = lvar_new(name, 0, type);
    hashmap_put(program->globals, name->data, lvar);
  } else {
    if (!type_equals(lvar->type, type)) {
      fprintf(stderr,
              "conflicting types for '%s' %s vs %s\n",
              name->data,
              type_to_string(lvar->type)->data,
              type_to_string(type)->data);
      exit(1);
    }
  }

  return lvar;
}

LVar*
expect_var(String* name)
{
  LVar* lvar = find_lvar(name);
  if (!lvar) {
    lvar = find_gvar(name);
  }
  if (!lvar) {
    fprintf(stderr, "undefined variable: %s\n", name->data);
    exit(1);
  }
  return lvar;
}
