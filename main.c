#include "reccursive.h"
#include <stdio.h>

int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Usage: %s <program>\n", argv[0]);
    return 1;
  }

  program = new_program();

  user_input = argv[1];
  Token* token = tokenize(argv[1]);
  add_node(program, &token);

  for (int i = 0; i < program->len; i++) {
    gen_stmt(program->code[i]);
  }
  return 0;
}