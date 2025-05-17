#include <stdint.h>
#include <stdio.h>

extern int64_t dumlang(int64_t a);

int main() {
  //printf("%ld\n", dumlang(5));
  for (int i = -20; i < 20; ++i) {
    printf("%ld\n", dumlang(i));
  }
}
