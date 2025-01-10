#include "lex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_help() { printf("usage: dumc [file]\n"); }

int main(int argc, char **argv) {
  if (argc != 2) {
    print_help();
    return EXIT_FAILURE;
  }

  FILE *fd = fopen(argv[1], "r");
  if (fd == NULL) {
    perror("Failed to read file");
    return EXIT_FAILURE;
  }

  // Read 1K
  char *program_contents = malloc(sizeof(char) * 1024);
  fread(program_contents, sizeof(char), 1024, fd);

  // Lex file
  token_array_t *tokens =
      parse_tokens(program_contents, strlen(program_contents));
  free(program_contents);

  // Print tokens
  for (size_t i = 0; i < tokens->len; ++i) {
    print_token(tokens->data[i]);
  }
  printf("\n");

  return EXIT_SUCCESS;
}
