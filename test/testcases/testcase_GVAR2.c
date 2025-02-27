int x[10];

int
testcase()
{
  for (int i = 0; i < 10; i++) {
    x[i] = i;
  }
  return x[5];
}