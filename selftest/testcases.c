void
alloc_array(int** p2, int n);

void
alloc_array2(char** p2, int n);

void
assert(int expected, int actual, char* name);

int
foo();

int
bar(int a, int b);

int
testcase_ADDR_ADD0()
{
  int* p;
  alloc_array(&p, 4);
  int* q = p + 2;
  return *q;
}

int
testcase_ADDR_ADD1()
{
  int* p;
  alloc_array(&p, 4);
  int* q = p + 3;
  return *q;
}

int
testcase_ADDR_ADD2()
{
  char* p;
  alloc_array2(&p, 4);
  char* q = p + 3;
  return *q;
}

int
testcase_ADDR_SUB0()
{
  int* p;
  alloc_array(&p, 4);
  int* q = p + 3;
  int* r = q - 2;
  return *r;
}

int
a(int x)
{
  return x * 4;
}

int
testcase_ARG0()
{
  return a(1);
}

int
b(int x, int y)
{
  return x / y;
}
int
testcase_ARG1()
{
  return b(9, 3);
}

int
testcase_ARRAY0()
{
  int a[2];
  *a = 1;
  *(a + 1) = 2;
  int* p = a;
  return *p + *(p + 1);
}

int
testcase_ARRAY1()
{
  int a[10];
  a[0] = 1;
  for (int i = 1; i < 10; i++) {
    a[i] = i + a[i - 1];
  }
  return a[9];
}

int
testcase_ARRAY2()
{
  int a[10];
  a[0] = 1;
  for (int i = 1; i < 10; i++) {
    a[i] = i + a[i - 1];
  }
  return 9 [a];
}

int
testcase_RECCURSIVE(int n, int from, int to, int work)
{
  if (n == 0)
    return 0;
  int ret = 0;
  ret += testcase_RECCURSIVE(n - 1, from, work, to);
  ret += 1;
  ret += testcase_RECCURSIVE(n - 1, work, to, from);
  return ret;
}

int
testcase_ASSIGN_ADD()
{
  int a = 1;
  a += 2;
  return a;
}

int
testcase_ASSIGN_SUB()
{
  int a = 3;
  a -= 2;
  return a;
}

int
testcase_ASSIGN_AND()
{
  int a = 15;
  a &= 3;
  return a;
}

int
testcase_ASSIGN_OR()
{
  int a = 8;
  a |= 4;
  return a;
}

int
testcase_ASSIGN_XOR()
{
  int a = 15;
  a ^= 3;
  return a;
}

int
testcase_ASSIGN_DIV()
{
  int a = 6;
  a /= 3;
  return a;
}

int
testcase_ASSIGN_LSHIFT()
{
  int a = 1;
  a <<= 3;
  return a;
}

int
testcase_ASSIGN_RSHIFT()
{
  int a = 8;
  a >>= 3;
  return a;
}

int
testcase_ASSIGN_MOD()
{
  int a = 5;
  a %= 3;
  return a;
}

int
testcase_ASSIGN_MUL()
{
  int a = 2;
  a *= 3;
  return a;
}

int
testcase_CHAR()
{
  char x[3];
  x[0] = -1;
  x[1] = 2;
  int y;
  y = 4;
  return x[0] + y;
}

int
testcase_COMMENT()
{
  int a = 5;
  int b = 3; // b = 4;
  int c = 2;
  return a /* + c */ + b;
}

int
testcase_CONDITIONAL0()
{
  return 1 ? 123 : 12;
}

int
testcase_CONDITIONAL1()
{
  return 0 ? 123 : 12;
}

int
testcase_CONDITIONAL2()
{
  return 0 ? 123 : 12 ? 1 : 2;
}

int
testcase_EXTERNAL_FUNC0()
{
  return foo();
}

int
testcase_EXTERNAL_FUNC1()
{
  return foo() + bar(3, 5);
}

int
testcase_FOR_BLOCK()
{
  int a = 1;
  for (int i = 0; i < 3; i = i + 1) {
    a = a * 2;
  }
  return a;
}

int x;

int
testcase_GVAR0()
{
  x = 3;
  return x;
}

int x2 = 3;

int
testcase_GVAR1()
{
  int x2 = 5;
  return x2;
}

int x3[10];

int
testcase_GVAR2()
{
  for (int i = 0; i < 10; i++) {
    x3[i] = i;
  }
  return x3[5];
}

int
testcase_IF_BLOCK()
{
  if (1) {
    return 1;
  } else {
    return 2;
  }
}

int
testcase_ADD()
{
  return 1 + 1;
}

int
testcase_MOD()
{
  return 5 % 2;
}

int
testcase_LAND()
{
  return 1 && 0;
}

int
testcase_LOR()
{
  return 1 || 0;
}

int
testcase_LNOT()
{
  return !(1 != 1);
}

int
testcase_AND()
{
  return 15 & 3;
}

int
testcase_OR()
{
  return 8 | 4 | 2 | 1;
}

int
testcase_NOT()
{
  if (~0 != 1) {
    return 123;
  } else {
    return 12;
  }
}

int
a2()
{
  return 3 * 4;
}

int
testcase_MULFUNC0()
{
  return a2();
}

int
testcase_MULFUNC1()
{
  return a2() + a2();
}

int
b2()
{
  return 5 / 2;
}
int
testcase_MULFUNC2()
{
  return a2() + b2();
}

int
testcase_POSTINCREMENT()
{
  int a = 1;
  if (a++ == 1) {
    return 123 + a;
  } else {
    return 12 + a;
  }
}

int
testcase_PREINCREMENT()
{
  int a = 1;
  if (++a == 2) {
    return 123 + a;
  } else {
    return 12 + a;
  }
}

int
testcase_PTR0()
{
  int x;
  int* y;
  y = &x;
  *y = 3;
  return x;
}

int
testcase_REFDEREF()
{
  int a = 1;
  int* b = &a;
  int c = *b;
  return c;
}

int
testcase_SHIFT()
{
  return 1 << 3;
}

int
testcase_SIZEOF0()
{
  int x = 1;
  return sizeof(x);
}

int
testcase_SIZEOF1()
{
  long long x;
  return sizeof(x);
}

int
testcase_SIZEOF2()
{
  int* x;
  return sizeof(x);
}

int
testcase_STR()
{
  char* x = "cafe";
  char* y = "abc";
  return x[0];
}

int
testcase_WHILE_RETURN()
{
  int a = 1;
  while (1) {
    a = a * 2;
    if (a > 4) {
      return a;
    }
  }
}

int
testcase_XOR()
{
  return 15 ^ 3;
}

int
main()
{
  assert(4, testcase_ADDR_ADD0(), "testcase_ADDR_ADD0");
  assert(8, testcase_ADDR_ADD1(), "testcase_ADDR_ADD1");
  assert(3, testcase_ADDR_ADD2(), "testcase_ADDR_ADD2");
  assert(2, testcase_ADDR_SUB0(), "testcase_ADDR_SUB0");
  assert(4, testcase_ARG0(), "testcase_ARG0");
  assert(3, testcase_ARG1(), "testcase_ARG1");
  assert(3, testcase_ARRAY0(), "testcase_ARRAY0");
  assert(46, testcase_ARRAY1(), "testcase_ARRAY1");
  assert(46, testcase_ARRAY2(), "testcase_ARRAY2");
  assert(3, testcase_ASSIGN_ADD(), "testcase_ASSIGN_ADD");
  assert(1, testcase_ASSIGN_SUB(), "testcase_ASSIGN_SUB");
  assert(6, testcase_ASSIGN_MUL(), "testcase_ASSIGN_MUL");
  assert(2, testcase_ASSIGN_DIV(), "testcase_ASSIGN_DIV");
  assert(2, testcase_ASSIGN_MOD(), "testcase_ASSIGN_MOD");
  assert(3, testcase_ASSIGN_AND(), "testcase_ASSIGN_AND");
  assert(12, testcase_ASSIGN_OR(), "testcase_ASSIGN_OR");
  assert(12, testcase_ASSIGN_XOR(), "testcase_ASSIGN_XOR");
  assert(8, testcase_ASSIGN_LSHIFT(), "testcase_ASSIGN_LSHIFT");
  assert(1, testcase_ASSIGN_RSHIFT(), "testcase_ASSIGN_RSHIFT");
  assert(3, testcase_CHAR(), "testcase_CHAR");
  assert(123, testcase_CONDITIONAL0(), "testcase_CONDITIONAL0");
  assert(12, testcase_CONDITIONAL1(), "testcase_CONDITIONAL1");
  assert(1, testcase_CONDITIONAL2(), "testcase_CONDITIONAL2");
  assert(123, testcase_EXTERNAL_FUNC0(), "testcase_EXTERNAL_FUNC0");
  assert(131, testcase_EXTERNAL_FUNC1(), "testcase_EXTERNAL_FUNC1");
  assert(8, testcase_FOR_BLOCK(), "testcase_FOR_BLOCK");
  assert(3, testcase_GVAR0(), "testcase_GVAR0");
  assert(5, testcase_GVAR1(), "testcase_GVAR1");
  assert(5, testcase_GVAR2(), "testcase_GVAR2");
  assert(1, testcase_IF_BLOCK(), "testcase_IF_BLOCK");
  assert(2, testcase_ADD(), "testcase_ADD");
  assert(1, testcase_MOD(), "testcase_MOD");
  assert(0, testcase_LAND(), "testcase_LAND");
  assert(1, testcase_LOR(), "testcase_LOR");
  assert(1, testcase_LNOT(), "testcase_LNOT");
  assert(3, testcase_AND(), "testcase_AND");
  assert(15, testcase_OR(), "testcase_OR");
  assert(123, testcase_NOT(), "testcase_NOT");
  assert(12, testcase_MULFUNC0(), "testcase_MULFUNC0");
  assert(24, testcase_MULFUNC1(), "testcase_MULFUNC1");
  assert(14, testcase_MULFUNC2(), "testcase_MULFUNC2");
  assert(125, testcase_POSTINCREMENT(), "testcase_POSTINCREMENT");
  assert(125, testcase_PREINCREMENT(), "testcase_PREINCREMENT");
  assert(3, testcase_PTR0(), "testcase_PTR0");
  assert(1, testcase_REFDEREF(), "testcase_REFDEREF");
  assert(8, testcase_SHIFT(), "testcase_SHIFT");
  assert(4, testcase_SIZEOF0(), "testcase_SIZEOF0");
  assert(8, testcase_SIZEOF1(), "testcase_SIZEOF1");
  assert(8, testcase_SIZEOF2(), "testcase_SIZEOF2");
  assert(99, testcase_STR(), "testcase_STR");
  assert(8, testcase_WHILE_RETURN(), "testcase_WHILE_RETURN");
  assert(12, testcase_XOR(), "testcase_XOR");

  assert(1, testcase_RECCURSIVE(1, 0, 1, 2), "testcase_RECCURSIVE_1012");
  assert(3, testcase_RECCURSIVE(2, 0, 1, 2), "testcase_RECCURSIVE_2012");
  assert(7, testcase_RECCURSIVE(3, 0, 1, 2), "testcase_RECCURSIVE_3012");

  return 0;
}