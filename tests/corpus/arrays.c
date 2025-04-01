int main() {
  // "hello";
  int x[3] = { 3, 2, 1 };

  int i = 0;
  while (i < 3) {
    x[i] = i;
    i    = i + 1;
  }
}
