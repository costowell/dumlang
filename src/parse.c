#include "parse.h"
#include "lex.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

char error_msg[128] = {0};
long stable_pos = 0;

#define ASSERT_TOKEN(TOKEN_TYPE)                                               \
  do {                                                                         \
    if (!try_parse_token(TOKEN_TYPE)) {                                        \
      gen_token_error(TOKEN_TYPE);                                             \
      return NULL;                                                             \
    }                                                                          \
  } while (0)

#define ASSERT_TOKEN_VALUE(TOKEN_TYPE)                                         \
  ({                                                                           \
    token_value_t *__assert_token_value_var =                                  \
        try_parse_token_value(TOKEN_TYPE);                                     \
    if (__assert_token_value_var == NULL) {                                    \
      gen_token_error(TOKEN_TYPE);                                             \
      return NULL;                                                             \
    }                                                                          \
    __assert_token_value_var;                                                  \
  })

#define NULL_RESTORE(CODE...)                                                  \
  ({                                                                           \
    void *_null_restore = CODE;                                                \
    if (_null_restore == NULL) {                                               \
      lex_set_pos(stable_pos);                                                 \
    }                                                                          \
    _null_restore;                                                             \
  })

#define ERRX(STATUS) errx(STATUS, "error: %s\n", error_msg)

#define TRANSACTION(CODE...)                                                   \
  do {                                                                         \
    lex_set_pos(stable_pos);                                                   \
    do                                                                         \
      CODE while (0);                                                          \
    stable_pos = lex_get_pos();                                                \
  } while (0)

void gen_token_error(token_type_t type) {
  snprintf(error_msg, sizeof(error_msg) - 1, "failed to parse token '%s'",
           token_type_names[type]);
}

type_t try_parse_type() {
  if (try_parse_token(TOKEN_TYPE_INT)) {
    return TYPE_INT64;
  } else {
    return TYPE_NONE;
  }
}

vartype_t *try_parse_vartype() {
  token_value_t *name = ASSERT_TOKEN_VALUE(TOKEN_IDENTIFIER);
  ASSERT_TOKEN(TOKEN_COLON);
  type_t type = try_parse_type();

  if (type == TYPE_NONE) {
    gen_token_error(TOKEN_TYPE_INT);
    return NULL;
  }

  vartype_t *vartype = malloc(sizeof(vartype_t));
  vartype->name = name->str;
  vartype->type = type;
  return vartype;
}

function_t *try_parse_func() {
  token_value_t *name;
  vartype_t **args = calloc(MAX_FUNC_ARGS, sizeof(vartype_t *));
  unsigned int argc = 0;
  TRANSACTION({
    ASSERT_TOKEN(TOKEN_AT);
    name = ASSERT_TOKEN_VALUE(TOKEN_IDENTIFIER);
    ASSERT_TOKEN(TOKEN_PAREN_LEFT);

    vartype_t *vartype;
    if (!try_parse_token(TOKEN_PAREN_RIGHT)) {
      vartype = try_parse_vartype();
      if (vartype == NULL)
        ERRX(EXIT_FAILURE);
      args[argc++] = vartype;
      while (try_parse_token(TOKEN_COMMA)) {
        vartype = try_parse_vartype();
        if (vartype == NULL)
          ERRX(EXIT_FAILURE);
        args[argc++] = vartype;
      }
      ASSERT_TOKEN(TOKEN_PAREN_RIGHT);
    }
  });

  function_t *func = malloc(sizeof(function_t));
  func->name = name->str;
  func->args = args;
  return func;
}

function_t **try_parse_ast() {
  function_t **funcs = calloc(10, sizeof(function_t *));
  unsigned int i = 0;

  while (!try_parse_token(TOKEN_EOF)) {
    function_t *func = try_parse_func();
    if (func == NULL)
      ERRX(EXIT_FAILURE);
    funcs[i++] = func;
  }
  return funcs;
}
