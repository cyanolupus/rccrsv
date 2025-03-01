int
testcase()
{
  int a[10];
  a[0] = 1;
  for (int i = 1; i < 10; i++) {
    a[i] = i + a[i - 1];
  }
  return 9 [a];
}
