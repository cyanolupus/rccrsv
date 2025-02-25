#include <stdio.h>
#include <stdlib.h>

int
foo()
{
  printf("Hello, World!\n");
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
print_int(int x)
{
  printf("%u\n", x);
}

void
print_ptr(int* p)
{
  printf("%p\n", p);
}