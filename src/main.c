#include "codegen.h"
#include "instr.h"
#include "lex.h"
#include "parse.h"

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

  set_source_file(fd);
  token_value_t *val = try_parse_token_value(TOKEN_IDENTIFIER);
  if (val) {
    printf("%s\n", val->str);
  } else {
    printf("1 :(\n");
  }
  token_value_t *val2 = try_parse_token_value(TOKEN_IDENTIFIER);
  if (val2) {
    printf("%s\n", val2->str);
  } else {
    printf("2 :(\n");
  }
  if (try_parse_token(TOKEN_PAREN_LEFT)) {
    printf("1 Successfully parsed!\n");
  }
  if (try_parse_token(TOKEN_PAREN_LEFT)) {
    printf("2 Successfully parsed!\n");
  }
  if (try_parse_token(TOKEN_PAREN_RIGHT)) {
    printf("3 Successfully parsed!\n");
  }
  /* // Read 1K */
  /* char *program_contents = malloc(sizeof(char) * 1024); */
  /* fread(program_contents, sizeof(char), 1024, fd); */
  /* fclose(fd); */

  /* // Lex file */
  /* token_array_t *tokens = */
  /*     parse_tokens(program_contents, strlen(program_contents)); */
  /* free(program_contents); */

  /* // Print tokens */
  /* for (size_t i = 0; i < tokens->len; ++i) { */
  /*   print_token(tokens->data[i]); */
  /* } */
  /* printf("\n"); */

  /* char *tmp, *name = strtok(argv[1], "/"); */
  /* while (name != NULL) { */
  /*   if ((tmp = strtok(NULL, "/")) == NULL) { */
  /*     name = strtok(name, "."); */
  /*     break; */
  /*   } */
  /*   name = tmp; */
  /* } */

  /* char *object_name = calloc(strlen(name) + 3, sizeof(char)); */
  /* strcpy(object_name, name); */
  /* strcat(object_name, ".o"); */

  /* function_t **funcs = parse_ast(tokens); */

  /* gen_object(funcs, object_name); */

  return EXIT_SUCCESS;
}
