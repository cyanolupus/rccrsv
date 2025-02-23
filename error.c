#include "reccursive.h"

void
error(char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void
error_at(char* loc, char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void
error_at_until(char* loc, int len, char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  char* underline = malloc(len + 1);
  memset(underline, '^', len);
  underline[len] = '\0';

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "%s", underline);
  fprintf(stderr, " ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}