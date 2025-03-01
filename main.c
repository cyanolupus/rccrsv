#include "rccrsv.h"

void
print_help(char* argv[])
{
  fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -o <output_path>  Output file path\n");
  fprintf(stderr, "  -h                Show help\n");
}

char*
read_file(const char* path)
{
  FILE* fp = fopen(path, "r");
  if (!fp) {
    fprintf(stderr, "cannot open %s\n", path);
    exit(1);
  }
  if (fseek(fp, 0, SEEK_END) == -1) {
    fprintf(stderr, "fseek error\n");
    exit(1);
  }
  size_t size = ftell(fp);
  if (fseek(fp, 0, SEEK_SET) == -1) {
    fprintf(stderr, "fseek error\n");
    exit(1);
  }
  char* buf = calloc(1, size + 2);
  fread(buf, size, 1, fp);

  if (size == 0 || buf[size - 1] != '\n')
    buf[size++] = '\n';
  buf[size] = '\0';
  fclose(fp);
  return buf;
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

  if (strncmp(output_path, "-", 1) == 0) {
    output_fp = stdout;
  } else {
    output_fp = fopen(output_path, "w");
    if (!output_fp) {
      fprintf(stderr, "cannot open %s\n", output_path);
      exit(1);
    }
    fclose(output_fp);
    output_fp = fopen(output_path, "a");
    if (!output_fp) {
      fprintf(stderr, "cannot open %s\n", output_path);
      exit(1);
    }
  }

  program = program_new();

  char* buf = read_file(input_path);
  user_input = buf;
  Tokens* tokens = tokenize(buf);

  add_node(program, tokens);

  gen_code(program);

  fclose(output_fp);
  return 0;
}