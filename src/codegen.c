#include "codegen.h"
#include "hashmap.h"
#include "instr.h"
#include "obj.h"
#include "parse.h"
#include "scope.h"

#include <err.h>
#include <libelf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXT_SIZE 1024
#define STRTAB_SIZE 256
#define SYMTAB_SIZE 64

Elf64_Sym symtab[SYMTAB_SIZE];
size_t symtab_len;

uint8_t text[TEXT_SIZE];
size_t text_len;

char strtab[STRTAB_SIZE];
size_t strtab_len;

// clang-format off
bool regtab[NUM_REGISTERS] = {
    [RSP] = true, [RBP] = true, // Obviously
    [RAX] = true, [RDX] = true, // For division :( TODO: find a better way to do this
    0
};
// clang-format on

reg_t param_regs[6] = {RDI, RSI, RDX, RCX, R8, R9};

/* Utils */

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

reg_t next_reg() {
  for (int i = 0; i < NUM_REGISTERS; ++i) {
    // this register is not in use
    if (!regtab[i]) {
      // now in use
      regtab[i] = true;
      return (reg_t)i;
    }
  }

  errx(EXIT_FAILURE, "not enough registers available!");
}

void emit() {
  uint8_t *instr;
  uint8_t size = instr_flush(&instr);

  if (text_len + size >= TEXT_SIZE)
    errx(EXIT_FAILURE, "text section bigger than %d bytes", TEXT_SIZE);

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

void mov_reg_to_reg(reg_t dst, reg_t src) {
  REXBR(src, dst, REX_W);
  instr_set_opcode(MOV_R_RM);
  instr_set_mod(MOD_REG);
  instr_set_reg(src);
  instr_set_rm(dst);
  emit();
}

void mov_imm64_to_reg(reg_t reg, int64_t imm) {
  REXB(reg, REX_W);
  instr_set_opcode_inc(MOV_R_IMM, reg_num(reg));
  instr_set_imm64((uint64_t)imm);
  emit();
}

void mov_mem_offset_to_reg(reg_t dst, reg_t src_base, int32_t displacement) {
  REXBR(src_base, dst, REX_W);
  instr_set_opcode(MOV_RM_R);
  instr_set_mod(MOD_DISP_4); // Four byte signed displacement
  instr_set_rm(src_base);
  instr_set_reg(dst);
  instr_set_disp32((uint32_t)displacement);
  emit();
}

void mov_reg_to_mem_offset(reg_t src, reg_t dst_base, int32_t displacement) {
  REXBR(dst_base, src, REX_W);
  instr_set_opcode(MOV_R_RM);
  instr_set_mod(MOD_DISP_4); // Four byte signed displacement
  instr_set_rm(dst_base);
  instr_set_reg(src);
  instr_set_disp32((uint32_t)displacement);
  emit();
}

void sub_imm32(reg_t reg, int32_t imm) {
  REXB(reg, REX_W);
  instr_set_opcode(SUB_RM_IMM);
  instr_set_mod(MOD_REG);
  instr_set_rm(reg);
  instr_set_reg(0b101);
  instr_set_imm32((uint32_t)imm);
  emit();
}

void sub_reg_to_reg(reg_t dst, reg_t src) {
  REXBR(dst, src, REX_W);
  instr_set_opcode(SUB_R_RM);
  instr_set_mod(MOD_REG);
  instr_set_rm(src);
  instr_set_reg(dst);
  emit();
}

void add_reg_to_reg(reg_t dst, reg_t src) {
  REXBR(dst, src, REX_W);
  instr_set_opcode(ADD_R_RM);
  instr_set_mod(MOD_REG);
  instr_set_rm(src);
  instr_set_reg(dst);
  emit();
}

void imul_reg_to_reg(reg_t dst, reg_t src) {
  REXBR(dst, src, REX_W);
  instr_set_opcode(IMUL_R_RM);
  instr_set_mod(MOD_REG);
  instr_set_rm(src);
  instr_set_reg(dst);
  emit();
}

void div_reg_to_reg(reg_t quotient) {
  REXB(quotient, REX_W);
  instr_set_opcode(DIV_RM);
  instr_set_mod(MOD_REG);
  instr_set_rm(quotient);
  instr_set_reg(6);
  emit();
}

void ret() {
  instr_set_opcode(RET_NEAR);
  emit();
}

void call_rel32(int32_t disp) {
  instr_set_opcode(CALL_REL32);
  instr_set_disp32((uint32_t)disp);
  emit();
}

void je_rel32(int32_t disp) {
  instr_set_opcode(JE_REL32);
  instr_set_disp32((uint32_t)disp);
  emit();
}

void cmp_reg_imm8(reg_t reg, uint8_t imm) {
  REXB(reg, REX_W);
  instr_set_opcode(CMP_RM_IMM8);
  instr_set_mod(MOD_REG);
  instr_set_reg(7);
  instr_set_rm(reg);
  instr_set_imm8(imm);
  emit();
}

/* Write Machine Instructions */

void evaluate_expression(expression_t *expr, reg_t result, scope_t *scope);
void write_codeblock(code_block_t *block, scope_t *scope);

reg_t _evaluate_arith_expression(arith_expression_t *expr, scope_t *scope) {
  // Arith expr parsing might not populate all nodes in the tree
  // because I'm bad at programming
  // Lets check before we do anything and leave a nice error message
  if (expr == NULL) {
    errx(EXIT_FAILURE, "NULL node found in arith tree");
  }
  if (expr->type == ARITH_NUM) {
    reg_t r = next_reg();
    mov_imm64_to_reg(r, expr->instance.int64);
    return r;
  } else if (expr->type == ARITH_IDENT) {
    const scope_var_t *scope_var = scope_get(scope, expr->instance.name);
    if (scope_var == NULL)
      errx(EXIT_FAILURE, "error: '%s' not found in scope", expr->instance.name);
    reg_t r = next_reg();
    mov_mem_offset_to_reg(r, RBP, scope_var->position);
    return r;
  } else if (expr->type == ARITH_OP) {
    const arith_operation_t *op = expr->instance.op;
    reg_t lhsr = _evaluate_arith_expression(op->lhs, scope);
    reg_t rhsr = _evaluate_arith_expression(op->rhs, scope);

    regtab[rhsr] = false;

    switch (op->op) {
    case OP_ADD:
      add_reg_to_reg(lhsr, rhsr);
      break;
    case OP_MUL:
      imul_reg_to_reg(lhsr, rhsr);
      break;
    case OP_SUB:
      sub_reg_to_reg(lhsr, rhsr);
      break;
    case OP_DIV:
      mov_imm64_to_reg(RDX, 0);
      mov_reg_to_reg(RAX, lhsr);
      div_reg_to_reg(rhsr);
      mov_reg_to_reg(lhsr, RAX);
      break;
    }
    return lhsr;
  } else if (expr->type == ARITH_FUNC_CALL) {
    func_call_t *func = expr->instance.func_call;
    // TODO: verify params match func arg type and count
    for (int i = 0; i < MAX_FUNC_ARGS; ++i) {
      if (func->args[i] == NULL) {
        break;
      }
      evaluate_expression(func->args[i], param_regs[i], scope);
    }
    // TODO: maybe hashmap it up? gotta make sure there is order though
    for (size_t i = 0; i < symtab_len; ++i) {
      Elf64_Sym sym = symtab[i];
      char *fn_name = strtab + sym.st_name;
      if (strcmp(func->name, fn_name) == 0) {
        // Subtract the function's position by our current position,
        // then subtract the size of call() instr (5) since its relative to the
        // next instr
        int32_t disp = (int32_t)(sym.st_value - text_len) - 5;
        call_rel32(disp);
        return RAX;
      }
    }
    errx(EXIT_FAILURE, "no function named '%s'", func->name);
  } else {
    errx(EXIT_FAILURE, "unknown expression type");
  }
}

void evaluate_arith_expression(arith_expression_t *expr, reg_t result,
                               scope_t *scope) {
  reg_t r = _evaluate_arith_expression(expr, scope);
  mov_reg_to_reg(result, r);
}

void evaluate_expression(expression_t *expr, reg_t result, scope_t *scope) {
  // We can assume all expressions are arith expressions
  evaluate_arith_expression(expr->instance.arith, result, scope);
}

void write_declare_statement(declare_statement_t *stmt, scope_t *scope,
                             char **added_vars, uint8_t *added_vars_size) {
  // We can discard type for now since we know it has to be an INT
  if (scope_get(scope, stmt->name) != NULL)
    errx(EXIT_FAILURE, "error: '%s' already declared", stmt->name);

  // Size is by default 8 since INT is the only type
  const scope_var_t *scope_var = scope_insert(scope, stmt->name, 8);
  added_vars[(*added_vars_size)++] = stmt->name;

  evaluate_expression(stmt->expr, RAX, scope);
  mov_reg_to_mem_offset(RAX, RBP, scope_var->position);
}

void write_ret_statement(ret_statement_t *stmt, scope_t *scope) {
  evaluate_expression(stmt->expr, RAX, scope);
  mov_reg_to_reg(RSP, RBP);
  pop(RBP);
  ret();
}

void write_assign_statement(assign_statement_t *stmt, scope_t *scope) {
  const scope_var_t *scope_var;
  if ((scope_var = scope_get(scope, stmt->lhs)) == NULL)
    errx(EXIT_FAILURE, "error: no variable '%s'", stmt->lhs);

  evaluate_expression(stmt->expr, RAX, scope);
  mov_reg_to_mem_offset(RAX, RBP, scope_var->position);
}

void write_cond_statement(cond_statement_t *stmt, scope_t *scope) {
  evaluate_expression(stmt->cond, RAX, scope);
  cmp_reg_imm8(RAX, 0);
  size_t jepos = text_len;
  je_rel32(0); // Insert placeholder instr
  size_t afterjepos = text_len;
  write_codeblock(stmt->code_block, scope);
  size_t endpos = text_len;
  text_len = jepos;
  je_rel32((int32_t)(endpos - afterjepos));
  text_len = endpos;
}

void write_statement(statement_t *stmt, scope_t *scope, char **added_vars,
                     uint8_t *added_vars_size) {
  switch (stmt->type) {
  case STMT_DECLARE:
    write_declare_statement(stmt->instance.declare, scope, added_vars,
                            added_vars_size);
    break;
  case STMT_RET:
    write_ret_statement(stmt->instance.ret, scope);
    break;
  case STMT_ASSIGN:
    write_assign_statement(stmt->instance.assign, scope);
    break;
  case STMT_COND:
    write_cond_statement(stmt->instance.cond, scope);
    break;
  }
}

void write_codeblock(code_block_t *block, scope_t *scope) {
  char *added_vars[8] = {0};
  uint8_t added_vars_size = 0;
  for (statement_t **stmts = block->statements; *stmts != NULL; ++stmts) {
    statement_t *stmt = *stmts;
    write_statement(stmt, scope, added_vars, &added_vars_size);
  }
  for (uint8_t i = 0; i < added_vars_size; ++i) {
    scope_remove(scope, added_vars[i]);
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
      .st_name = (Elf64_Word)(strtab_len),
      .st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC),
      .st_other = STV_DEFAULT,
      .st_value = text_len,
  };

  // Init scope
  scope_t *scope = scope_init();
  uint32_t stack_size = calc_stack_size(func->code_block);

  // Setup base pointer
  push(RBP);
  mov_reg_to_reg(RBP, RSP);

  // Init parameters
  for (int i = 0; i < MAX_FUNC_ARGS; ++i) {
    if (func->args[i] == NULL)
      break;
    char *arg_name = func->args[i]->name;
    // Size is by default 8 since INT is the only type
    const scope_var_t *var = scope_insert(scope, arg_name, 8);
    if (var == NULL) {
      errx(EXIT_FAILURE, "error: argument already named '%s'", arg_name);
    }
    mov_reg_to_mem_offset(param_regs[i], RBP, var->position);
    stack_size += var->size;
  }

  // Setup stack pointer
  sub_imm32(RSP, (int32_t)stack_size);

  // Write code block
  write_codeblock(func->code_block, scope);

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
