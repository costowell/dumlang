#include "parse.h"

#include <stdio.h>
#include <stdlib.h>

static size_t index;
static token_array_t *arr;

/// Utility

void fatal_token(const char *msg, size_t culprit) {
  printf("fatal: %s\n", msg);
  printf("at token %ld: ", culprit);
  token_t *token = array_get(arr, culprit);
  if (token != NULL)
    print_token(token);
  else
    printf("NULL");
  printf("\n");
}

void fatal_token_type(token_type_t expected, size_t culprit) {
  printf("fatal: expected '%s'\n", token_type_names[expected]);
  printf("at token %ld: ", culprit);
  token_t *token = array_get(arr, culprit);
  if (token != NULL)
    print_token(token);
  else
    printf("NULL");
  printf("\n");
}

token_t *peek_next_type(token_type_t expected) {
  token_t *token = array_get(arr, index);
  if (token == NULL || token->type != expected)
    return NULL;
  return token;
}

token_t *next() { return array_get(arr, index++); }

token_t *next_type(token_type_t expected) {
  token_t *token = peek_next_type(expected);
  if (token != NULL)
    index++;
  return token;
}

#define ASSERT_NEXT(Y)                                                         \
  {                                                                            \
    if (next_type((Y)) == NULL) {                                              \
      fatal_token_type((Y), (index - 1));                                      \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  }

/// Parsing

arith_operation_t *parse_arith_operation();

type_t parse_type() {
  if (next_type(TOKEN_TYPE_INT) == NULL) {
    fatal_token("invalid type", index - 1);
    exit(EXIT_FAILURE);
  } else {
    return TYPE_INT64;
  }
}

arith_operator_t parse_arith_op() {
  token_t *token = next();
  arith_operator_t op;
  switch (token->type) {
  case TOKEN_OP_ADD:
    op = OP_ADD;
    break;
  case TOKEN_OP_SUB:
    op = OP_SUB;
    break;
  case TOKEN_OP_MUL:
    op = OP_MUL;
    break;
  case TOKEN_OP_DIV:
    op = OP_DIV;
    break;
  default:
    fatal_token("expected arithmetic operator", index - 1);
    exit(EXIT_FAILURE);
  }
  return op;
}

arith_expression_t *parse_arith_expression() {
  token_t *token = next();
  arith_expression_t *elm = malloc(sizeof(arith_expression_t));
  if (token->type == TOKEN_INT) {
    elm->type = ARITH_NUM;
    elm->instance.int64 = token->value.int64;
  } else if (token->type == TOKEN_IDENTIFIER) {
    elm->type = ARITH_IDENT;
    elm->instance.name = token->value.str;
  } else if (token->type == TOKEN_PAREN_LEFT) {
    elm->type = ARITH_OP;
    elm->instance.op = parse_arith_operation();
  } else {
    free(elm);
    fatal_token("expected arithmetic element", index - 1);
    exit(EXIT_FAILURE);
  }
  return elm;
}

arith_operation_t *parse_arith_operation() {
  // TODO: parse more than binary expr
  arith_expression_t *lhs = parse_arith_expression();
  arith_operator_t op = parse_arith_op();
  arith_expression_t *rhs = parse_arith_expression();
  arith_operation_t *expr = malloc(sizeof(arith_operation_t));
  expr->op = op;
  expr->lhs = lhs;
  expr->rhs = rhs;
  return expr;
}

expression_t *parse_expression() {
  expression_t *expr = malloc(sizeof(expression_t));
  expr->type = EXPR_ARITH;
  expr->instance.arith = parse_arith_expression();
  return expr;
}

call_statement_t *parse_call_statement() {
  token_t *func_name = next_type(TOKEN_IDENTIFIER);
  ASSERT_NEXT(TOKEN_PAREN_LEFT);
  // TODO: parse params
  ASSERT_NEXT(TOKEN_PAREN_RIGHT);
  call_statement_t *stmt = malloc(sizeof(call_statement_t));
  stmt->name = func_name->value.str;
  stmt->params = NULL;
  return stmt;
}

assign_statement_t *parse_assign_statement() {
  token_t *lhs = next_type(TOKEN_IDENTIFIER);
  ASSERT_NEXT(TOKEN_OP_EQU);
  expression_t *expr = parse_expression();

  assign_statement_t *stmt = malloc(sizeof(assign_statement_t));
  stmt->lhs = lhs->value.str;
  stmt->expr = expr;
  return stmt;
}

declare_statement_t *parse_declare_statement() {
  type_t type = parse_type();
  token_t *name = next_type(TOKEN_IDENTIFIER);

  declare_statement_t *stmt = malloc(sizeof(declare_statement_t));
  stmt->name = name->value.str;
  stmt->type = type;

  if (peek_next_type(TOKEN_OP_EQU) != NULL) {
    next();
    stmt->expr = parse_expression();
  } else {
    stmt->expr = NULL;
  }

  return stmt;
}

ret_statement_t *parse_ret_statement() {
  ASSERT_NEXT(TOKEN_RETURN);
  expression_t *expr = parse_expression();

  ret_statement_t *stmt = malloc(sizeof(ret_statement_t));
  stmt->expr = expr;
  return stmt;
}

statement_t *parse_statement() {

  statement_t *stmt = malloc(sizeof(statement_t));
  if (peek_next_type(TOKEN_TYPE_INT) != NULL) {
    stmt->type = STMT_DECLARE;
    stmt->instance.declare = parse_declare_statement();
  } else if (peek_next_type(TOKEN_RETURN) != NULL) {
    stmt->type = STMT_RET;
    stmt->instance.ret = parse_ret_statement();
  } else {
    // Peek the next token since it distinguishes
    // a function call and a var assignment
    //
    // (Snippet from BNF):
    // <call-stmt>   ::= <identifier> '(' (<expr> (',' <expr>)*)? ')'
    // <assign-stmt> ::= <identifier> '=' <expr>
    token_t *tok = array_get(arr, index + 1);

    if (tok->type == TOKEN_PAREN_LEFT) {
      stmt->type = STMT_CALL;
      stmt->instance.call = parse_call_statement();
    } else if (tok->type == TOKEN_OP_EQU) {
      stmt->type = STMT_ASSIGN;
      stmt->instance.assign = parse_assign_statement();
    } else {
      free(stmt);
      fatal_token("exepected function call or variable assignment, got neither",
                  index);
      exit(EXIT_FAILURE);
    }
  }

  return stmt;
}

code_block_t *parse_code_block() {
  ASSERT_NEXT(TOKEN_BRACE_LEFT);
  statement_t **statements = calloc(20, sizeof(statement_t *));
  for (int i = 0; i < 20; ++i) {
    if (peek_next_type(TOKEN_BRACE_RIGHT) != NULL)
      break;
    statements[i] = parse_statement();
    ASSERT_NEXT(TOKEN_ENDLINE);
  }
  index++; // Increment because of successful peek
  code_block_t *code_block = malloc(sizeof(code_block_t));
  code_block->statements = statements;
  return code_block;
}

function_t *parse_function() {
  type_t return_type;
  token_t *name;
  code_block_t *code_block;

  // Get function properties
  return_type = parse_type();
  name = next_type(TOKEN_IDENTIFIER);
  if (name == NULL) {
    fatal_token_type(TOKEN_IDENTIFIER, index - 1);
    exit(EXIT_FAILURE);
  }
  ASSERT_NEXT(TOKEN_PAREN_LEFT)
  // TODO: parse args
  ASSERT_NEXT(TOKEN_PAREN_RIGHT)
  code_block = parse_code_block();

  // Return func
  function_t *func = malloc(sizeof(function_t));
  func->return_type = return_type;
  func->name = name->value.str;
  func->args = NULL;
  func->code_block = code_block;
  return func;
}

function_t **parse_ast(token_array_t *tok_arr) {
  index = 0;
  arr = tok_arr;

  // Max 8 functions
  function_t **funcs = calloc(8, sizeof(function_t *));
  size_t i = 0;

  while (index < arr->len) {
    funcs[i++] = parse_function();
  }
  return funcs;
}
