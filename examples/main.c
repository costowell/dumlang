#include <stdint.h>
#include <stdio.h>

extern int64_t dumlang();
extern int64_t dumlangb();

int main() {
  printf("%ld\n", dumlang());
  printf("%ld\n", dumlangb());
}
