#ifndef _OBJ_H
#define _OBJ_H

#include <libelf.h>
#include <stddef.h>
#include <stdint.h>

void write_obj(const char *file, Elf64_Sym *symtab, uint8_t *text, char *strtab,
               size_t symtab_len, size_t text_len, size_t strtab_len);

#endif
