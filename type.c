#include "reccursive.h"

Type*
type_new(TypeKind kind, Type* ptr_to)
{
  Type* type = calloc(1, sizeof(Type));
  type->kind = kind;
  type->ptr_to = ptr_to;
  return type;
}

Type*
type_new_int()
{
  return type_new(TY_ISIZE, NULL);
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
type_new_i8()
{
  return type_new(TY_I8, NULL);
}

Type*
type_new_i16()
{
  return type_new(TY_I16, NULL);
}

Type*
type_new_isize()
{
  return type_new(TY_ISIZE, NULL);
}

Type*
type_new_i32()
{
  return type_new(TY_I32, NULL);
}

Type*
type_new_i64()
{
  return type_new(TY_I64, NULL);
}

Type*
type_new_u8()
{
  return type_new(TY_U8, NULL);
}

Type*
type_new_u16()
{
  return type_new(TY_U16, NULL);
}

Type*
type_new_usize()
{
  return type_new(TY_USIZE, NULL);
}

Type*
type_new_u32()
{
  return type_new(TY_U32, NULL);
}

Type*
type_new_u64()
{
  return type_new(TY_U64, NULL);
}

Type*
type_new_float()
{
  return type_new(TY_FLOAT, NULL);
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

Type*
type_new_array(Type* ptr_to, size_t size)
{
  Type* type = type_new(TY_ARRAY, ptr_to);
  type->size = size;
  return type;
}

size_t
type_sizeof(Type* type)
{
  if (type->kind == TY_PTR)
    return 8;
  else if (type->kind == TY_ARRAY)
    return type->size * type_sizeof(type->ptr_to);
  else if (type->kind == TY_VOID)
    return 0;
  else if (type->kind == TY_I8 || type->kind == TY_U8)
    return 1;
  else if (type->kind == TY_I16 || type->kind == TY_U16)
    return 2;
  else if (type->kind == TY_ISIZE || type->kind == TY_USIZE)
    return 4;
  else if (type->kind == TY_I32 || type->kind == TY_U32)
    return 4;
  else if (type->kind == TY_I64 || type->kind == TY_U64)
    return 8;
  else if (type->kind == TY_FLOAT)
    return 4;
  else if (type->kind == TY_DOUBLE)
    return 8;
  else if (type->kind == TY_FUNC)
    return 0;
  error("Unknown type");
  return 0;
}

size_t
type_sizeof_aligned(Type* type)
{
  size_t size = type_sizeof(type);
  if (type->kind == TY_ARRAY)
    return type->size * type_sizeof_aligned(type->ptr_to);
  if (size < 4)
    return 4;
  return size;
}

bool
type_equals(Type* lhs, Type* rhs)
{
  if (lhs == NULL || rhs == NULL)
    return false;
  if (lhs->kind == TY_PTR && rhs->kind == TY_PTR)
    return type_equals(lhs->ptr_to, rhs->ptr_to);
  if (lhs->kind == TY_ARRAY && rhs->kind == TY_ARRAY)
    return type_equals(lhs->ptr_to, rhs->ptr_to) && lhs->size == rhs->size;
  if (lhs->kind == TY_FUNC && rhs->kind == TY_FUNC)
    return type_func_equals(lhs, rhs);
  return lhs->kind == rhs->kind;
}

bool
type_func_equals(Type* lhs, Type* rhs)
{
  if (lhs->args->size != rhs->args->size)
    return false;

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
type_array_to_string(Type* type)
{
  String* base = type_to_string(type->ptr_to);
  string_append(base, "[");
  char buf[256];
  snprintf(buf, 256, "%zu", type->size);
  string_append(base, buf);
  string_append(base, "]");
  return base;
}

String*
type_to_string(Type* type)
{
  if (type == NULL)
    return string_new("NULL");
  switch (type->kind) {
    case TY_PTR:
      return type_ptr_to_string(type);
    case TY_VOID:
      return string_new("void");
    case TY_FUNC:
      return type_func_to_string(type);
    case TY_ARRAY:
      return type_array_to_string(type);
    case TY_I8:
      return string_new("signed char");
    case TY_I16:
      return string_new("signed short int");
    case TY_ISIZE:
      return string_new("signed int");
    case TY_I32:
      return string_new("signed long int");
    case TY_I64:
      return string_new("signed long long int");
    case TY_U8:
      return string_new("unsigned char");
    case TY_U16:
      return string_new("unsigned short int");
    case TY_USIZE:
      return string_new("unsigned int");
    case TY_U32:
      return string_new("unsigned long int");
    case TY_U64:
      return string_new("unsigned long long int");
    case TY_FLOAT:
      return string_new("float");
    case TY_DOUBLE:
      return string_new("double");
    default:
      return string_new("unknown");
  }
  return string_new("unknown");
}
