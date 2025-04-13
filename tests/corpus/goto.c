int main() {
  int x = 3;
  int y;
  y = x >> 2;

  if (y < x) {
    goto my_label;
  }
  goto my_label2;

my_label:;
  y = x;

my_label2:;
  x = y;
}
