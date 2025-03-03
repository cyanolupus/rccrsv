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

Scope*
scope_new(Scope* parent)
{
  Scope* scope = calloc(1, sizeof(Scope));
  scope->vars = hashmap_new();
  scope->parent = parent;
  return scope;
}

Scope*
scope_parent(Scope* scope)
{
  return scope->parent;
}

Scope*
scope_root()
{
  Scope* scope = scope_new(NULL);
  scope->latest_offset = 0;
  return scope;
}

size_t
scope_get_latest_offset(Scope* scope)
{
  while (scope->parent) {
    scope = scope->parent;
  }
  return scope->latest_offset;
}

size_t
scope_add_latest_offset(Scope* scope, size_t size)
{
  while (scope->parent) {
    scope = scope->parent;
  }
  scope->latest_offset += size;
  return scope->latest_offset;
}

size_t
scope_set_latest_offset_aligned(Scope* scope, size_t align)
{
  size_t offset = scope_get_latest_offset(scope);
  return (offset + align - 1) / align * align;
}

LVar*
scope_find_lvar(Scope* scope, String* name)
{
  Scope* current = scope;
  while (current) {
    LVar* lvar = hashmap_get(current->vars, name->data);
    if (lvar) {
      return lvar;
    }
    current = current->parent;
  }
  return NULL;
}

LVar*
scope_expect_lvar(Scope* scope, String* name)
{
  LVar* lvar = scope_find_lvar(scope, name);
  if (!lvar) {
    fprintf(stderr, "undefined variable: %s\n", name->data);
    exit(1);
  }
  return lvar;
}

LVar*
scope_add_lvar(Scope* scope, String* name, Type* type)
{
  LVar* lvar = hashmap_get(scope->vars, name->data);
  if (!lvar) {
    scope_set_latest_offset_aligned(scope, type_sizeof_aligned(*type));
    scope_add_latest_offset(scope, type_sizeof_aligned(*type));
    lvar = lvar_new(name, scope_get_latest_offset(scope), type);
    hashmap_put(scope->vars, name->data, lvar);
  } else {
    if (!type_equals(*lvar->type, *type)) {
      fprintf(stderr,
              "conflicting types for '%s' %s vs %s\n",
              name->data,
              type_to_string(*lvar->type)->data,
              type_to_string(*type)->data);
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
    if (!type_equals(*lvar->type, *type)) {
      fprintf(stderr,
              "conflicting types for '%s' %s vs %s\n",
              name->data,
              type_to_string(*lvar->type)->data,
              type_to_string(*type)->data);
      exit(1);
    }
  }

  return lvar;
}

LVar*
scope_expect_var(Scope* scope, String* name)
{
  LVar* lvar = scope_find_lvar(scope, name);
  if (!lvar) {
    lvar = find_gvar(name);
  }
  if (!lvar) {
    fprintf(stderr, "undefined variable: %s\n", name->data);
    exit(1);
  }
  return lvar;
}
