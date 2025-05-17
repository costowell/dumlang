#ifndef _CODEGEN_H
#define _CODEGEN_H

#include "hashmap.h"
#include "parse.h"
#include "instr.h"

void gen_object(function_t **funcs, const char *file);
void write_jmp(opcode_t opc, int32_t dest);
void text_set_pos(size_t pos);
size_t text_get_pos();

#endif // _CODEGEN_H
