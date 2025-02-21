#include "reccursive.h"
#include <stdio.h>

void
gen_lval(Node* node)
{
  if (node->kind != ND_LVAR)
    error("Not a variable on the left side of the assignment");

  printf("  mov x0, fp\n");
  printf("  sub x0, x0, #%d\n", node->offset);
  printf("  str x0, [sp, #-16]!\n");
}

void
gen(Node* node)
{
  switch (node->kind) {
    case ND_NUM:
      printf("  mov x0, %d\n", node->val);
      printf("  str x0, [sp, #-16]!\n");
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  ldr x0, [sp], #16\n");
      printf("  ldr x0, [x0]\n");
      printf("  str x0, [sp, #-16]!\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  ldr x1, [sp], #16\n");
      printf("  ldr x0, [sp], #16\n");
      printf("  str x1, [x0]\n");
      printf("  str x1, [sp, #-16]!\n");
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  ldr x0, [sp], #16\n");
      printf("  mov sp, fp\n");
      printf("  ldp fp, lr, [sp], #16\n");
      printf("  ret\n");
      return;
    default:
      break;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  ldr x1, [sp], #16\n");
  printf("  ldr x0, [sp], #16\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add x0, x0, x1\n");
      break;
    case ND_SUB:
      printf("  sub x0, x0, x1\n");
      break;
    case ND_MUL:
      printf("  mul x0, x0, x1\n");
      break;
    case ND_DIV:
      printf("  sdiv x0, x0, x1\n");
      break;
    case ND_EQ:
      printf("  cmp x0, x1\n");
      printf("  cset x0, EQ\n");
      break;
    case ND_NE:
      printf("  cmp x0, x1\n");
      printf("  cset x0, NE\n");
      break;
    case ND_LE:
      printf("  cmp x0, x1\n");
      printf("  cset x0, LE\n");
      break;
    case ND_LT:
      printf("  cmp x0, x1\n");
      printf("  cset x0, LT\n");
      break;
    default:
      error("NodeKind is not supported %d", node->kind);
  }

  printf("  str x0, [sp, #-16]!\n");
}