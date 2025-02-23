#include "reccursive.h"

Type*
type_new(TypeKind kind, Type* ptr_to)
{
  Type* type = malloc(sizeof(Type));
  type->kind = kind;
  type->ptr_to = ptr_to;
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
type_new_func()
{
  return type_new(TY_FUNC, NULL);
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

char*
type_to_string(Type* type)
{
  switch (type->kind) {
    case TY_INT:
      return "int";
    case TY_PTR:
      return "pointer";
    case TY_FUNC:
      return "function";
    case TY_VOID:
      return "void";
    case TY_LONG:
      return "long";
    case TY_CHAR:
      return "char";
    case TY_FLOAT:
      return "float";
    case TY_SHORT:
      return "short";
    case TY_DOUBLE:
      return "double";
  }
}
