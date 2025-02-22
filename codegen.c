#include "reccursive.h"
#include <stdio.h>

void
gen_lval(Node* node)
{
  if (node->kind != ND_LVAR)
    error("Not a variable on the left side of the assignment");

  printf("  mov x9, fp\n");
  printf("  sub x9, x9, #%d\n", node->lvar->offset);
  printf("  str x9, [sp, #-16]!\n");
}

void
gen_stmt(Node* node)
{
  int curcnt;
  if (node == NULL)
    return;
  switch (node->kind) {
    case ND_NUM:
      printf("  mov x0, %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  ldr x0, [sp], #16\n");
      printf("  ldr x0, [x0]\n");
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
      printf("  add sp, sp, %d\n", 128);
      printf("  mov sp, fp\n");
      printf("  ldp fp, lr, [sp], #16\n");
      printf("  ret\n");
      return;
    case ND_IF:
      curcnt = jmpcnt++;
      gen(node->lhs);
      printf("  ldr x0, [sp], #16\n");
      printf("  cmp x0, #0\n");

      if (node->rrhs) {
        printf("  beq .Lelse%d\n", curcnt);
        gen(node->rhs);
        printf("  ldr x0, [sp], #16\n");
        printf("  b .Lend%d\n", curcnt);
        printf(".Lelse%d:\n", curcnt);
        gen_stmt(node->rrhs);
        printf(".Lend%d:\n", curcnt);
        printf("  str x0, [sp, #-16]!\n");
      } else {
        printf("  beq .Lend%d\n", curcnt);
        gen_stmt(node->rhs);
        printf(".Lend%d:\n", curcnt);
        printf("  str x0, [sp, #-16]!\n");
      }
      return;
    case ND_FOR:
      curcnt = jmpcnt++;
      gen(node->lhs);
      printf("  ldr x0, [sp], #16\n");
      printf(".Lbegin%d:\n", curcnt);
      gen(node->rhs);
      printf("  ldr x0, [sp], #16\n");
      printf("  cmp x0, #0\n");
      printf("  beq .Lend%d\n", curcnt);
      gen_stmt(node->rrrhs);
      gen_stmt(node->rrhs);
      printf("  b .Lbegin%d\n", curcnt);
      printf(".Lend%d:\n", curcnt);
      printf("  str x0, [sp, #-16]!\n");
      return;
    case ND_BLOCK:
      for (Node* cur = node; cur; cur = cur->rhs) {
        gen_stmt(cur->lhs);
      }
      return;
    case ND_FUNC:
      printf(".global _%.*s\n", node->lvar->len, node->lvar->name);
      printf("_%.*s:\n", node->lvar->len, node->lvar->name);
      printf("  stp fp, lr, [sp, #-16]!\n");
      printf("  mov fp, sp\n");
      printf("  sub sp, sp, %d\n", 128);
      int num = 0;
      if (node->lvar->next) {
        for (LVar* cur = node->lvar->next->next; cur; cur = cur->next) {
          gen_lval(node_new_lvar(cur));
          printf("  ldr x9, [sp], #16\n");
          printf("  str x%d, [x9]\n", num);
          num++;
        }
      }
      for (Node* cur = node; cur; cur = cur->rhs) {
        gen_stmt(cur->lhs);
      }
      return;
    case ND_CALL:
      for (int i = 0; i < node->argc; i++) {
        printf("  mov x%d, %d\n", i, node->argv[i]);
      }
      char* funcname = calloc(node->lvar->len + 1, sizeof(char));
      strncpy(funcname, node->lvar->name, node->lvar->len);
      funcname[node->lvar->len] = '\0';
      printf("  bl _%s\n", funcname);
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