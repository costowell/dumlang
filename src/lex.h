#ifndef __LEX_H
#define __LEX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

extern char *token_type_names[];

typedef enum _token_type {
  TOKEN_NONE,
  TOKEN_EOF,
  TOKEN_AT,
  TOKEN_COMMA,
  TOKEN_COLON,
  TOKEN_SEMICOLON,
  TOKEN_PAREN_LEFT,
  TOKEN_PAREN_RIGHT,
  TOKEN_BRACE_LEFT,
  TOKEN_BRACE_RIGHT,
  TOKEN_OP_ADD,
  TOKEN_OP_SUB,
  TOKEN_OP_MUL,
  TOKEN_OP_DIV,
  TOKEN_OP_EQU,
  TOKEN_IDENTIFIER,
  TOKEN_INT,
  TOKEN_KW_RET,
  TOKEN_KW_DEC,
  TOKEN_TYPE_INT,
} token_type_t;

typedef union _token_value {
  int64_t int64;
  char *str;
} token_value_t;

typedef struct _token {
  token_type_t type;
  token_value_t value;
} token_t;

typedef struct _token_array {
  token_t **data;
  size_t alloc_len;
  size_t len;
} token_array_t;

typedef enum _try_error {
  TRY_OK,
  TRY_EOF,
  TRY_FAIL,
} try_error_t;

long lex_get_pos();
int lex_set_pos(long pos);
void set_source_file(FILE *fd);
bool try_parse_token(token_type_t type);
token_value_t *try_parse_token_value(token_type_t type);
token_array_t *parse_tokens(char *str, size_t size);
void print_token(const token_t *);
token_t *array_get(token_array_t *arr, size_t index);

#endif // __LEX_H
