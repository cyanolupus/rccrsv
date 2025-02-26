void print_ptr(int *x);int testcase() { int a[10]; print_ptr(a); print_ptr(a + 0); a[0] = 1; for (int i=1; i<10; i=i+1) { a[i] = i + a[i-1]; } return a[0]; }
