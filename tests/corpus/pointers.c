void foo(int* ptr) {
  *ptr = 1;
}

int main() {
  char str[4] = "abc";
  int x;
  if (1) {
    x = 1;
  }
  int* ptr   = &x;
  x          = 2;
  *(ptr - 4) = 3;
}
