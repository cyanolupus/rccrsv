#include "reccursive.h"

Type*
type_new(TypeKind kind, Type* ptr_to)
{
  Type* type = calloc(1, sizeof(Type));
  type->kind = kind;
  type->ptr_to = ptr_to;
  if (kind == TY_INT)
    type->size = 4;
  else if (kind == TY_PTR)
    type->size = 8;
  else if (kind == TY_VOID)
    type->size = 0;
  else if (kind == TY_LONG)
    type->size = 8;
  else if (kind == TY_CHAR)
    type->size = 1;
  else if (kind == TY_FLOAT)
    type->size = 4;
  else if (kind == TY_SHORT)
    type->size = 2;
  else if (kind == TY_DOUBLE)
    type->size = 8;
  return type;
}

Type*
type_new_int()
{
  return type_new(TY_INT, NULL);
}

Type*
type_new_ptr(Type* ptr_to)
{
  return type_new(TY_PTR, ptr_to);
}

Type*
type_new_void()
{
  return type_new(TY_VOID, NULL);
}

Type*
type_new_long()
{
  return type_new(TY_LONG, NULL);
}

Type*
type_new_char()
{
  return type_new(TY_CHAR, NULL);
}

Type*
type_new_float()
{
  return type_new(TY_FLOAT, NULL);
}

Type*
type_new_short()
{
  return type_new(TY_SHORT, NULL);
}

Type*
type_new_double()
{
  return type_new(TY_DOUBLE, NULL);
}

Type*
type_new_func(Type* ret_type)
{
  Type* type = type_new(TY_FUNC, NULL);
  type->args = vector_new();
  type->ptr_to = ret_type;
  return type;
}

size_t
type_sizeof(Type* type)
{
  return type->size;
}

bool
type_equals(Type* lhs, Type* rhs)
{
  if (lhs->kind == TY_PTR && rhs->kind == TY_PTR)
    return type_equals(lhs->ptr_to, rhs->ptr_to);
  if (lhs->kind == TY_FUNC && rhs->kind == TY_FUNC)
    return type_func_equals(lhs, rhs);
  return lhs->kind == rhs->kind;
}

bool
type_func_equals(Type* lhs, Type* rhs)
{
  if (lhs->args->size != rhs->args->size)
    return false;

  fprintf(stderr, "lhs->args->size: %zu\n", lhs->args->size);
  fprintf(stderr, "rhs->args->size: %zu\n", rhs->args->size);

  for (int i = 0; i < lhs->args->size; i++) {
    Type* l = vector_get_type(lhs->args, i);
    Type* r = vector_get_type(rhs->args, i);
    if (!type_equals(l, r))
      return false;
  }

  return type_equals(lhs->ptr_to, rhs->ptr_to);
}

String*
type_ptr_to_string(Type* type)
{
  String* base = type_to_string(type->ptr_to);
  string_append(base, "*");
  return base;
}

String*
type_func_to_string(Type* type)
{
  String* base = type_to_string(type->ptr_to);
  string_append(base, " func(");

  for (int i = 0; i < type->args->size; i++) {
    Type* arg = vector_get(type->args, i);
    String* arg_str = type_to_string(arg);
    string_append(base, string_as_cstring(arg_str));
    if (i != type->args->size - 1)
      string_append(base, ", ");
    free(arg_str->data);
    free(arg_str);
  }

  string_append(base, ")");
  return base;
}

String*
type_to_string(Type* type)
{
  switch (type->kind) {
    case TY_INT:
      return string_new("int");
    case TY_PTR:
      return type_ptr_to_string(type);
    case TY_VOID:
      return string_new("void");
    case TY_LONG:
      return string_new("long");
    case TY_CHAR:
      return string_new("char");
    case TY_FLOAT:
      return string_new("float");
    case TY_SHORT:
      return string_new("short");
    case TY_DOUBLE:
      return string_new("double");
    case TY_FUNC:
      return type_func_to_string(type);
  }
  return NULL;
}
