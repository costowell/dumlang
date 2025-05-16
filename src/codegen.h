#ifndef _CODEGEN_H
#define _CODEGEN_H

#include "hashmap.h"
#include "parse.h"
#include "instr.h"

typedef enum _jmp_target_t {
  LABEL_BLOCK_START,
  LABEL_BLOCK_END,
  LABEL_NEXT_COND,
} jmp_target_t;

typedef struct _jmp {
  jmp_target_t target;
  opcode_t op;
  size_t loc;
} jmp_t;

void gen_object(function_t **funcs, const char *file);

#endif // _CODEGEN_H
