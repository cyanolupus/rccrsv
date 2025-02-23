#include "reccursive.h"
#include <stdio.h>

int jmpcnt = 0;

void
gen_lval(Node* node)
{
  if (node->kind != ND_LVAR)
    error("Not a variable on the left side of the assignment");

  LVar* lvar = vector_get_lvar(node->argv, 0);

  printf("  sub x0, fp, #%d ; var %s\n",
         lvar->offset,
         string_as_cstring(lvar->name));
  printf("  str x0, [sp, #-16]!\n");
}

void
gen_num(Node* node)
{
  printf("  mov x0, %d\n", node->val);
}

void
gen_lvar(Node* node)
{
  gen_lval(node);
  printf("  ldr x0, [sp], #16\n");
  printf("  ldr x0, [x0]\n");
}

void
gen_ref(Node* node)
{
  gen_lval(vector_get_node(node->children, 0));
  printf("  ldr x0, [sp], #16\n");
}

void
gen_assign(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);
  gen_lval(lhs);
  gen_stmt(rhs);

  printf("  ldr x1, [sp], #16\n");
  printf("  str x0, [x1]\n");
}

void
gen_post_assign(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);
  gen_lvar(lhs);
  printf("  str x0, [sp, #-16]!\n");
  gen_lval(lhs);
  gen_stmt(rhs);

  printf("  ldr x1, [sp], #16\n");
  printf("  str x0, [x1]\n");
  printf("  ldr x0, [sp], #16\n");
}

void
gen_return(Node* node)
{
  gen_stmt(vector_get_node(node->children, 0));
  printf("  add sp, sp, %d\n", 128);
  printf("  mov sp, fp\n");
  printf("  ldp fp, lr, [sp], #16\n");
  printf("  ret\n");
}

void
gen_if(Node* node)
{
  int curcnt = jmpcnt++;
  Node* node_cond = vector_get_node(node->children, 0);
  Node* node_then = vector_get_node(node->children, 1);
  Node* node_else = vector_get_node(node->children, 2);
  gen_stmt(node_cond);
  printf("  cmp x0, #0\n");

  if (node_else) {
    printf("  beq .Lelse%d\n", curcnt);
    gen_stmt(node_then);
    printf("  b .Lend%d\n", curcnt);
    printf(".Lelse%d:\n", curcnt);
    gen_stmt(node_else);
    printf(".Lend%d:\n", curcnt);
  } else {
    printf("  beq .Lend%d\n", curcnt);
    gen_stmt(node_then);
    printf(".Lend%d:\n", curcnt);
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
    gen_stmt(node_init);
  printf(".Lbegin%d:\n", curcnt);
  if (node_cond) {
    gen_stmt(node_cond);
    printf("  cmp x0, #0\n");
    printf("  beq .Lend%d\n", curcnt);
  }
  gen_stmt(node_body);
  if (node_inc)
    gen_stmt(node_inc);
  printf("  b .Lbegin%d\n", curcnt);
  printf(".Lend%d:\n", curcnt);
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
  printf(".global _%s\n", string_as_cstring(func->name));
  printf("_%s:\n", string_as_cstring(func->name));
  printf("  stp fp, lr, [sp, #-16]!\n");
  printf("  mov fp, sp\n");
  printf("  sub sp, sp, %d\n", 128);
  for (int i = 1; i < node->argv->size; i++) {
    LVar* lvar = vector_get_lvar(node->argv, i);
    if (lvar) {
      printf("  str x%d, [fp, #-%d]\n", i - 1, lvar->offset);
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
    gen(vector_get_node(node->children, i));
  }
  for (int i = node->children->size - 1; i >= 0; i--) {
    printf("  ldr x%d, [sp], #16\n", i);
  }
  printf("  bl _%s\n", string_as_cstring(func->name));
}

void
gen_2op(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  Node* rhs = vector_get_node(node->children, 1);

  gen(lhs);
  gen(rhs);

  printf("  ldr x1, [sp], #16\n");
  printf("  ldr x0, [sp], #16\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add x0, x0, x1\n");
      return;
    case ND_SUB:
      printf("  sub x0, x0, x1\n");
      return;
    case ND_MUL:
      printf("  mul x0, x0, x1\n");
      return;
    case ND_DIV:
      printf("  sdiv x0, x0, x1\n");
      return;
    case ND_MOD:
      printf("  sdiv x2, x0, x1\n");
      printf("  msub x0, x2, x1, x0\n");
      return;
    case ND_EQ:
      printf("  cmp x0, x1\n");
      printf("  cset x0, EQ\n");
      return;
    case ND_NE:
      printf("  cmp x0, x1\n");
      printf("  cset x0, NE\n");
      return;
    case ND_LE:
      printf("  cmp x0, x1\n");
      printf("  cset x0, LE\n");
      return;
    case ND_LT:
      printf("  cmp x0, x1\n");
      printf("  cset x0, LT\n");
      return;
    case ND_SHIFT_L:
      printf("  lsl x0, x0, x1\n");
      return;
    case ND_SHIFT_R:
      printf("  lsr x0, x0, x1\n");
      return;
    case ND_AND:
      printf("  and x0, x0, x1\n");
      return;
    case ND_XOR:
      printf("  eor x0, x0, x1\n");
      return;
    case ND_OR:
      printf("  orr x0, x0, x1\n");
      return;
    case ND_LAND:
      printf("  cmp x0, #0\n");
      printf("  cset x0, NE\n");
      printf("  cmp x1, #0\n");
      printf("  cset x1, NE\n");
      printf("  and x0, x0, x1\n");
      return;
    case ND_LOR:
      printf("  cmp x0, #0\n");
      printf("  cset x0, NE\n");
      printf("  cmp x1, #0\n");
      printf("  cset x1, NE\n");
      printf("  orr x0, x0, x1\n");
      return;
    default:
      error("NodeKind is not supported %d", node->kind);
  }
}

void
gen_1op(Node* node)
{
  Node* lhs = vector_get_node(node->children, 0);
  gen_stmt(lhs);

  switch (node->kind) {
    case ND_LNOT:
      printf("  cmp x0, #0\n");
      printf("  cset x0, EQ\n");
      return;
    case ND_NOT:
      printf("  mvn x0, x0\n");
      return;
    case ND_DEREF:
      printf("  ldr x0, [x0]\n");
      return;
    case ND_SIZEOF:
      printf("  mov x0, 8\n"); // TODO
      return;
    default:
      error("NodeKind is not supported %d", node->kind);
  }
}

void
gen_stmt(Node* node)
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
gen(Node* node)
{
  if (node == NULL)
    return;
  gen_stmt(node);
  printf("  str x0, [sp, #-16]!\n");
}