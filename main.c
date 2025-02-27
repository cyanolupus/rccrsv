#include "rccrsv.h"

void
print_help(char* argv[])
{
  fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -o <output_path>  Output file path\n");
  fprintf(stderr, "  -h                Show help\n");
}

int
main(int argc, char** argv)
{
  if (argc < 2) {
    print_help(argv);
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-o") == 0) {
      if (i + 1 < argc) {
        output_path = argv[i + 1];
        i++;
      } else {
        print_help(argv);
        return 1;
      }
    } else if (strcmp(argv[i], "-h") == 0) {
      print_help(argv);
      return 0;
    } else {
      input_path = argv[i];
    }
  }

  FILE* fp = fopen(input_path, "r");
  if (!fp) {
    error("cannot open %s", argv[1]);
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char* buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);
  fclose(fp);

  if (strncmp(output_path, "-", 1) == 0) {
    output_fp = stdout;
  } else {
    output_fp = fopen(output_path, "w");
    if (!output_fp) {
      error("cannot open %s", output_path);
    }
    fclose(output_fp);
    output_fp = fopen(output_path, "a");
    if (!output_fp) {
      error("cannot open %s", output_path);
    }
  }

  program = program_new();

  user_input = buf;
  Tokens* tokens = tokenize(buf);

  add_node(program, tokens);

  gen_code(program);

  fclose(output_fp);
  return 0;
}