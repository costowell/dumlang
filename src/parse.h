#ifndef __PARSE_H
#define __PARSE_H

#include <stdint.h>

typedef enum _type {
  INT32,
} type_t;

typedef struct _arg {
  type_t type;
  char *name;
} arg_t;

typedef enum _expression_type { EXPR_ARITH } expression_type_t;

typedef enum _arith_op { OP_ADD, OP_SUB, OP_MUL, OP_DIV } arith_op_t;

typedef enum _arith_elm_type {
  ARITH_NUM,
  ARITH_IDENT,
  ARITH_EXPR
} arith_elm_type_t;

typedef struct _arith_expression arith_expression_t;

typedef struct _arith_elm {
  arith_elm_type_t type;
  union {
    arith_expression_t *expr;
    int32_t int32;
    char *name;
  } instance;
} arith_elm_t;

struct _arith_expression {
  arith_op_t op;
  arith_elm_t lhs;
  arith_elm_t rhs;
};

typedef struct _expression {
  expression_type_t type;
  union {
    arith_expression_t arith;
  } instance;
} expression_t;

typedef enum _statement_type { STMT_ASSIGN, STMT_CALL } statement_type_t;

typedef struct _assign_statement {
  char *lhs;
  expression_t *expr;
} assign_statement_t;

typedef struct _call_statement {
  char *name;
  expression_t *params;
} call_statement_t;

typedef struct _statement {
  statement_type_t type;
  union {
    assign_statement_t assign;
    call_statement_t call;
  } instance;
} statement_t;

typedef struct _code_block {
  statement_t *statements;
} code_block_t;

typedef struct _function {
  type_t return_type;
  arg_t *args;
  char *name;
  code_block_t code_block;
} function_t;

#endif // __PARSE_H
