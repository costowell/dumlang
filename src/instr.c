#include "instr.h"

#include <stdbool.h>
#include <stddef.h>

#define MAX_INSTR_SIZE 15
#define REX_BASE 0b01000000

#define BYTE(NUM, INDEX) ((uint8_t)(((NUM) >> ((INDEX) * 8)) & 0xFF))
#define INT_TO_BYTEARR(BYTEARR, NUM, SIZE)                                     \
  do {                                                                         \
    for (int BYTEARR##_count = 0; BYTEARR##_count < SIZE; BYTEARR##_count++)   \
      BYTEARR[BYTEARR##_count] = BYTE(NUM, BYTEARR##_count);                   \
    BYTEARR##_size = SIZE;                                                     \
  } while (0)

uint8_t rex_prefix = 0;

opcode_type_t opc_type;
uint8_t opc_byte;

bool modregrm = false;
uint8_t mod = 0;
uint8_t reg = 0;
uint8_t rm = 0;

uint8_t imm[8];
uint8_t imm_size = 0;

uint8_t disp[8];
uint8_t disp_size = 0;

uint8_t instruction[MAX_INSTR_SIZE + 1] = {0};
uint8_t instruction_size = 0;

void append_uint8(uint8_t byte) {
  if (instruction_size >= MAX_INSTR_SIZE)
    return;
  instruction[instruction_size++] = byte;
}

void append_buf(uint8_t *buf, uint8_t size) {
  for (int i = 0; i < size; ++i)
    append_uint8(buf[i]);
}

uint8_t instr_flush(uint8_t **buf) {
  instruction_size = 0;

  // REX Prefix
  if (rex_prefix != 0)
    append_uint8(REX_BASE | rex_prefix);

  // Opcode
  if (opc_byte != 0) {
    switch (opc_type) {
    case SINGLE_BYTE:
      break;
    case DOUBLE_BYTE:
      append_uint8(0x0F);
      break;
    case TRIPLE_BYTE_A:
      append_uint8(0x0F);
      append_uint8(0x38);
      break;
    case TRIPLE_BYTE_B:
      append_uint8(0x0F);
      append_uint8(0x3A);
      break;
    }
    append_uint8(opc_byte);
  } else {
    *buf = NULL;
    return 0;
  }

  // Mod-Reg-R/M
  if (modregrm) {
    append_uint8((uint8_t)((mod << 6) | (reg << 3) | rm));
  }

  if (imm_size != 0)
    append_buf(imm, imm_size);
  if (disp_size != 0)
    append_buf(disp, disp_size);

  modregrm = false;
  mod = 0;
  reg = 0;
  rm = 0;
  rex_prefix = 0;
  opc_type = SINGLE_BYTE;
  opc_byte = 0x00;
  imm_size = 0;
  disp_size = 0;
  *buf = instruction;
  return instruction_size;
}

void instr_set_opcode(opcode_t opc) {
  opc_byte = opcode_enc_map[opc];
  opc_type = opcode_type_map[opc];
}

void instr_set_opcode_inc(opcode_t opc, uint8_t off) {
  instr_set_opcode(opc);
  opc_byte += off;
}

void instr_set_rex(rex_flags_t rex) { rex_prefix = (uint8_t)rex; }

void instr_set_mod(uint8_t m) {
  modregrm = true;
  mod = m;
}
void instr_set_reg(reg_t r) {
  modregrm = true;
  reg = (uint8_t)r & 0x07;
}
void instr_set_rm(uint8_t r) {
  modregrm = true;
  rm = r & 0x07;
}
void instr_set_disp8(uint8_t i) { INT_TO_BYTEARR(disp, i, 1); }
void instr_set_disp16(uint16_t i) { INT_TO_BYTEARR(disp, i, 2); }
void instr_set_disp32(uint32_t i) { INT_TO_BYTEARR(disp, i, 4); }
void instr_set_disp64(uint64_t i) { INT_TO_BYTEARR(disp, i, 8); }

void instr_set_imm8(uint8_t i) { INT_TO_BYTEARR(imm, i, 1); }
void instr_set_imm16(uint16_t i) { INT_TO_BYTEARR(imm, i, 2); }
void instr_set_imm32(uint32_t i) { INT_TO_BYTEARR(imm, i, 4); }
void instr_set_imm64(uint64_t i) { INT_TO_BYTEARR(imm, i, 8); }
