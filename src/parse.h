#ifndef __PARSE_H
#define __PARSE_H

#include "lex.h"

#include <stdint.h>

#define MAX_FUNC_ARGS 6

typedef enum _type {
  TYPE_NONE,
  TYPE_INT64,
} type_t;

typedef struct _vartype {
  type_t type;
  char *name;
} vartype_t;

typedef enum _expression_type { EXPR_ARITH } expression_type_t;

// P
// E
// M D
// A S
typedef enum _arith_operator {
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV
} arith_operator_t;

typedef enum _arith_elm_type {
  ARITH_NUM,
  ARITH_IDENT,
  ARITH_FUNC_CALL,
  ARITH_OP
} arith_elm_type_t;

typedef struct _arith_operation arith_operation_t;
typedef struct _expression expression_t;
typedef struct _code_block code_block_t;

typedef struct _func_call {
  expression_t **args;
  char *name;
} func_call_t;

typedef struct _arith_expression {
  arith_elm_type_t type;
  union {
    arith_operation_t *op;
    func_call_t *func_call;
    int64_t int64;
    char *name;
  } instance;
} arith_expression_t;

struct _arith_operation {
  arith_operator_t op;
  arith_expression_t *lhs;
  arith_expression_t *rhs;
};

struct _expression {
  expression_type_t type;
  union {
    arith_expression_t *arith;
  } instance;
};

typedef enum _statement_type {
  STMT_DECLARE,
  STMT_ASSIGN,
  STMT_RET,
  STMT_COND,
  STMT_WHILE,
} statement_type_t;

typedef struct _while_statement {
  expression_t *cond;
  code_block_t *code_block;
} while_statement_t;

typedef struct _if_statement {
  expression_t *cond;
  code_block_t *code_block;
} cond_statement_t;

typedef struct _declare_statement {
  type_t type;
  char *name;
  expression_t *expr; // CAN BE NULL
} declare_statement_t;

typedef struct _assign_statement {
  char *lhs;
  expression_t *expr;
} assign_statement_t;

typedef struct _ret_statement {
  expression_t *expr;
} ret_statement_t;

typedef struct _statement {
  statement_type_t type;
  union {
    assign_statement_t *assign;
    declare_statement_t *declare;
    ret_statement_t *ret;
    expression_t *expr;
    cond_statement_t *cond;
    while_statement_t *while_loop;
  } instance;
} statement_t;

struct _code_block {
  statement_t **statements;
};

typedef struct _function {
  type_t return_type;
  vartype_t **args;
  char *name;
  code_block_t *code_block;
} function_t;

function_t **try_parse_ast();

#endif // __PARSE_H
