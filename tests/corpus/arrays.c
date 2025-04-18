int x[5] = { 5, 4, 3, 2, 1 };
int y    = 0;

int main() {

  while (y < 5) {
    x[y] = y;
    y    = y + 1;
  }
}
