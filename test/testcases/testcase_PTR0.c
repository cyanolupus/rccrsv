int
testcase()
{
  int x;
  int* y;
  y = &x;
  *y = 3;
  return x;
}
