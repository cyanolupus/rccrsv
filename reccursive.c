#include "reccursive.h"

int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Usage: %s <expr>\n", argv[0]);
    return 1;
  }

  Token* token;
  token = tokenize(argv[1]);
  user_input = argv[1];

  Node* node = expr(&token);

  printf(".global _main\n");
  printf("_main:\n");

  gen(node);

  printf("  ldr x0, [sp], #16\n");
  printf("  ret\n");
  return 0;
}