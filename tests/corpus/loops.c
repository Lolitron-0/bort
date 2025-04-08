int main() {
  int i;
  i = 0;
  while (i < 1000000) {
    i += 1;

    if (i % 2374 == 0) {
      break;
    }
  }

  for (int j = 0; j < 1000000; ++j) {
    if (i % 2 == 0) {
      i -= 3;
      continue;
    }

    i = i - 1;
  }
}
