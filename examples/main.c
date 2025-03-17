#include <stdint.h>
#include <stdio.h>

extern int64_t dumlangdivide(uint64_t dividend, uint64_t divisor);
extern int64_t dumlang();

int main() {
  printf("%ld\n", dumlangdivide(592835, 100));
  printf("%ld\n", dumlang());
}
