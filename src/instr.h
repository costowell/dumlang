#ifndef _INSTR_H
#define _INSTR_H

#include <stdint.h>

#define MAX_INSTR_SIZE 15

// Resource: https://wiki.osdev.org/System_V_ABI
typedef enum _reg : uint8_t {
  RAX,
  RCX,
  RDX,
  RBX,
  RSP,
  RBP,
  RSI,
  RDI,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
  NUM_REGISTERS,
} reg_t;

typedef enum _rex_flags : uint8_t {
  REX_B = 1 << 0, // extension to the MODRM.rm field or the SIB.base field
  REX_X = 1 << 1, // extension to the SIB.index field
  REX_R = 1 << 2, // extension to the MODRM.reg field
  REX_W = 1 << 3, // 1 indicates 64bit operand size,
                  // 0 indicates default operand size
} rex_flags_t;

typedef enum _opcode_type : uint8_t {
  SINGLE_BYTE = 1, // <op>
  DOUBLE_BYTE,     // 0x0F <op>
  TRIPLE_BYTE_A,   // 0x0F 0x38 <op>
  TRIPLE_BYTE_B,   // 0x0F 0x3A <op>
} opcode_type_t;

typedef enum _opcode : uint32_t {
  MOV_R_IMM = 1,
  MOV_R_RM,
  MOV_RM_R,
  SUB_EAX_IMM,
  SUB_RM_IMM,
  ADD_R_RM,
  SUB_R_RM,
  IMUL_R_RM,
  DIV_RM,
  PUSH_R,
  POP_R,
  RET_NEAR,
  CALL_REL32
} opcode_t;

typedef enum _mod : uint8_t {
  MOD_INDIRECT = 0b00,
  MOD_DISP_1 = 0b01,
  MOD_DISP_4 = 0b10,
  MOD_REG = 0b11
} mod_t;

static const uint8_t opcode_enc_map[] = {
    [MOV_R_IMM] = 0xB8,   [MOV_R_RM] = 0x89,   [MOV_RM_R] = 0x8B,
    [SUB_EAX_IMM] = 0x2D, [SUB_RM_IMM] = 0x81, [ADD_R_RM] = 0x03,
    [SUB_R_RM] = 0x2B,    [IMUL_R_RM] = 0xAF,  [PUSH_R] = 0x50,
    [POP_R] = 0x58,       [RET_NEAR] = 0xC3,   [DIV_RM] = 0xF7,
    [CALL_REL32] = 0xE8};

static const opcode_type_t opcode_type_map[] = {
    [MOV_R_IMM] = SINGLE_BYTE,  [MOV_R_RM] = SINGLE_BYTE,
    [MOV_RM_R] = SINGLE_BYTE,   [SUB_EAX_IMM] = SINGLE_BYTE,
    [SUB_RM_IMM] = SINGLE_BYTE, [ADD_R_RM] = SINGLE_BYTE,
    [IMUL_R_RM] = DOUBLE_BYTE,  [PUSH_R] = SINGLE_BYTE,
    [POP_R] = SINGLE_BYTE,      [RET_NEAR] = SINGLE_BYTE,
    [SUB_R_RM] = SINGLE_BYTE,   [DIV_RM] = SINGLE_BYTE,
    [CALL_REL32] = SINGLE_BYTE};

uint8_t instr_flush(uint8_t **buf);
void instr_set_opcode(opcode_t opc);
void instr_set_opcode_inc(opcode_t opc, uint8_t off);
void instr_set_rex(rex_flags_t rex);
void instr_set_mod(mod_t mod);
void instr_set_reg(reg_t reg);
void instr_set_rm(uint8_t rm);
void instr_set_imm8(uint8_t i);
void instr_set_imm16(uint16_t i);
void instr_set_imm32(uint32_t i);
void instr_set_imm64(uint64_t i);
void instr_set_disp8(uint8_t i);
void instr_set_disp16(uint16_t i);
void instr_set_disp32(uint32_t i);
void instr_set_disp64(uint64_t i);

#endif
