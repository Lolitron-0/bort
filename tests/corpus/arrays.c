int main() {
  int x[5] = {5,4,3,2,1};

  int y = 0;

  while (y < 5) {
    x[y] = y;
    y    = y + 1;
  }
}
