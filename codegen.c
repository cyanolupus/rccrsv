#include "reccursive.h"
#include <stdio.h>

int jmpcnt = 0;

int
writer(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  int ret = vfprintf(stdout, format, args);
  va_end(args);
  return ret;
}

void
gen_lval(Node* node)
{
  if (node->kind != ND_LVAR)
    error("Not a variable on the left side of the assignment");

  LVar* lvar = vector_get_lvar(node->argv, 0);

  writer("  sub x0, fp, #%d ; var %s\n",
         lvar->offset,
         string_as_cstring(lvar->name));
  writer("  str x0, [sp, #-16]!\n");
}

void
gen_num(Node* node)
{
  writer("  mov x0, %d\n", node->val);
}

void
gen_lvar(Node* node)
{
  gen_lval(node);
  writer("  ldr x0, [sp], #16\n");
  writer("  ldr x0, [x0]\n");
}

void
gen_ref(Node* node)
{
  gen_lval(vector_get_node(node->children, 0));
  writer("  ldr x0, [sp], #16\n");
}

void
gen_assign(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);
  gen_lval(lhs);
  gen(rhs);

  writer("  ldr x1, [sp], #16\n");
  writer("  str x0, [x1]\n");
}

void
gen_post_assign(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);
  gen_lvar(lhs);
  writer("  str x0, [sp, #-16]!\n");
  gen_lval(lhs);
  gen(rhs);

  writer("  ldr x1, [sp], #16\n");
  writer("  str x0, [x1]\n");
  writer("  ldr x0, [sp], #16\n");
}

void
gen_return(Node* node)
{
  gen(vector_get_node(node->children, 0));
  writer("  add sp, sp, %d\n", 128);
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
  gen(node_cond);
  writer("  cmp x0, #0\n");

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
  if (node_init)
    gen(node_init);
  writer(".Lbegin%d:\n", curcnt);
  if (node_cond) {
    gen(node_cond);
    writer("  cmp x0, #0\n");
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
  LVar* func = vector_get_lvar(node->argv, 0);
  writer(".global _%s\n", string_as_cstring(func->name));
  writer("_%s:\n", string_as_cstring(func->name));
  writer("  stp fp, lr, [sp, #-16]!\n");
  writer("  mov fp, sp\n");
  writer("  sub sp, sp, %d\n", 128);
  for (int i = 1; i < node->argv->size; i++) {
    LVar* lvar = vector_get_lvar(node->argv, i);
    if (lvar) {
      writer("  str x%d, [fp, #-%d]\n", i - 1, lvar->offset);
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
    writer("  ldr x%d, [sp], #16\n", i);
  }
  writer("  bl _%s\n", string_as_cstring(func->name));
}

void
gen_2op(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);

  gen_expr(lhs);
  gen_expr(rhs);

  writer("  ldr x1, [sp], #16\n");
  writer("  ldr x0, [sp], #16\n");

  switch (node->kind) {
    case ND_ADD:
      writer("  add x0, x0, x1\n");
      return;
    case ND_SUB:
      writer("  sub x0, x0, x1\n");
      return;
    case ND_MUL:
      writer("  mul x0, x0, x1\n");
      return;
    case ND_DIV:
      writer("  sdiv x0, x0, x1\n");
      return;
    case ND_MOD:
      writer("  sdiv x2, x0, x1\n");
      writer("  msub x0, x2, x1, x0\n");
      return;
    case ND_EQ:
      writer("  cmp x0, x1\n");
      writer("  cset x0, EQ\n");
      return;
    case ND_NE:
      writer("  cmp x0, x1\n");
      writer("  cset x0, NE\n");
      return;
    case ND_LE:
      writer("  cmp x0, x1\n");
      writer("  cset x0, LE\n");
      return;
    case ND_LT:
      writer("  cmp x0, x1\n");
      writer("  cset x0, LT\n");
      return;
    case ND_SHIFT_L:
      writer("  lsl x0, x0, x1\n");
      return;
    case ND_SHIFT_R:
      writer("  lsr x0, x0, x1\n");
      return;
    case ND_AND:
      writer("  and x0, x0, x1\n");
      return;
    case ND_XOR:
      writer("  eor x0, x0, x1\n");
      return;
    case ND_OR:
      writer("  orr x0, x0, x1\n");
      return;
    case ND_LAND:
      writer("  cmp x0, #0\n");
      writer("  cset x0, NE\n");
      writer("  cmp x1, #0\n");
      writer("  cset x1, NE\n");
      writer("  and x0, x0, x1\n");
      return;
    case ND_LOR:
      writer("  cmp x0, #0\n");
      writer("  cset x0, NE\n");
      writer("  cmp x1, #0\n");
      writer("  cset x1, NE\n");
      writer("  orr x0, x0, x1\n");
      return;
    default:
      error("NodeKind is not supported %d", node->kind);
  }
}

void
gen_1op(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  gen(lhs);

  switch (node->kind) {
    case ND_LNOT:
      writer("  cmp x0, #0\n");
      writer("  cset x0, EQ\n");
      return;
    case ND_NOT:
      writer("  mvn x0, x0\n");
      return;
    case ND_DEREF:
      writer("  ldr x0, [x0]\n");
      return;
    case ND_SIZEOF:
      writer("  mov x0, 8\n"); // TODO
      return;
    default:
      error("NodeKind is not supported %d", node->kind);
  }
}

void
gen(Node* node)
{
  if (node == NULL)
    return;
  switch (node->kind) {
    case ND_NUM:
      gen_num(node);
      break;
    case ND_LVAR:
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
      gen_1op(node);
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
  writer("  str x0, [sp, #-16]!\n");
}

void
gen_stmt(Node* node)
{
  if (node == NULL)
    return;
  gen(node);
}