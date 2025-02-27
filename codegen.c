#include "rccrsv.h"

int jmpcnt = 0;

int
writer(const char* format, ...)
{
  if (!output_fp) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stdout, format, args);
    va_end(args);
    return ret;
  } else {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(output_fp, format, args);
    va_end(args);
    return ret;
  }
}

void
gen_lval(Node* node)
{
  const char* r0 = rn(0, 8);
  if (node->kind == ND_DEREF) {
    Node* child = vector_get_node(node->children, 0);
    if (child->type->kind != TY_ARRAY)
      gen(child);
    else {
      gen_lval(child);
    }
  } else if (node->kind == ND_LVAR) {
    LVar* lvar = vector_get_lvar(node->argv, 0);
    writer("  sub %s, fp, #%lu ; %s %s addr\n",
           r0,
           lvar->offset,
           string_as_cstring(lvar->name),
           type_to_string(lvar->type)->data);
  } else if (node->kind == ND_GVAR) {
    writer("  adrp %s, _%s@PAGE\n", /* in arm64-apple-darwin @PAGE is needed */
           r0,
           string_as_cstring(vector_get_lvar(node->argv, 0)->name));
    writer("  add %s, %s, _%s@PAGEOFF\n", /* in arm64-apple-darwin @PAGEOFF is
                                            used instead of :lo12: */
           r0,
           r0,
           string_as_cstring(vector_get_lvar(node->argv, 0)->name));
  } else
    error("Not a variable on the left side of the assignment");
}

void
gen_addr(Node* node)
{
  if (node->kind == ND_DEREF) {
    gen(vector_get_node(node->children, 0));
  } else if (node->kind == ND_LVAR) {
    gen_lval(node);
  } else
    error("Not a variable on the left side of the assignment");
}

void
gen_num(Node* node)
{
  const char* r0 = rn(0, type_sizeof_aligned(node->type));
  writer("  mov %s, #%d\n", r0, node->val);
}

void
gen_lvar(Node* node)
{
  const char* r0 = rn(0, 8);
  const char* r_r0 = rn(0, type_sizeof_aligned(node->type));
  gen_lval(node);
  writer("  ldr %s, [%s] ; read val\n", r_r0, r0);
}

void
gen_ref(Node* node)
{
  gen_lval(vector_get_node(node->children, 0));
}

void
gen_assign(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);
  const char* r0 = rn(0, type_sizeof_aligned(type_new_ptr(node->type)));
  const char* r_r0 = rn(0, type_sizeof_aligned(lhs->type));
  const char* r9 = rn(9, type_sizeof_aligned(type_new_ptr(node->type)));
  gen_lval(lhs);
  writer("  str %s, [sp, #-16]!\n", r0);
  gen(rhs);

  writer("  ldr %s, [sp], #16\n", r9);
  switch (lhs->type->kind) {
    case TY_I8:
    case TY_U8:
      writer("  strb %s, [%s]\n", r_r0, r9);
      break;
    case TY_I16:
    case TY_U16:
      writer("  strh %s, [%s]\n", r_r0, r9);
      break;
    case TY_ISIZE:
    case TY_USIZE:
    case TY_I32:
    case TY_U32:
    case TY_I64:
    case TY_U64:
    case TY_PTR:
    case TY_ARRAY:
      writer("  str %s, [%s]\n", r_r0, r9);
      break;
    default:
      error("Not supported type %s", type_to_string(lhs->type)->data);
  }
}

void
gen_post_assign(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);
  const char* r0 = rn(0, type_sizeof_aligned(type_new_ptr(node->type)));
  const char* r_r0 = rn(0, type_sizeof_aligned(lhs->type));
  const char* r9 = rn(9, type_sizeof_aligned(type_new_ptr(node->type)));
  gen_lvar(lhs);
  writer("  str %s, [sp, #-16]!\n", r_r0);
  gen_lval(lhs);
  writer("  str %s, [sp, #-16]!\n", r0);
  gen(rhs);

  writer("  ldr %s, [sp], #16\n", r9);
  switch (lhs->type->kind) {
    case TY_I8:
    case TY_U8:
      writer("  strb %s, [%s]\n", r_r0, r9);
      break;
    case TY_I16:
    case TY_U16:
      writer("  strh %s, [%s]\n", r_r0, r9);
      break;
    case TY_ISIZE:
    case TY_USIZE:
    case TY_I32:
    case TY_U32:
    case TY_I64:
    case TY_U64:
    case TY_PTR:
    case TY_ARRAY:
      writer("  str %s, [%s]\n", r_r0, r9);
      break;
    default:
      error("Not supported type %s", type_to_string(lhs->type)->data);
  }
  writer("  ldr %s, [sp], #16\n", r_r0);
}

void
gen_return(Node* node)
{
  gen(vector_get_node(node->children, 0));
  writer("  add sp, sp, #%lu\n", 256);
  writer("  mov sp, fp\n");
  writer("  ldp fp, lr, [sp], #16\n");
  writer("  ret\n");
}

void
gen_if(Node* node)
{
  int curcnt = jmpcnt++;
  Node* node_cond = vector_get_node(node->children, 0);
  Node* node_then = vector_get_node(node->children, 1);
  Node* node_else = vector_get_node(node->children, 2);
  const char* r0 = rn(0, 1);

  gen(node_cond);
  writer("  cmp %s, #0\n", r0);

  if (node_else) {
    writer("  beq .Lelse%d\n", curcnt);
    gen_stmt(node_then);
    writer("  b .Lend%d\n", curcnt);
    writer(".Lelse%d:\n", curcnt);
    gen_stmt(node_else);
    writer(".Lend%d:\n", curcnt);
  } else {
    writer("  beq .Lend%d\n", curcnt);
    gen_stmt(node_then);
    writer(".Lend%d:\n", curcnt);
  }
}

void
gen_for(Node* node)
{
  int curcnt = jmpcnt++;
  Node* node_init = vector_get_node(node->children, 0);
  Node* node_cond = vector_get_node(node->children, 1);
  Node* node_inc = vector_get_node(node->children, 2);
  Node* node_body = vector_get_node(node->children, 3);
  const char* r0 = rn(0, 1);

  if (node_init)
    gen(node_init);
  writer(".Lbegin%d:\n", curcnt);
  if (node_cond) {
    gen(node_cond);
    writer("  cmp %s, #0\n", r0);
    writer("  beq .Lend%d\n", curcnt);
  }
  gen_stmt(node_body);
  if (node_inc)
    gen(node_inc);
  writer("  b .Lbegin%d\n", curcnt);
  writer(".Lend%d:\n", curcnt);
}

void
gen_block(Node* node)
{
  for (int i = 0; i < node->children->size; i++) {
    gen_stmt(vector_get_node(node->children, i));
  }
}

void
gen_func(Node* node)
{
  LVar* func = vector_get_lvar(node->argv, node->argv->size - 1);
  writer(".global _%s\n", string_as_cstring(func->name));
  writer("_%s:\n", string_as_cstring(func->name));
  writer("  stp fp, lr, [sp, #-16]!\n");
  writer("  mov fp, sp\n");
  writer("  sub sp, sp, #%lu\n", 256);

  const char* r9 = rn(9, 8);
  for (int i = 0; i < node->argv->size - 1; i++) {
    LVar* lvar = vector_get_lvar(node->argv, i);

    if (lvar) {
      const char* ri = rn(i, type_sizeof_aligned(lvar->type));
      writer("  sub %s, fp, #%lu\n", r9, lvar->offset);
      writer("  str %s, [%s]\n", ri, r9);
    }
  }
  for (int i = 0; i < node->children->size; i++) {
    gen_stmt(vector_get_node(node->children, i));
  }
}

void
gen_call(Node* node)
{
  LVar* func = vector_get_lvar(node->argv, 0);
  for (int i = 0; i < node->children->size; i++) {
    gen_expr(vector_get_node(node->children, i));
  }
  for (int i = node->children->size - 1; i >= 0; i--) {
    const char* ri =
      rn(i, type_sizeof_aligned(vector_get_node(node->children, i)->type));
    writer("  ldr %s, [sp], #16\n", ri);
  }
  writer("  bl _%s\n", string_as_cstring(func->name));
}

void
gen_2op(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);

  const char* r0 = rn(0, type_sizeof_aligned(node->type));
  const char* r_r9 = rn(9, type_sizeof_aligned(rhs->type));
  const char* r9 = rn(9, type_sizeof_aligned(node->type));
  const char* r10 = rn(10, type_sizeof_aligned(node->type));

  gen_expr(lhs);
  gen_expr(rhs);

  writer("  ldr %s, [sp], #16\n", r9);
  writer("  ldr %s, [sp], #16\n", r0);

  switch (node->kind) {
    case ND_ADD:
      writer("  add %s, %s, %s\n", r0, r0, r9);
      return;
    case ND_SUB:
      writer("  sub %s, %s, %s\n", r0, r0, r9);
      return;
    case ND_MUL:
      writer("  mul %s, %s, %s\n", r0, r0, r9);
      return;
    case ND_DIV:
      writer("  sdiv %s, %s, %s\n", r0, r0, r9);
      return;
    case ND_MOD:
      writer("  sdiv %s, %s, %s\n", r10, r0, r9);
      writer("  msub %s, %s, %s, %s\n", r0, r10, r9, r0);
      return;
    case ND_EQ:
      writer("  cmp %s, %s\n", r0, r9);
      writer("  cset %s, EQ\n", r0);
      return;
    case ND_NE:
      writer("  cmp %s, %s\n", r0, r9);
      writer("  cset %s, NE\n", r0);
      return;
    case ND_LT:
      writer("  cmp %s, %s\n", r0, r9);
      writer("  cset %s, LT\n", r0);
      return;
    case ND_LE:
      writer("  cmp %s, %s\n", r0, r9);
      writer("  cset %s, LE\n", r0);
      return;
    case ND_SHIFT_L:
      writer("  lsl %s, %s, %s\n", r0, r0, r9);
      return;
    case ND_SHIFT_R:
      writer("  lsr %s, %s, %s\n", r0, r0, r9);
      return;
    case ND_AND:
      writer("  and %s, %s, %s\n", r0, r0, r9);
      return;
    case ND_XOR:
      writer("  eor %s, %s, %s\n", r0, r0, r9);
      return;
    case ND_OR:
      writer("  orr %s, %s, %s\n", r0, r0, r9);
      return;
    case ND_LAND:
      writer("  cmp %s, #0\n", r0);
      writer("  cset %s, NE\n", r0);
      writer("  cmp %s, #0\n", r9);
      writer("  cset %s, NE\n", r9);
      writer("  and %s, %s, %s\n", r0, r0, r9);
      return;
    case ND_LOR:
      writer("  cmp %s, #0\n", r0);
      writer("  cset %s, NE\n", r0);
      writer("  cmp %s, #0\n", r9);
      writer("  cset %s, NE\n", r9);
      writer("  orr %s, %s, %s\n", r0, r0, r9);
      return;
    default:
      error("NodeKind is not supported %d", node->kind);
  }
}

void
gen_1op(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  const char* r_r0 = rn(0, type_sizeof_aligned(lhs->type));
  const char* r0 = rn(0, type_sizeof_aligned(node->type));
  const char* r9 = rn(9, type_sizeof_aligned(node->type));
  gen(lhs);

  switch (node->kind) {
    case ND_LNOT:
      writer("  cmp %s, #0\n", r0);
      writer("  cset %s, EQ\n", r0);
      return;
    case ND_NOT:
      writer("  mvn %s, %s\n", r0, r0);
      return;
    case ND_DEREF:
      writer("  ldr %s, [%s]\n", r0, rn(0, 8));
      return;
    case ND_SIZEOF:
      writer("  mov %s, %d\n", r0, type_sizeof(lhs->type));
      return;
    case ND_AUTOCAST:
      if (type_equals(lhs->type, node->type)) {
        eprintf("Reached unreachable code\n");
        return;
      }
      switch (lhs->type->kind) {
        case TY_PTR:
          switch (node->type->kind) {
            case TY_ARRAY:
              writer("  mov %s, %s\n", r0, r_r0);
              return;
            default:
              break;
          }
          break;
        case TY_ARRAY:
          switch (node->type->kind) {
            case TY_PTR:
              writer("  mov %s, %s\n", r0, r_r0);
              return;
            default:
              break;
          }
          break;
        case TY_I8:
          switch (node->type->kind) {
            case TY_I16:
            case TY_ISIZE:
            case TY_I32:
            case TY_I64:
              writer("  sxtb %s, %s\n", r0, r_r0);
              return;
            default:
              break;
          }
        case TY_I16:
          switch (node->type->kind) {
            case TY_ISIZE:
            case TY_I32:
            case TY_I64:
              writer("  sxth %s, %s\n", r0, r_r0);
              return;
            default:
              break;
          }
        case TY_ISIZE:
        case TY_I32:
          switch (node->type->kind) {
            case TY_ISIZE:
            case TY_I32:
            case TY_I64:
              writer("  sxtw %s, %s\n", r0, r_r0);
              return;
            default:
              break;
          }
        case TY_I64:
          switch (node->type->kind) {
            default:
              break;
          }
        case TY_U8:
          switch (node->type->kind) {
            case TY_U16:
            case TY_USIZE:
            case TY_U32:
            case TY_U64:
            case TY_I16:
            case TY_ISIZE:
            case TY_I32:
            case TY_I64:
              writer("  uxtb %s, %s\n", r0, r_r0);
              return;
            default:
              break;
          }
        case TY_U16:
          switch (node->type->kind) {
            case TY_USIZE:
            case TY_U32:
            case TY_U64:
            case TY_ISIZE:
            case TY_I32:
            case TY_I64:
              writer("  uxth %s, %s\n", r0, r_r0);
              return;
            default:
              break;
          }
        case TY_USIZE:
        case TY_U32:
          switch (node->type->kind) {
            case TY_USIZE:
            case TY_U32:
            case TY_U64:
            case TY_ISIZE:
            case TY_I32:
            case TY_I64:
              writer("  uxtw %s, %s\n", r0, r_r0);
              return;
            default:
              break;
          }
        case TY_U64:
          switch (node->type->kind) {
            case TY_PTR:
            case TY_ARRAY:
              writer("  uxtw %s, %s\n", r0, r_r0);
              writer(
                "  mov %s, #%d\n", r9, type_sizeof_aligned(node->type->ptr_to));
              writer("  mul %s, %s, %s\n", r0, r0, r9);
              return;
            default:
              break;
          }
          break;
        default:
          break;
      }
      fprintf(stderr,
              "Invalid type cast %s -> %s\n",
              type_to_string(lhs->type)->data,
              type_to_string(node->type)->data);
      exit(1);
      return;
    default:
      error("NodeKind is not supported %d", node->kind);
  }
}

void
gen(Node* node)
{
  Node* assign;
  if (node == NULL)
    return;
  switch (node->kind) {
    case ND_NUM:
      gen_num(node);
      break;
    case ND_LVAR:
    case ND_GVAR:
      gen_lvar(node);
      break;
    case ND_ASSIGN:
      gen_assign(node);
      break;
    case ND_REF:
      gen_ref(node);
      break;
    case ND_POST_ASSIGN:
      gen_post_assign(node);
      break;
    case ND_RETURN:
      gen_return(node);
      break;
    case ND_IF:
      gen_if(node);
      break;
    case ND_FOR:
      gen_for(node);
      break;
    case ND_BLOCK:
      gen_block(node);
      break;
    case ND_FUNC:
      gen_func(node);
      break;
    case ND_CALL:
      gen_call(node);
      break;
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_MOD:
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_SHIFT_L:
    case ND_SHIFT_R:
    case ND_AND:
    case ND_XOR:
    case ND_OR:
    case ND_LAND:
    case ND_LOR:
      gen_2op(node);
      break;
    case ND_LNOT:
    case ND_NOT:
    case ND_DEREF:
    case ND_SIZEOF:
    case ND_AUTOCAST:
      gen_1op(node);
      break;
    case ND_LDECLARE:
      writer("  ; declare local variable %s\n",
             string_as_cstring(vector_get_lvar(node->argv, 0)->name));
      if (node->children) {
        assign = vector_get_node(node->children, 0);
        if (assign && assign->kind == ND_ASSIGN) {
          gen_assign(assign);
          return;
        }
      }
      break;
    case ND_GDECLARE:
      writer("; declare global variable %s\n",
             string_as_cstring(vector_get_lvar(node->argv, 0)->name));
      long long val = 0;
      if (node->children) {
        assign = vector_get_node(node->children, 0);
        if (assign && assign->kind == ND_ASSIGN) {
          val = vector_get_node(assign->children, 1)->val;
        }
      }
      if (!node->type) {
        error("Type is not set for global variable %s",
              string_as_cstring(vector_get_lvar(node->argv, 0)->name));
      }
      if (node->type->kind == TY_FUNC) {
        return;
      }
      writer(".global _%s\n",
             string_as_cstring(vector_get_lvar(node->argv, 0)->name));
      writer("_%s:\n", string_as_cstring(vector_get_lvar(node->argv, 0)->name));
      if (node->type->kind == TY_ARRAY) {
        writer("  .zero %lu\n", type_sizeof_aligned(node->type));
      }
      if (node->type->kind == TY_PTR) {
        writer("  .xword %d\n", val);
      }
      if (node->type->kind == TY_I8 || node->type->kind == TY_U8) {
        writer("  .byte %d\n", *(signed int*)&val);
      }
      if (node->type->kind == TY_I16 || node->type->kind == TY_U16) {
        writer("  .hword %d\n", *(signed int*)&val);
      }
      if (node->type->kind == TY_ISIZE || node->type->kind == TY_USIZE) {
        writer("  .word %d\n", *(signed int*)&val);
      }
      if (node->type->kind == TY_I32 || node->type->kind == TY_U32) {
        writer("  .word %ld\n", *(signed long*)&val);
      }
      if (node->type->kind == TY_I64 || node->type->kind == TY_U64) {
        writer("  .xword %lld\n", *(signed long long*)&val);
      }
      break;
    default:
      error("NodeKind is not supported %d", node->kind);
  }
}

void
gen_expr(Node* node)
{
  if (node == NULL)
    return;
  gen(node);
  const char* r0 = rn(0, type_sizeof_aligned(node->type));
  writer("  str %s, [sp, #-16]!\n", r0);
}

void
gen_stmt(Node* node)
{
  if (node == NULL)
    return;
  gen(node);
}

void
gen_code(Program* program)
{
  if (!program)
    return;
  if (!(program->vars->size == 0)) {
    writer(".section __DATA,__data\n");
  }
  for (int i = 0; i < program->vars->size; i++) {
    gen_stmt(vector_get_node(program->vars, i));
  }
  writer(".section __TEXT,__text\n");
  writer(".align 2\n");
  for (int i = 0; i < program->code->size; i++) {
    gen_stmt(vector_get_node(program->code, i));
  }
}
