int foo(int a, int b) {
  int x;
  if (a > b) {
    x = 4;
    return x;
  } else {
    x = 5;
  }

  return 5;
}

int main() {
  int y;
  foo(1, y);
  y = y + 1;
}
