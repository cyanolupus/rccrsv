int testcase() { int a[2]; *a = 1; *(a+1) = 2; int *p = a; return *p + *(p+1); }
