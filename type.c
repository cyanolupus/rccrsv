#include "rccrsv.h"

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
type_new_ptr(Type ptr_to)
{
  Type* type = calloc(1, sizeof(Type));
  type->kind = ptr_to.kind;
  type->args = ptr_to.args;
  type->size = ptr_to.size;
  type->ptr_to = ptr_to.ptr_to;
  return type_new(TY_PTR, type);
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
  type->args = vector_new(8);
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
type_sizeof(Type type)
{
  if (type.kind == TY_PTR)
    return 8;
  else if (type.kind == TY_ARRAY)
    return type.size * type_sizeof(*type.ptr_to);
  else if (type.kind == TY_VOID)
    return 0;
  else if (type.kind == TY_I8 || type.kind == TY_U8)
    return 1;
  else if (type.kind == TY_I16 || type.kind == TY_U16)
    return 2;
  else if (type.kind == TY_ISIZE || type.kind == TY_USIZE)
    return 4;
  else if (type.kind == TY_I32 || type.kind == TY_U32)
    return 4;
  else if (type.kind == TY_I64 || type.kind == TY_U64)
    return 8;
  else if (type.kind == TY_FLOAT)
    return 4;
  else if (type.kind == TY_DOUBLE)
    return 8;
  else if (type.kind == TY_FUNC)
    return 0;
  fprintf(stderr, "Unknown type\n");
  exit(1);
}

size_t
type_sizeof_aligned(Type type)
{
  size_t size = type_sizeof(type);
  if (type.kind == TY_ARRAY)
    return type.size * type_sizeof_aligned(*type.ptr_to);
  // if (size < 4)
  //   return 4;
  return size;
}

bool
type_func_equals(Type lhs, Type rhs)
{
  if (lhs.args->size != rhs.args->size)
    return false;

  for (int i = 0; i < lhs.args->size; i++) {
    Type l = *vector_get_type(lhs.args, i);
    Type r = *vector_get_type(rhs.args, i);
    if (!type_equals(l, r))
      return false;
  }

  return type_equals(*lhs.ptr_to, *rhs.ptr_to);
}

bool
type_equals(Type lhs, Type rhs)
{
  if (lhs.kind == TY_PTR && rhs.kind == TY_PTR)
    return type_equals(*lhs.ptr_to, *rhs.ptr_to);
  if (lhs.kind == TY_ARRAY && rhs.kind == TY_ARRAY)
    return type_equals(*lhs.ptr_to, *rhs.ptr_to) && lhs.size == rhs.size;
  if (lhs.kind == TY_FUNC && rhs.kind == TY_FUNC)
    return type_func_equals(lhs, rhs);
  return lhs.kind == rhs.kind;
}

Type*
type_integer_promotion(Type type)
{
  if (type.kind == TY_I8 || type.kind == TY_U8)
    return type_new_i32();
  if (type.kind == TY_I16 || type.kind == TY_U16)
    return type_new_i32();
  if (type.kind == TY_ISIZE)
    return type_new_i32();
  if (type.kind == TY_ARRAY)
    return type_new_ptr(*type.ptr_to);
  return type_new(type.kind, type.ptr_to);
}

Type*
type_arithmetic_autocast(Type lhs, Type rhs)
{
  if (type_equals(lhs, rhs))
    return type_new(lhs.kind, NULL);
  if (lhs.kind == TY_PTR)
    return type_new(TY_PTR, lhs.ptr_to);
  if (rhs.kind == TY_PTR)
    return type_new(TY_PTR, rhs.ptr_to);
  if (lhs.kind == TY_ARRAY)
    return type_new(TY_PTR, lhs.ptr_to);
  if (rhs.kind == TY_ARRAY)
    return type_new(TY_PTR, rhs.ptr_to);

  switch (lhs.kind) {
    case TY_I8:
      return type_new(rhs.kind, NULL);
    case TY_I16:
      switch (rhs.kind) {
        case TY_I8:
        case TY_U8:
          return type_new(lhs.kind, NULL);
        case TY_ISIZE:
        case TY_I32:
        case TY_I64:
        case TY_U16:
        case TY_USIZE:
        case TY_U32:
        case TY_U64:
          return type_new(rhs.kind, NULL);
        default:
          break;
      }
      break;
    case TY_ISIZE:
    case TY_I32:
      switch (rhs.kind) {
        case TY_I8:
        case TY_I16:
        case TY_U8:
        case TY_U16:
          return type_new(lhs.kind, NULL);
        case TY_ISIZE:
        case TY_I32:
        case TY_I64:
        case TY_USIZE:
        case TY_U32:
        case TY_U64:
          return type_new(rhs.kind, NULL);
        default:
          break;
      }
      break;
    case TY_I64:
      switch (rhs.kind) {
        case TY_I8:
        case TY_I16:
        case TY_ISIZE:
        case TY_I32:
        case TY_U8:
        case TY_U16:
        case TY_USIZE:
        case TY_U32:
          return type_new(lhs.kind, NULL);
        case TY_U64:
          return type_new(rhs.kind, NULL);
        default:
          break;
      }
      break;
    case TY_U8:
      switch (rhs.kind) {
        case TY_U16:
        case TY_USIZE:
        case TY_U32:
        case TY_U64:
          return type_new(rhs.kind, NULL);
        case TY_I16:
        case TY_ISIZE:
        case TY_I32:
        case TY_I64:
          return type_new(lhs.kind, NULL);
        case TY_I8:
          return type_new_u8();
        default:
          break;
      }
      break;
    case TY_U16:
      switch (rhs.kind) {
        case TY_U8:
        case TY_I8:
        case TY_I16:
          return type_new(lhs.kind, NULL);
        case TY_USIZE:
        case TY_U32:
        case TY_U64:
        case TY_ISIZE:
        case TY_I32:
        case TY_I64:
          return type_new(rhs.kind, NULL);
        default:
          break;
      }
      break;
    case TY_USIZE:
    case TY_U32:
      switch (rhs.kind) {
        case TY_U8:
        case TY_U16:
        case TY_I8:
        case TY_I16:
        case TY_ISIZE:
        case TY_I32:
          return type_new(lhs.kind, NULL);
        case TY_USIZE:
        case TY_U32:
        case TY_U64:
        case TY_I64:
          return type_new(rhs.kind, NULL);
        default:
          break;
      }
      break;
    case TY_U64:
      switch (rhs.kind) {
        case TY_U8:
        case TY_U16:
        case TY_USIZE:
        case TY_U32:
          return type_new(lhs.kind, NULL);
        default:
          break;
      }
      break;
    default:
      break;
  }

  fprintf(stderr,
          "Invalid arithmetic operation: %s %s\n",
          type_to_string(lhs)->data,
          type_to_string(rhs)->data);
  exit(1);
}

String*
type_ptr_to_string(Type type)
{
  String* base = type_to_string(*type.ptr_to);
  base = string_append(base, "*");
  return base;
}

String*
type_func_to_string(Type type)
{
  String* base = type_to_string(*type.ptr_to);
  base = string_append(base, " func(");

  for (int i = 0; i < type.args->size; i++) {
    Type* arg = vector_get(type.args, i);
    String* arg_str = type_to_string(*arg);
    base = string_append(base, arg_str->data);
    if (i != type.args->size - 1)
      base = string_append(base, ", ");
    free(arg_str->data);
    free(arg_str);
  }

  base = string_append(base, ")");
  return base;
}

String*
type_array_to_string(Type type)
{
  String* base = type_to_string(*type.ptr_to);
  base = string_append(base, "[");
  char buf[256];
  snprintf(buf, 256, "%zu", type.size);
  base = string_append(base, buf);
  base = string_append(base, "]");
  return base;
}

String*
type_to_string(Type type)
{
  switch (type.kind) {
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
