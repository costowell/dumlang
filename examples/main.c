#include <stdint.h>
#include <stdio.h>

extern int64_t dumlang(int64_t a);

int64_t dumlang_test(int64_t param) {
  int64_t i = 0;
  while (param > 3 && i <= 10) {
    param = param - 1;
    i = i + 1;
    if (param == 5) {
      param = param - 1;
      continue;
    }
    if (param != 5) {
      break;
    }
  }
  return i;
}

int main() {
  for (int64_t i = -20; i < 20; ++i) {
    printf("%ld: \t %ld \t %ld\n", i, dumlang(i), dumlang_test(i));
  }
}
