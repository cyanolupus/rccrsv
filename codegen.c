#include "reccursive.h"

void
gen(Node* node)
{
  if (node->kind == ND_NUM) {
    printf("  mov x9, %d\n", node->val);
    printf("  str x9, [sp, #-16]!\n");
    return;
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