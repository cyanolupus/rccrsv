void alloc_array(int **p2, int n); int testcase() { int *p; alloc_array(&p, 4); int *q = p + 3; int *r = q - 2; return *r; }
