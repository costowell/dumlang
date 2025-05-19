#ifndef _JMP_H
#define _JMP_H

#include <stddef.h>

#include "instr.h"

typedef enum _jmp_target_t {
  LABEL_BLOCK_START,
  LABEL_BLOCK_END,
  LABEL_LOOP_START,
  LABEL_RET,
  LABEL_NEXT_COND,
} jmp_target_t;

typedef struct _jmp {
  struct _jmp *next;
  struct _jmp *prev;
  jmp_target_t target;
  opcode_t op;
  size_t loc;
} jmp_t;

typedef struct _jmptab {
  jmp_t *first;
  jmp_t *last;
} jmptab_t;

jmptab_t *jmptab_init();
void jmptab_insert(jmptab_t *tab, size_t loc, jmp_target_t target, opcode_t op);
void jmptab_eval(jmptab_t *tab, jmp_target_t target, size_t value);
void jmptab_free(jmptab_t *tab);
void jmptab_merge(jmptab_t *dest, jmptab_t *src);
void jmptab_print(jmptab_t *tab);

#endif
