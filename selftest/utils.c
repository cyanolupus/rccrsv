#include <stdio.h>
#include <stdlib.h>

void
ok()
{
  printf("\x1b[32mo\x1b[0m");
}

void
ng()
{
  printf("\x1b[31mx\x1b[0m");
}

int
foo()
{
  return 123;
}

int
bar(int a, int b)
{
  return a + b;
}

void
alloc_array(int** p, int n)
{
  *p = malloc(sizeof(int) * n);
  for (int i = 0; i < n; i++) {
    (*p)[i] = 1 << i;
  }
}

void
alloc_array2(char** p, int n)
{
  *p = malloc(n);
  for (int i = 0; i < n; i++) {
    (*p)[i] = i;
  }
}

void
print_int(int x)
{
  printf("%u\n", x);
}

void
print_ptr(int* p)
{
  printf("%p\n", p);
}

void
assert(int expected, int actual, char* name)
{
  if (expected != actual) {
    ng();
    printf("\n");
    fprintf(stderr, "%s: expected %d but got %d\n", name, expected, actual);
    exit(1);
  } else {
    ok();
  }
}
