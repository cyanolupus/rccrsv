#include "reccursive.h"
#include <stdio.h>

int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Usage: %s <program>\n", argv[0]);
    return 1;
  }

  user_input = argv[1];
  Token* token = tokenize(argv[1]);

  Program* program = new_program(&token);

  printf(".global _main\n");
  printf("_main:\n");

  printf("  stp fp, lr, [sp, #-16]!\n");
  printf("  mov fp, sp\n");
  printf("  sub sp, sp, 208\n");

  for (int i = 0; i < program->len; i++) {
    gen(program->code[i]);
    printf("  ldr x0, [sp], #16\n");
  }

  printf("  mov sp, fp\n");
  printf("  ldp fp, lr, [sp], #16\n");
  printf("  ret\n");
  return 0;
}