int a;
int b       = 5;
char str[5] = "abcd";
int* ptr;

int main() {
  a      = b;
  int x  = 3;
  ptr    = &x;
  int* gptr = &a;
  str[1] = 'e';
}
