void foo(int* ptr) {
}

int main() {
  int x;
  x        = 1;
  int* ptr = &x;
  x        = 2;
  *ptr     = 3;
}
