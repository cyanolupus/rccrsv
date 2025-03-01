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

size_t
reg_size(Type type)
{
  if (type.kind == TY_ARRAY)
    return 8;
  return type_sizeof_aligned(type);
}

void
gen_lval(Node* node)
{
  const char* r0 = rn(0, 8);
  if (node->kind == ND_DEREF) {
    Node* child = vector_get_node(node->children, 0);
    gen(child);
  } else if (node->kind == ND_LVAR) {
    LVar* lvar = vector_get_lvar(node->argv, 0);
    writer("  sub %s, fp, #%lu ; %s %s addr\n",
           r0,
           lvar->offset,
           lvar->name->data,
           type_to_string(*lvar->type)->data);
  } else if (node->kind == ND_GVAR) {
    writer("  adrp %s, _%s@PAGE\n", /* in arm64-apple-darwin @PAGE is needed */
           r0,
           vector_get_lvar(node->argv, 0)->name->data);
    writer("  add %s, %s, _%s@PAGEOFF\n", /* in arm64-apple-darwin @PAGEOFF is
                                            used instead of :lo12: */
           r0,
           r0,
           vector_get_lvar(node->argv, 0)->name->data);
  } else {
    fprintf(stderr, "Not a variable on the left side of the assignment\n");
    exit(1);
  }
}

void
gen_num(Node* node)
{
  const char* r0 = rn(0, reg_size(*node->type));
  writer("  mov %s, #%d\n", r0, node->val);
}

void
gen_lvar(Node* node)
{
  const char* r0 = rn(0, 8);
  const char* r_r0 = rn(0, reg_size(*node->type));
  gen_lval(node);
  switch (node->type->kind) {
    case TY_I8:
      writer("  ldrsb %s, [%s]\n", r_r0, r0);
    case TY_U8:
      writer("  ldrb %s, [%s]\n", r_r0, r0);
      break;
    case TY_I16:
      writer("  ldrsh %s, [%s]\n", r_r0, r0);
    case TY_U16:
      writer("  ldrh %s, [%s]\n", r_r0, r0);
      break;
    case TY_ISIZE:
    case TY_USIZE:
    case TY_I32:
    case TY_U32:
    case TY_I64:
    case TY_U64:
    case TY_PTR:
      writer("  ldr %s, [%s]\n", r_r0, r0);
      break;
    case TY_ARRAY:
      break;
    default:
      fprintf(
        stderr, "Not supported type %s\n", type_to_string(*node->type)->data);
      exit(1);
  }
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
  const char* r0 = rn(0, reg_size(*type_new_ptr(*node->type)));
  const char* r_r0 = rn(0, reg_size(*lhs->type));
  const char* r9 = rn(9, reg_size(*type_new_ptr(*node->type)));
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
      fprintf(
        stderr, "Not supported type %s\n", type_to_string(*lhs->type)->data);
      exit(1);
  }
}

void
gen_post_assign(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);
  const char* r0 = rn(0, reg_size(*type_new_ptr(*node->type)));
  const char* r_r0 = rn(0, reg_size(*lhs->type));
  const char* r9 = rn(9, reg_size(*type_new_ptr(*node->type)));
  gen_expr(lhs);
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
      fprintf(
        stderr, "Not supported type %s\n", type_to_string(*lhs->type)->data);
      exit(1);
  }
  writer("  ldr %s, [sp], #16\n", r_r0);
}

void
gen_return(Node* node)
{
  gen(vector_get_node(node->children, 0));
  writer("  add sp, sp, #%lu\n", program->latest_offset);
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
    writer("  beq LBB%d_else\n", curcnt);
    gen_stmt(node_then);
    writer("  b LBB%d_end\n", curcnt);
    writer("LBB%d_else:\n", curcnt);
    gen_stmt(node_else);
    writer("LBB%d_end:\n", curcnt);
  } else {
    writer("  beq LBB%d_end\n", curcnt);
    gen_stmt(node_then);
    writer("LBB%d_end:\n", curcnt);
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
  writer("LBB%d_begin:\n", curcnt);
  if (node_cond) {
    gen(node_cond);
    writer("  cmp %s, #0\n", r0);
    writer("  beq LBB%d_end\n", curcnt);
  }
  gen_stmt(node_body);
  if (node_inc)
    gen(node_inc);
  writer("  b LBB%d_begin\n", curcnt);
  writer("LBB%d_end:\n", curcnt);
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
  writer("  .globl _%s\n", func->name->data);
  writer("_%s:\n", func->name->data);
  writer("  stp fp, lr, [sp, #-16]!\n");
  writer("  mov fp, sp\n");
  writer("  sub sp, sp, #%lu\n", program_latest_offset_aligned(program, 16));

  const char* r9 = rn(9, 8);
  for (int i = 0; i < node->argv->size - 1; i++) {
    LVar* lvar = vector_get_lvar(node->argv, i);

    if (lvar) {
      const char* ri = rn(i, reg_size(*lvar->type));
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
    const char* ri = rn(i, reg_size(*vector_get_node(node->children, i)->type));
    writer("  ldr %s, [sp], #16\n", ri);
  }
  writer("  bl _%s\n", func->name->data);
}

void
gen_2op(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);

  const char* r0 = rn(0, reg_size(*node->type));
  const char* r_r9 = rn(9, reg_size(*rhs->type));
  const char* r9 = rn(9, reg_size(*node->type));
  const char* r10 = rn(10, reg_size(*node->type));

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
      fprintf(stderr, "NodeKind is not supported %d\n", node->kind);
      exit(1);
  }
}

void
gen_1op(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  const char* r_r0 = rn(0, reg_size(*lhs->type));
  const char* r0 = rn(0, reg_size(*node->type));
  const char* r9 = rn(9, reg_size(*node->type));
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
      switch (node->type->kind) {
        case TY_I8:
          writer("  ldrsb %s, [%s]\n", r0, r_r0);
          return;
        case TY_U8:
          writer("  ldrb %s, [%s]\n", r0, r_r0);
          return;
        case TY_I16:
          writer("  ldrsh %s, [%s]\n", r0, r_r0);
          return;
        case TY_U16:
          writer("  ldrh %s, [%s]\n", r0, r_r0);
          return;
        case TY_ISIZE:
        case TY_USIZE:
        case TY_I32:
        case TY_U32:
        case TY_I64:
        case TY_U64:
        case TY_PTR:
        case TY_ARRAY:
          writer("  ldr %s, [%s]\n", r0, r_r0);
          return;
        default:
          fprintf(stderr,
                  "Not supported type %s\n",
                  type_to_string(*lhs->type)->data);
          exit(1);
      }
      return;
    case ND_SIZEOF:
      writer("  mov %s, #%d\n", r0, type_sizeof(*lhs->type));
      return;
    case ND_AUTOCAST:
      if (type_equals(*lhs->type, *node->type)) {
        fprintf(stderr, "Reached unreachable code\n");
        exit(1);
        return;
      }
      switch (lhs->type->kind) {
        case TY_PTR:
          switch (node->type->kind) {
            case TY_ARRAY:
              return;
            default:
              break;
          }
          break;
        case TY_ARRAY:
          switch (node->type->kind) {
            case TY_PTR:
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
            case TY_U16:
            case TY_USIZE:
            case TY_U32:
            case TY_U64:
              writer("  uxtb %s, %s\n", r0, r_r0);
              return;
            case TY_PTR:
            case TY_ARRAY:
              writer("  uxtb %s, %s\n", r0, r_r0);
              writer("  mov %s, #%d\n",
                     r9,
                     type_sizeof_aligned(*node->type->ptr_to));
              writer("  mul %s, %s, %s\n", r0, r0, r9);
              return;
            default:
              break;
          }
          break;
        case TY_I16:
          switch (node->type->kind) {
            case TY_ISIZE:
            case TY_I32:
            case TY_I64:
              writer("  sxth %s, %s\n", r0, r_r0);
              return;
            case TY_USIZE:
            case TY_U32:
            case TY_U64:
              writer("  uxth %s, %s\n", r0, r_r0);
              return;
            case TY_PTR:
            case TY_ARRAY:
              writer("  uxth %s, %s\n", r0, r_r0);
              writer("  mov %s, #%d\n",
                     r9,
                     type_sizeof_aligned(*node->type->ptr_to));
              writer("  mul %s, %s, %s\n", r0, r0, r9);
              return;
            default:
              break;
          }
          break;
        case TY_ISIZE:
        case TY_I32:
          switch (node->type->kind) {
            case TY_ISIZE:
            case TY_I32:
              return;
            case TY_I64:
              writer("  sxtw %s, %s\n", r0, r_r0);
              return;
            case TY_U64:
              writer("  uxtw %s, %s\n", r0, r_r0);
              return;
            case TY_PTR:
            case TY_ARRAY:
              writer("  uxtw %s, %s\n", r0, r_r0);
              writer("  mov %s, #%d\n",
                     r9,
                     type_sizeof_aligned(*node->type->ptr_to));
              writer("  mul %s, %s, %s\n", r0, r0, r9);
              return;
            default:
              break;
          }
          break;
        case TY_I64:
          switch (node->type->kind) {
            default:
              break;
          }
          break;
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
            case TY_PTR:
            case TY_ARRAY:
              writer("  uxtb %s, %s\n", r0, r_r0);
              writer("  mov %s, #%d\n",
                     r9,
                     type_sizeof_aligned(*node->type->ptr_to));
              writer("  mul %s, %s, %s\n", r0, r0, r9);
              return;
            default:
              break;
          }
          break;
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
            case TY_PTR:
            case TY_ARRAY:
              writer("  uxth %s, %s\n", r0, r_r0);
              writer("  mov %s, #%d\n",
                     r9,
                     type_sizeof_aligned(*node->type->ptr_to));
              writer("  mul %s, %s, %s\n", r0, r0, r9);
              return;
            default:
              break;
          }
          break;
        case TY_USIZE:
        case TY_U32:
          switch (node->type->kind) {
            case TY_USIZE:
            case TY_U32:
              return;
            case TY_U64:
            case TY_ISIZE:
            case TY_I32:
            case TY_I64:
              writer("  uxtw %s, %s\n", r0, r_r0);
              return;
            case TY_PTR:
            case TY_ARRAY:
              writer("  uxtw %s, %s\n", r0, r_r0);
              writer("  mov %s, #%d\n",
                     r9,
                     type_sizeof_aligned(*node->type->ptr_to));
              writer("  mul %s, %s, %s\n", r0, r0, r9);
              return;
            default:
              break;
          }
          break;
        case TY_U64:
          switch (node->type->kind) {
            case TY_PTR:
            case TY_ARRAY:
              writer("  mov %s, #%d\n",
                     r9,
                     type_sizeof_aligned(*node->type->ptr_to));
              writer("  mul %s, %s, %s\n", r0, r0, r9);
              return;
            default:
              break;
          }
          break;
        default:
          break;
      }
      node_view_tree(program->node, 0, node);
      fprintf(stderr,
              "Invalid type cast %s -> %s\n",
              type_to_string(*lhs->type)->data,
              type_to_string(*node->type)->data);
      exit(1);
      return;
    default:
      fprintf(stderr, "NodeKind is not supported %d\n", node->kind);
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
             vector_get_lvar(node->argv, 0)->name->data);
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
             vector_get_lvar(node->argv, 0)->name->data);
      long long val = 0;
      if (node->children) {
        assign = vector_get_node(node->children, 0);
        if (assign && assign->kind == ND_ASSIGN) {
          val = vector_get_node(assign->children, 1)->val;
        }
      }
      if (!node->type) {
        fprintf(stderr,
                "Type is not set for global variable %s\n",
                vector_get_lvar(node->argv, 0)->name->data);
        exit(1);
      }
      writer(".global _%s\n", vector_get_lvar(node->argv, 0)->name->data);
      writer("_%s:\n", vector_get_lvar(node->argv, 0)->name->data);
      if (node->type->kind == TY_ARRAY) {
        writer("  .zero %lu\n", type_sizeof_aligned(*node->type));
        break;
      }
      if (node->type->kind == TY_PTR) {
        writer("  .xword %d\n", val);
        break;
      }
      if (node->type->kind == TY_I8 || node->type->kind == TY_U8) {
        writer("  .byte %d\n", *(signed int*)&val);
        break;
      }
      if (node->type->kind == TY_I16 || node->type->kind == TY_U16) {
        writer("  .hword %d\n", *(signed int*)&val);
        break;
      }
      if (node->type->kind == TY_ISIZE || node->type->kind == TY_USIZE) {
        writer("  .word %d\n", *(signed int*)&val);
        break;
      }
      if (node->type->kind == TY_I32 || node->type->kind == TY_U32) {
        writer("  .word %ld\n", *(signed long*)&val);
        break;
      }
      if (node->type->kind == TY_I64 || node->type->kind == TY_U64) {
        writer("  .xword %lld\n", *(signed long long*)&val);
        break;
      }
      fprintf(stderr,
              "Cannot declare global variable %s (type %s)\n",
              vector_get_lvar(node->argv, 0)->name->data,
              type_to_string(*node->type)->data);
      exit(1);
      break;
    case ND_STRING:
      writer("  adrp %s, l_.str.%d@PAGE\n", rn(0, 8), node->val);
      writer(
        "  add %s, %s, l_.str.%d@PAGEOFF\n", rn(0, 8), rn(0, 8), node->val);
      break;
    default:
      fprintf(stderr, "NodeKind is not supported %d\n", node->kind);
      exit(1);
  }
}

void
gen_expr(Node* node)
{
  if (node == NULL)
    return;
  gen(node);
  const char* r0 = rn(0, reg_size(*node->type));
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
    writer("  .section __DATA,__data\n");
  }
  for (int i = 0; i < program->vars->size; i++) {
    gen_stmt(vector_get_node(program->vars, i));
  }
  writer("  .section __TEXT,__text,regular,pure_instructions\n");
  writer("  .p2align 2\n");
  for (int i = 0; i < program->node->children->size; i++) {
    gen_stmt(vector_get_node(program->node->children, i));
  }

  if (!(program->strs->size == 0))
    writer("  .section __TEXT,__cstring,cstring_literals\n");

  for (int i = 0; i < program->strs->size; i++) {
    writer("l_.str.%d:\n", i);
    writer("  .asciz \"%s\"\n", vector_get_string(program->strs, i)->data);
  }

  writer("  .subsections_via_symbols\n");
}
