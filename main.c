#include "reccursive.h"
#include <stdio.h>

int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Usage: %s <program>\n", argv[0]);
    return 1;
  }

  program = program_new();

  user_input = argv[1];
  Token* token = tokenize(argv[1]);
  add_node(program, &token);

  for (int i = 0; i < program->code->size; i++) {
    gen_stmt(vector_get_node(program->code, i));
  }
  return 0;
}