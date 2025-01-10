#ifndef __LEX_H
#define __LEX_H

#include <stddef.h>
#include <stdint.h>

extern char *token_type_names[];

typedef enum _token_type {
  TOKEN_NONE,
  TOKEN_ENDLINE,
  TOKEN_PAREN_LEFT,
  TOKEN_PAREN_RIGHT,
  TOKEN_BRACE_LEFT,
  TOKEN_BRACE_RIGHT,
  TOKEN_COMMA,
  TOKEN_TYPE_INT,
  TOKEN_OP_ADD,
  TOKEN_OP_SUB,
  TOKEN_OP_MUL,
  TOKEN_OP_DIV,
  TOKEN_OP_EQU,
  TOKEN_CMP_EQU,
  TOKEN_CMP_GT,
  TOKEN_CMP_GTE,
  TOKEN_CMP_LT,
  TOKEN_CMP_LTE,
  TOKEN_IDENTIFIER,
  TOKEN_INT,
} token_type_t;

typedef union _token_value {
  int32_t int32;
  char *str;
  void *none;
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

token_array_t *parse_tokens(char *str, size_t size);
void print_token(const token_t *);
token_t *array_get(token_array_t *arr, size_t index);

#endif // __LEX_H
