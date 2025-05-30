#include "parse.h"
#include "lex.h"

#include <err.h>
#include <stdint.h>
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

expression_t *try_parse_expression();
arith_expression_t *try_parse_arith_expression();
code_block_t *try_parse_code_block();

void gen_token_error(token_type_t type) {
  snprintf(error_msg, sizeof(error_msg) - 1,
           "failed to parse token '%s' at char %ld", token_type_names[type],
           lex_get_pos());
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

bool try_parse_arith_op(arith_operator_t *op) {
  long prevpos = lex_get_pos();
  if (try_parse_token(TOKEN_OP_ADD)) {
    *op = ARITH_OP_ADD;
  } else if (try_parse_token(TOKEN_OP_SUB)) {
    *op = ARITH_OP_SUB;
  } else if (try_parse_token(TOKEN_OP_MUL)) {
    *op = ARITH_OP_MUL;
  } else if (try_parse_token(TOKEN_OP_DIV)) {
    *op = ARITH_OP_DIV;
  } else {
    lex_set_pos(prevpos);
    return false;
  }
  return true;
}

bool try_parse_cmp_op(cmp_operator_t *op) {
  long prevpos = lex_get_pos();
  if (try_parse_token(TOKEN_CMP_EQU)) {
    *op = CMP_OP_EQU;
  } else if (try_parse_token(TOKEN_CMP_GTE)) {
    *op = CMP_OP_GTE;
  } else if (try_parse_token(TOKEN_CMP_LTE)) {
    *op = CMP_OP_LTE;
  } else if (try_parse_token(TOKEN_CMP_NEQ)) {
    *op = CMP_OP_NEQ;
  } else if (try_parse_token(TOKEN_CMP_GT)) {
    *op = CMP_OP_GT;
  } else if (try_parse_token(TOKEN_CMP_LT)) {
    *op = CMP_OP_LT;
  } else {
    lex_set_pos(prevpos);
    return false;
  }
  return true;
}

bool try_parse_bool_op(bool_operator_t *op) {
  long prevpos = lex_get_pos();
  if (try_parse_token(TOKEN_LOG_AND)) {
    *op = BOOL_OP_AND;
  } else if (try_parse_token(TOKEN_LOG_OR)) {
    *op = BOOL_OP_OR;
  } else if (try_parse_token(TOKEN_LOG_NEG)) {
    *op = BOOL_OP_NOT;
  } else {
    lex_set_pos(prevpos);
    return false;
  }
  return true;
}

uint8_t bool_op_precedence(bool_operator_t op) {
  switch (op) {
  case BOOL_OP_NOT:
    return 3;
  case BOOL_OP_AND:
    return 2;
  case BOOL_OP_OR:
    return 1;
  }
  errx(EXIT_FAILURE, "bool_op_precedence(): unknown operator");
}

uint8_t arith_op_precedence(arith_operator_t op) {
  switch (op) {
  case ARITH_OP_MUL:
  case ARITH_OP_DIV:
    return 2;
  case ARITH_OP_ADD:
  case ARITH_OP_SUB:
    return 1;
  }
  errx(EXIT_FAILURE, "arith_op_precedence(): unknown operator");
}

func_call_t *try_parse_func_call() {
  long prevpos = lex_get_pos();
  token_value_t *name;

  expression_t **args = calloc(MAX_FUNC_ARGS, sizeof(expression_t *));
  unsigned int argc = 0;

  name = ASSERT_TOKEN_VALUE(TOKEN_IDENTIFIER);
  ASSERT_TOKEN(TOKEN_PAREN_LEFT);

  expression_t *expr;
  if (!try_parse_token(TOKEN_PAREN_RIGHT)) {
    if ((expr = try_parse_expression()) == NULL)
      goto fail;
    args[argc++] = expr;
    while (try_parse_token(TOKEN_COMMA)) {
      if ((expr = try_parse_expression()) == NULL)
        goto fail;
      args[argc++] = expr;
    }
    ASSERT_TOKEN(TOKEN_PAREN_RIGHT);
  }
  func_call_t *call = malloc(sizeof(func_call_t));
  call->name = name->str;
  call->args = args;
  return call;
fail:
  lex_set_pos(prevpos);
  return NULL;
}

arith_expression_t *try_parse_arith_atom() {
  long prevpos = lex_get_pos();
  arith_expression_t *expr = malloc(sizeof(arith_expression_t));
  token_value_t *value;
  func_call_t *call;
  if ((call = try_parse_func_call()) != NULL) {
    expr->instance.func_call = call;
    expr->type = ARITH_FUNC_CALL;
  } else if ((value = try_parse_token_value(TOKEN_IDENTIFIER)) != NULL) {
    expr->instance.name = value->str;
    expr->type = ARITH_IDENT;
  } else if ((value = try_parse_token_value(TOKEN_INT)) != NULL) {
    expr->instance.int64 = value->int64;
    expr->type = ARITH_NUM;
  } else if (try_parse_token(TOKEN_PAREN_LEFT)) {
    arith_expression_t *subexpr;
    if ((subexpr = try_parse_arith_expression()) == NULL)
      goto fail;
    ASSERT_TOKEN(TOKEN_PAREN_RIGHT);

    expr->instance.expr = subexpr;
    expr->type = ARITH_EXPR;
  } else {
    goto fail;
  }
  return expr;
fail:
  free(expr);
  lex_set_pos(prevpos);
  return NULL;
}

// Pratt parsing!
arith_expression_t *try_parse_arith_expression_bp(uint8_t min_prec) {
  long prevpos = lex_get_pos();
  arith_expression_t *lhs;
  arith_expression_t *rhs;

  if ((lhs = try_parse_arith_atom()) == NULL)
    goto fail;

  while (true) {
    arith_operator_t oprtr;
    long back = lex_get_pos();
    if (!try_parse_arith_op(&oprtr)) {
      lex_set_pos(back);
      break;
    }

    uint8_t prec = arith_op_precedence(oprtr);
    if (prec <= min_prec) {
      lex_set_pos(back);
      break;
    }

    if ((rhs = try_parse_arith_expression_bp(prec)) == NULL)
      goto fail;

    arith_operation_t *op = malloc(sizeof(arith_operation_t));
    op->op = oprtr;
    op->lhs = lhs;
    op->rhs = rhs;
    lhs = malloc(sizeof(arith_expression_t));
    lhs->type = ARITH_OP;
    lhs->instance.op = op;
  }
  return lhs;
fail:
  lex_set_pos(prevpos);
  return NULL;
}

arith_expression_t *try_parse_arith_expression() {
  return try_parse_arith_expression_bp(0);
}

cmp_operation_t *try_parse_cmp_operation() {
  long prevpos = lex_get_pos();
  arith_expression_t *lhs;
  arith_expression_t *rhs;
  cmp_operator_t op;
  if ((lhs = try_parse_arith_expression()) == NULL)
    goto fail;
  if (!try_parse_cmp_op(&op))
    goto fail;
  if ((rhs = try_parse_arith_expression()) == NULL)
    goto fail;
  cmp_operation_t *cmp = malloc(sizeof(cmp_operation_t));
  cmp->lhs = lhs;
  cmp->rhs = rhs;
  cmp->op = op;
  return cmp;
fail:
  lex_set_pos(prevpos);
  return NULL;
}

expression_t *try_parse_expr_atom() {
  long prevpos = lex_get_pos();
  expression_t *expr = malloc(sizeof(expression_t));
  arith_expression_t *aexpr;
  cmp_operation_t *cmp;
  if ((cmp = try_parse_cmp_operation()) != NULL) {
    expr->type = EXPR_CMP;
    expr->instance.cmp = cmp;
  } else if ((aexpr = try_parse_arith_expression()) != NULL) {
    expr->type = EXPR_ARITH;
    expr->instance.aexpr = aexpr;
  } else if (try_parse_token(TOKEN_PAREN_LEFT)) {
    expression_t *subexpr;
    if ((subexpr = try_parse_expression()) == NULL)
      goto fail;
    ASSERT_TOKEN(TOKEN_PAREN_RIGHT);
    expr->type = EXPR_EXPR;
    expr->instance.expr = subexpr;
  } else if (try_parse_token(TOKEN_LOG_NEG) &&
             try_parse_token(TOKEN_PAREN_LEFT)) {
    expression_t *subexpr;
    if ((subexpr = try_parse_expression()) == NULL)
      goto fail;
    ASSERT_TOKEN(TOKEN_PAREN_RIGHT);

    bool_operation_t *op = malloc(sizeof(bool_operation_t));
    op->op = BOOL_OP_NOT;
    op->lhs = subexpr;
    op->rhs = NULL;

    expr->type = EXPR_BOOL;
    expr->instance.bop = op;
  } else {
    goto fail;
  }
  return expr;
fail:
  free(expr);
  lex_set_pos(prevpos);
  return NULL;
}

expression_t *try_parse_expression_bp(uint8_t min_prec) {
  long prevpos = lex_get_pos();
  expression_t *lhs;
  expression_t *rhs;

  if ((lhs = try_parse_expr_atom()) == NULL)
    goto fail;

  while (true) {
    bool_operator_t oprtr;
    long back = lex_get_pos();
    if (!try_parse_bool_op(&oprtr)) {
      lex_set_pos(back);
      break;
    }

    uint8_t prec = bool_op_precedence(oprtr);
    if (prec <= min_prec) {
      lex_set_pos(back);
      break;
    }

    if ((rhs = try_parse_expression_bp(prec)) == NULL)
      goto fail;

    bool_operation_t *op = malloc(sizeof(bool_operation_t));
    op->op = oprtr;
    op->lhs = lhs;
    op->rhs = rhs;
    lhs = malloc(sizeof(expression_t));
    lhs->type = EXPR_BOOL;
    lhs->instance.bop = op;
  }
  return lhs;
fail:
  lex_set_pos(prevpos);
  return NULL;
}

expression_t *try_parse_expression() { return try_parse_expression_bp(0); }

while_statement_t *try_parse_while_loop() {
  long prevpos = lex_get_pos();
  expression_t *cond;
  code_block_t *code_block;

  ASSERT_TOKEN(TOKEN_KW_WHILE);
  if ((cond = try_parse_expression()) == NULL)
    goto fail;
  if ((code_block = try_parse_code_block()) == NULL)
    goto fail;
  while_statement_t *stmt = malloc(sizeof(while_statement_t));
  stmt->code_block = code_block;
  stmt->cond = cond;
  return stmt;
fail:
  lex_set_pos(prevpos);
  return NULL;
}

cond_statement_t *try_parse_cond_statement() {
  long prevpos = lex_get_pos();
  expression_t *cond;
  code_block_t *code_block;

  ASSERT_TOKEN(TOKEN_KW_IF);
  if ((cond = try_parse_expression()) == NULL)
    goto fail;
  if ((code_block = try_parse_code_block()) == NULL)
    goto fail;

  cond_statement_t *stmt = malloc(sizeof(cond_statement_t));
  stmt->code_block = code_block;
  stmt->cond = cond;
  return stmt;
fail:
  lex_set_pos(prevpos);
  return NULL;
}

declare_statement_t *try_parse_dec_statement() {
  long prevpos = lex_get_pos();
  vartype_t *type;
  expression_t *expr;

  ASSERT_TOKEN(TOKEN_KW_DEC);
  if ((type = try_parse_vartype()) == NULL)
    goto fail;
  ASSERT_TOKEN(TOKEN_OP_EQU);

  if ((expr = try_parse_expression()) == NULL)
    goto fail;

  declare_statement_t *stmt = malloc(sizeof(declare_statement_t));
  stmt->expr = expr;
  stmt->name = type->name;
  stmt->type = type->type;
  return stmt;
fail:
  lex_set_pos(prevpos);
  return NULL;
}

ret_statement_t *try_parse_ret_statement() {
  long prevpos = lex_get_pos();
  expression_t *expr;

  ASSERT_TOKEN(TOKEN_KW_RET);

  if ((expr = try_parse_expression()) == NULL)
    goto fail;

  ret_statement_t *stmt = malloc(sizeof(ret_statement_t));
  stmt->expr = expr;
  return stmt;
fail:
  lex_set_pos(prevpos);
  return NULL;
}

assign_statement_t *try_parse_assign_statement() {
  long prevpos = lex_get_pos();
  token_value_t *value;
  expression_t *expr;

  value = ASSERT_TOKEN_VALUE(TOKEN_IDENTIFIER);
  ASSERT_TOKEN(TOKEN_OP_EQU);

  if ((expr = try_parse_expression()) == NULL)
    goto fail;

  assign_statement_t *stmt = malloc(sizeof(assign_statement_t));
  stmt->expr = expr;
  stmt->lhs = value->str;
  return stmt;
fail:
  lex_set_pos(prevpos);
  return NULL;
}

statement_t *try_parse_statement() {
  statement_t *stmt = malloc(sizeof(statement_t));
  if ((stmt->instance.declare = try_parse_dec_statement()) != NULL) {
    stmt->type = STMT_DECLARE;
  } else if ((stmt->instance.ret = try_parse_ret_statement()) != NULL) {
    stmt->type = STMT_RET;
  } else if ((stmt->instance.assign = try_parse_assign_statement()) != NULL) {
    stmt->type = STMT_ASSIGN;
  } else if ((stmt->instance.cond = try_parse_cond_statement()) != NULL) {
    stmt->type = STMT_COND;
  } else if ((stmt->instance.while_loop = try_parse_while_loop()) != NULL) {
    stmt->type = STMT_WHILE;
  } else if (try_parse_token(TOKEN_KW_CONT)) {
    stmt->type = STMT_CONT;
  } else if (try_parse_token(TOKEN_KW_BREAK)) {
    stmt->type = STMT_BREAK;
  } else if ((stmt->instance.expr = try_parse_expression()) != NULL) {
    stmt->type = STMT_EXPR;
  } else {
    return NULL;
  }
  return stmt;
}

code_block_t *try_parse_code_block() {
  statement_t **stmts = calloc(25, sizeof(statement_t));
  long prevpos = lex_get_pos();
  unsigned int i = 0;

  ASSERT_TOKEN(TOKEN_BRACE_LEFT);
  while (!try_parse_token(TOKEN_BRACE_RIGHT)) {
    statement_t *stmt = try_parse_statement();
    if (stmt == NULL)
      goto fail;
    stmts[i++] = stmt;
  }

  code_block_t *code_block = malloc(sizeof(code_block_t));
  code_block->statements = stmts;
  return code_block;
fail:
  lex_set_pos(prevpos);
  return NULL;
}

function_t *try_parse_func() {
  token_value_t *name;
  code_block_t *code_block;
  vartype_t **args = calloc(MAX_FUNC_ARGS, sizeof(vartype_t *));
  unsigned int argc = 0;
  long prevpos = lex_get_pos();

  ASSERT_TOKEN(TOKEN_AT);
  name = ASSERT_TOKEN_VALUE(TOKEN_IDENTIFIER);
  ASSERT_TOKEN(TOKEN_PAREN_LEFT);

  vartype_t *vartype;
  if (!try_parse_token(TOKEN_PAREN_RIGHT)) {
    if ((vartype = try_parse_vartype()) == NULL)
      goto fail;
    args[argc++] = vartype;
    while (try_parse_token(TOKEN_COMMA)) {
      if ((vartype = try_parse_vartype()) == NULL)
        goto fail;
      args[argc++] = vartype;
    }
    ASSERT_TOKEN(TOKEN_PAREN_RIGHT);
  }

  if ((code_block = try_parse_code_block()) == NULL)
    goto fail;

  function_t *func = malloc(sizeof(function_t));
  func->name = name->str;
  func->args = args;
  func->code_block = code_block;
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
