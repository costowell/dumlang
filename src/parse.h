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

typedef enum _expression_type {
  EXPR_ARITH,
  EXPR_CMP,
  EXPR_BOOL,
  EXPR_EXPR
} expression_type_t;

typedef enum _arith_operator {
  ARITH_OP_ADD,
  ARITH_OP_SUB,
  ARITH_OP_MUL,
  ARITH_OP_DIV
} arith_operator_t;

typedef enum _bool_operator {
  BOOL_OP_AND,
  BOOL_OP_OR,
  BOOL_OP_NOT
} bool_operator_t;

typedef enum _cmp_operator {
  CMP_OP_LT,
  CMP_OP_GT,
  CMP_OP_LTE,
  CMP_OP_GTE,
  CMP_OP_EQU,
  CMP_OP_NEQ,
} cmp_operator_t;

typedef enum _arith_elm_type {
  ARITH_NUM,
  ARITH_IDENT,
  ARITH_FUNC_CALL,
  ARITH_OP,
  ARITH_EXPR,
} arith_elm_type_t;

typedef struct _arith_operation arith_operation_t;
typedef struct _bool_operation bool_operation_t;
typedef struct _expression expression_t;
typedef struct _code_block code_block_t;
typedef struct _arith_expression arith_expression_t;

typedef struct _func_call {
  expression_t **args;
  char *name;
} func_call_t;

struct _arith_expression {
  arith_elm_type_t type;
  union {
    arith_expression_t *expr;
    arith_operation_t *op;
    func_call_t *func_call;
    int64_t int64;
    char *name;
  } instance;
};

struct _arith_operation {
  arith_operator_t op;
  arith_expression_t *lhs;
  arith_expression_t *rhs;
};

typedef struct _cmp_operation {
  cmp_operator_t op;
  arith_expression_t *lhs;
  arith_expression_t *rhs;
} cmp_operation_t;

struct _bool_operation {
  bool_operator_t op;
  expression_t *lhs;
  expression_t *rhs;
};

struct _expression {
  expression_type_t type;
  union {
    expression_t *expr;
    arith_expression_t *aexpr;
    bool_operation_t *bop;
    cmp_operation_t *cmp;
  } instance;
};

typedef enum _statement_type {
  STMT_DECLARE,
  STMT_ASSIGN,
  STMT_RET,
  STMT_COND,
  STMT_WHILE,
  STMT_CONT,
  STMT_BREAK
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
