void
alloc_array2(char** p2, int n);
int
testcase()
{
  char* p;
  alloc_array2(&p, 4);
  char* q = p + 3;
  return *q;
}
