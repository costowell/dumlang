#include "lex.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *token_type_names[] = {[TOKEN_NONE] = "no token",
                                   [TOKEN_PAREN_LEFT] = "(",
                                   [TOKEN_PAREN_RIGHT] = ")",
                                   [TOKEN_BRACE_LEFT] = "{",
                                   [TOKEN_BRACE_RIGHT] = "}",
                                   [TOKEN_COMMA] = ",",
                                   [TOKEN_ENDLINE] = "END",
                                   [TOKEN_TYPE_INT] = "int_type",
                                   [TOKEN_OP_ADD] = "+",
                                   [TOKEN_OP_SUB] = "-",
                                   [TOKEN_OP_MUL] = "*",
                                   [TOKEN_OP_DIV] = "/",
                                   [TOKEN_OP_EQU] = "=",
                                   [TOKEN_CMP_EQU] = "==",
                                   [TOKEN_CMP_GTE] = ">=",
                                   [TOKEN_CMP_LTE] = "<=",
                                   [TOKEN_CMP_GT] = ">",
                                   [TOKEN_CMP_LT] = "<",
                                   [TOKEN_IDENTIFIER] = "identifier",
                                   [TOKEN_INT] = "int"};

token_array_t *array_init() {
  token_array_t *arr = malloc(sizeof(token_array_t));
  arr->alloc_len = 16;
  arr->data = calloc(arr->alloc_len, sizeof(token_t *));
  arr->len = 0;
  return arr;
}

void array_resize(token_array_t *arr) {
  arr->alloc_len = arr->alloc_len * 2;
  arr->data = realloc(arr->data, arr->alloc_len * sizeof(token_t *));
}

void array_push(token_array_t *arr, token_t *token) {
  if (arr->alloc_len == arr->len) {
    array_resize(arr);
  }
  arr->data[arr->len++] = token;
}

token_t *array_get(token_array_t *arr, size_t index) {
  if (index < arr->len) {
    return arr->data[index];
  } else {
    return NULL;
  }
}

bool is_valid_identifier(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_valid_number(char c) { return (c >= '0' && c <= '9'); }

void print_token(const token_t *token) {
  printf("('%s'", token_type_names[token->type]);
  switch (token->type) {
  case TOKEN_INT:
    printf(": %d", token->value.int32);
    break;
  case TOKEN_IDENTIFIER:
    printf(": \"%s\"", token->value.str);
    break;
  default:
    break;
  }
  printf(")");
}

/// TODO: parse a little less messily
/// Didn't want to 'cheat' and use regex, and this nightmare
/// can definitely be improved even without it
token_array_t *parse_tokens(char *str, size_t size) {
  token_array_t *arr = array_init();

  for (size_t i = 0; i < size; ++i) {
    size_t rem = size - i;
    token_type_t type = TOKEN_NONE;
    token_value_t value = {.none = NULL};

    if (rem >= 3 && strncmp(str + i, "int ", 3) == 0) {
      type = TOKEN_TYPE_INT;
      i += 3;
    } else if (is_valid_identifier(str[i])) {
      type = TOKEN_IDENTIFIER;
      size_t id_size = 0;
      while (id_size <= rem && is_valid_identifier(str[i + id_size++])) {
      }
      value.str = calloc(id_size, sizeof(char));
      strncpy(value.str, str + i, id_size - 1);
      i += id_size - 2;
    } else if (is_valid_number(str[i]) || str[i] == '-') {
      type = TOKEN_INT;
      int num = 0;
      int neg = str[i] == '-' ? -1 : 1;
      while (is_valid_number(str[i])) {
        num *= 10;
        num += str[i] - '0';
        i++;
      }
      i--;
      value.int32 = num * neg;
    } else {
      switch (str[i]) {
      case ';':
        type = TOKEN_ENDLINE;
        break;
      case '(':
        type = TOKEN_PAREN_LEFT;
        break;
      case ')':
        type = TOKEN_PAREN_RIGHT;
        break;
      case '{':
        type = TOKEN_BRACE_LEFT;
        break;
      case '}':
        type = TOKEN_BRACE_RIGHT;
        break;
      case ',':
        type = TOKEN_COMMA;
        break;
      case '=':
        if (str[i + 1] == '=')
          type = TOKEN_CMP_EQU;
        else
          type = TOKEN_OP_EQU;
        break;
      case '>':
        if (str[i + 1] == '=')
          type = TOKEN_CMP_GTE;
        else
          type = TOKEN_CMP_GT;
        break;
      case '<':
        if (str[i + 1] == '=')
          type = TOKEN_CMP_LTE;
        else
          type = TOKEN_CMP_LT;
        break;
      case '+':
        type = TOKEN_OP_ADD;
        break;
      case '-':
        type = TOKEN_OP_SUB;
        break;
      case '*':
        type = TOKEN_OP_MUL;
        break;
      case '/':
        type = TOKEN_OP_DIV;
        break;
      case '\n':
      case '\0':
      case ' ':
        break;
      default:
        fprintf(stderr, "Error: unrecognized char '%c'\n", str[i]);
        exit(EXIT_FAILURE);
        break;
      }
    }

    if (type == TOKEN_NONE) {
      continue;
    }

    token_t *token = malloc(sizeof(token_t));
    token->type = type;
    token->value = value;
    array_push(arr, token);
  }

  return arr;
}
