int main() {
  int i;
  i = 0;
  while (i < 1000000) {
    i += 1;
  }

  for (int j = 0; j < 1000000; ++j) {
    i = i - 1;
  }
}
