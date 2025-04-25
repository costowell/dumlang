#include "parse.h"
#include "lex.h"

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

char error_msg[128] = {0};

#define ASSERT_TOKEN(TOKEN_TYPE)                                               \
  do {                                                                         \
    if (!try_parse_token(TOKEN_TYPE)) {                                        \
      gen_token_error(TOKEN_TYPE);                                             \
      goto fail;                                                               \
    }                                                                          \
  } while (0)

#define ASSERT_TOKEN_VALUE(TOKEN_TYPE)                                         \
  ({                                                                           \
    token_value_t *__assert_token_value_var =                                  \
        try_parse_token_value(TOKEN_TYPE);                                     \
    if (__assert_token_value_var == NULL) {                                    \
      gen_token_error(TOKEN_TYPE);                                             \
      goto fail;                                                               \
    }                                                                          \
    __assert_token_value_var;                                                  \
  })

#define ERRX(STATUS) errx(STATUS, "error: %s\n", error_msg)

void gen_token_error(token_type_t type) {
  snprintf(error_msg, sizeof(error_msg) - 1, "failed to parse token '%s'",
           token_type_names[type]);
}

type_t try_parse_type() {
  type_t type;
  long prevpos = lex_get_pos();
  if (try_parse_token(TOKEN_TYPE_INT)) {
    type = TYPE_INT64;
  } else {
    lex_set_pos(prevpos);
    return TYPE_NONE;
  }
  return type;
}

vartype_t *try_parse_vartype() {
  token_value_t *name;
  type_t type;
  long prevpos = lex_get_pos();

  name = ASSERT_TOKEN_VALUE(TOKEN_IDENTIFIER);
  ASSERT_TOKEN(TOKEN_COLON);
  type = try_parse_type();

  if (type == TYPE_NONE) {
    gen_token_error(TOKEN_TYPE_INT);
    goto fail;
  }

  vartype_t *vartype = malloc(sizeof(vartype_t));
  vartype->name = name->str;
  vartype->type = type;
  return vartype;

fail:
  lex_set_pos(prevpos);
  return NULL;
}

function_t *try_parse_func() {
  token_value_t *name;
  vartype_t **args = calloc(MAX_FUNC_ARGS, sizeof(vartype_t *));
  unsigned int argc = 0;
  long prevpos = lex_get_pos();

  ASSERT_TOKEN(TOKEN_AT);
  name = ASSERT_TOKEN_VALUE(TOKEN_IDENTIFIER);
  ASSERT_TOKEN(TOKEN_PAREN_LEFT);

  vartype_t *vartype;
  if (!try_parse_token(TOKEN_PAREN_RIGHT)) {
    vartype = try_parse_vartype();
    if (vartype == NULL)
      goto fail;
    args[argc++] = vartype;
    while (try_parse_token(TOKEN_COMMA)) {
      vartype = try_parse_vartype();
      if (vartype == NULL)
        goto fail;
      args[argc++] = vartype;
    }
    ASSERT_TOKEN(TOKEN_PAREN_RIGHT);
  }

  function_t *func = malloc(sizeof(function_t));
  func->name = name->str;
  func->args = args;
  return func;

fail:
  free(args);
  lex_set_pos(prevpos);
  return NULL;
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
