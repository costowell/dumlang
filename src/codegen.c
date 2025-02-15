#include "codegen.h"
#include "hashmap.h"
#include "instr.h"
#include "obj.h"

#include <err.h>
#include <libelf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Elf64_Sym symtab[64];
size_t symtab_len;

uint8_t text[1024];
size_t text_len;

char strtab[256];
unsigned int strtab_len;

/* Utils */

scope_t *init_scope() {
  scope_t *scope = calloc(1, sizeof(scope_t));
  if (hashmap_create(8, &scope->nametomem) != 0)
    return NULL;
  return scope;
}

void push_scope(scope_t *super_scope, scope_t *scope) {
  if (super_scope == NULL)
    return;
  while (super_scope->next != NULL) {
    super_scope = super_scope->next;
  }
  super_scope->next = scope;
}

void pop_scope(scope_t *super_scope) {
  if (super_scope == NULL || super_scope->next == NULL)
    return;
  while (super_scope->next->next != NULL) {
    super_scope = super_scope->next;
  }
  super_scope->next = NULL;
}

void append_strtab(char *str) {
  strcpy(strtab + strtab_len, str);
  strtab_len += strlen(str) + 1;
  strtab[strtab_len] = '\0';
}

void print_strtab() {
  for (size_t i = 0; i < strtab_len; ++i) {
    if (strtab[i] == 0)
      printf("(null)");
    else
      printf("%c", strtab[i]);
  }
  printf("\n");
}

void emit() {
  uint8_t *instr;
  uint8_t size = instr_flush(&instr);

  if (instr == NULL)
    errx(EXIT_FAILURE, "failed to write instruction");

  memcpy(text + text_len, instr, size);
  text_len += size;
}

#define REXB(REG, FLAGS)                                                       \
  if ((REG) >= R8)                                                             \
    instr_set_rex(REX_B | (FLAGS));                                            \
  else                                                                         \
    instr_set_rex(FLAGS);

#define REXR(REG, FLAGS)                                                       \
  if ((REG) >= R8)                                                             \
    instr_set_rex(REX_R | (FLAGS));                                            \
  else                                                                         \
    instr_set_rex(FLAGS);

#define REXBR(REG, RM, FLAGS)                                                  \
  do {                                                                         \
    rex_flags_t flags = (FLAGS);                                               \
    if ((REG) >= R8)                                                           \
      flags |= REX_R;                                                          \
    if ((RM) >= R8)                                                            \
      flags |= REX_B;                                                          \
    instr_set_rex(flags);                                                      \
  } while (0)


uint8_t reg_num(reg_t r) { return r & 0x07; }

void pop(reg_t reg) {
  REXB(reg, 0);
  instr_set_opcode_inc(POP_R, reg_num(reg));
  emit();
}

void push(reg_t reg) {
  REXB(reg, 0);
  instr_set_opcode_inc(PUSH_R, reg_num(reg));
  emit();
}

void mov_regs(reg_t dst, reg_t src) {
  REXBR(src, dst, REX_W);
  instr_set_opcode(MOV_R_RM);
  instr_set_mod(0b11);
  instr_set_reg(src);
  instr_set_rm(dst);
  emit();
}

void mov_imm64(reg_t reg, int64_t imm) {
  REXB(reg, REX_W);
  instr_set_opcode_inc(MOV_R_IMM, reg_num(reg));
  instr_set_imm64(imm);
  emit();
}

void sub_imm32(reg_t reg, int32_t imm) {
  REXB(reg, REX_W);
  instr_set_opcode(SUB_RM_IMM);
  instr_set_mod(0b11);
  instr_set_rm(reg);
  instr_set_reg(0b101);
  instr_set_imm32(imm);
  emit();
}

void ret() {
  instr_set_opcode(RET_NEAR);
  emit();
}

/* Write Machine Instructions */

void write_declare_statement(declare_statement_t *stmt, scope_t *scope) {
  // We can discard type for now since we know it has to be an INT
}

void write_ret_statement(ret_statement_t *stmt, scope_t *scope) {
  switch (stmt->expr->instance.arith->type) {
    case ARITH_NUM:
      mov_imm64(RAX, stmt->expr->instance.arith->instance.int64);
      ret();
      break;
  }
}

void write_statement(statement_t *stmt, scope_t *scope) {
  switch (stmt->type) {
  case STMT_DECLARE:
    write_declare_statement(stmt->instance.declare, scope);
    break;
  case STMT_RET:
    write_ret_statement(stmt->instance.ret, scope);
    break;
  }

}

void write_codeblock(code_block_t *block, scope_t *super_scope) {
  scope_t *scope = init_scope();
  push_scope(super_scope, scope);
  for (statement_t **stmts = block->statements; *stmts != NULL; ++stmts) {
    statement_t *stmt = *stmts;
    write_statement(stmt, super_scope);
  }
}

uint32_t calc_stack_size(code_block_t *code_block) {
  statement_t **s = code_block->statements;
  uint32_t size = 0;
  for (; *s != NULL; s++) {
    statement_t *stmt = *s;
    if (stmt->type == STMT_DECLARE) {
      if (stmt->instance.declare->type == TYPE_INT64) {
        size += 8;
      }
    }
  }
  return size;
}

void write_func(function_t *func) {
  // Create almost complete symbol (need section size)
  Elf64_Sym sym = {
      .st_name = strtab_len,
      .st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC),
      .st_other = STV_DEFAULT,
      .st_value = text_len,
  };

  uint32_t stack_size = calc_stack_size(func->code_block);

  // Setup stack
  if (stack_size > 0) {
    push(RBP);
    mov_regs(RBP, RSP);
    sub_imm32(RSP, stack_size);
  }

  // Init scope
  scope_t *scope = init_scope();

  // Write code block
  write_codeblock(func->code_block, scope);

  // Tear down stack
  if (stack_size > 0) {
    mov_regs(RSP, RBP);
    pop(RBP);
  }

  // Add to strtab
  append_strtab(func->name);
  sym.st_size = text_len;

  symtab[symtab_len++] = sym;
}

void gen_object(function_t **funcs, const char *file) {
  memset(strtab, 0, sizeof(strtab));
  memset(symtab, 0, sizeof(symtab));
  memset(text, 0, sizeof(text));
  strtab_len = 1;
  symtab_len = 2;
  text_len = 0;

  Elf64_Sym text_sym = {
      .st_name = 1,
      .st_info = ELF64_ST_INFO(STB_LOCAL, STT_SECTION),
      .st_other = STV_DEFAULT,
      .st_value = 0,
      .st_size = 0,
  };
  symtab[1] = text_sym;
  append_strtab(".text");

  for (; *funcs != NULL; funcs++) {
    write_func(*funcs);
  }

  write_obj(file, symtab, text, strtab, symtab_len, text_len, strtab_len);
}
