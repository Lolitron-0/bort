void foo(int a, int b) {
  int x;
  if (a > b) {
    x = 4;
  } else {
    x = 5;
  }
}

int main() {
  int y;
  foo(1, y);
  y = y + 1;
}
