#include "obj.h"
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

// clang-format off
char shstrtab[] = {
    '\0',                                                 // NULL (offset 0)
    '.',  't',  'e', 'x', 't', '\0',                      // .text (offset 1)
    '.',  's',  'h', 's', 't', 'r', 't', 'a', 'b', '\0',  // .shstrtab (offset 6)
    '.',  's',  'y', 'm', 't', 'a', 'b', '\0',            // .symtab (offset 16)
    '.',  's',  't', 'r', 't', 'a', 'b', '\0',            // .strtab (offset 24)
};
// clang-format on

void write_obj(const char *file, Elf64_Sym *symtab, uint8_t *text, char *strtab,
               size_t symtab_len, size_t text_len, size_t strtab_len) {
  int fd;
  Elf *e;
  Elf64_Ehdr *ehdr;
  Elf64_Shdr *shdr;
  Elf_Scn *scn;
  Elf_Data *data;
  size_t textscn_index;
  size_t strtabscn_index;

  if ((fd = open(file, O_WRONLY | O_CREAT, 0755)) < 0)
    errx(EXIT_FAILURE, "failed to open '%s'", file);
  if (elf_version(EV_CURRENT) == EV_NONE)
    errx(EXIT_FAILURE, "elf lib init failed: %s", elf_errmsg(-1));
  if ((e = elf_begin(fd, ELF_C_WRITE, NULL)) == NULL)
    errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1));

  if ((ehdr = elf64_newehdr(e)) == NULL)
    errx(EXIT_FAILURE, "elf_newehdr() failed: %s", elf_errmsg(-1));

  ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
  ehdr->e_machine = EM_X86_64;
  ehdr->e_type = ET_REL;

  // Create .text
  if ((scn = elf_newscn(e)) == NULL)
    errx(EXIT_FAILURE, "elf_newscn() failed: %s", elf_errmsg(-1));

  if ((data = elf_newdata(scn)) == NULL)
    errx(EXIT_FAILURE, "elf_newdata() failed: %s", elf_errmsg(-1));

  data->d_align = 8;
  data->d_off = 0LL;
  data->d_type = ELF_T_BYTE;
  data->d_buf = text;
  data->d_size = text_len;
  data->d_version = EV_CURRENT;

  if ((shdr = elf64_getshdr(scn)) == NULL)
    errx(EXIT_FAILURE, "elf_getshdr() failed: %s", elf_errmsg(-1));

  textscn_index = elf_ndxscn(scn);

  shdr->sh_name = 1;
  shdr->sh_type = SHT_PROGBITS;
  shdr->sh_flags = SHF_ALLOC | SHF_EXECINSTR;
  shdr->sh_entsize = 0;

  // Create .strtab
  if ((scn = elf_newscn(e)) == NULL)
    errx(EXIT_FAILURE, "elf_newscn() failed: %s", elf_errmsg(-1));

  if ((data = elf_newdata(scn)) == NULL)
    errx(EXIT_FAILURE, "elf_newdata() failed: %s", elf_errmsg(-1));

  strtabscn_index = elf_ndxscn(scn);

  data->d_align = 1;
  data->d_buf = strtab;
  data->d_off = 0LL;
  data->d_size = strtab_len;
  data->d_type = ELF_T_BYTE;
  data->d_version = EV_CURRENT;

  if ((shdr = elf64_getshdr(scn)) == NULL)
    errx(EXIT_FAILURE, "elf64_getshdr() failed: %s", elf_errmsg(-1));

  shdr->sh_name = 25;
  shdr->sh_type = SHT_STRTAB;
  shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
  shdr->sh_entsize = 0;

  // Create .symtab
  if ((scn = elf_newscn(e)) == NULL)
    errx(EXIT_FAILURE, "elf_newscn() failed: %s", elf_errmsg(-1));

  if ((data = elf_newdata(scn)) == NULL)
    errx(EXIT_FAILURE, "elf_newdata() failed: %s", elf_errmsg(-1));

  for (size_t i = 1; i < symtab_len; ++i)
    symtab[i].st_shndx = (unsigned short)textscn_index;

  data->d_align = 8;
  data->d_buf = (void *)symtab;
  data->d_off = 0LL;
  data->d_size = symtab_len * sizeof(Elf64_Sym);
  data->d_type = ELF_T_SYM;
  data->d_version = EV_CURRENT;

  if ((shdr = elf64_getshdr(scn)) == NULL)
    errx(EXIT_FAILURE, "elf64_getshdr() failed: %s", elf_errmsg(-1));

  shdr->sh_name = 17;
  shdr->sh_type = SHT_SYMTAB;
  shdr->sh_flags = SHF_ALLOC;
  shdr->sh_entsize = sizeof(Elf64_Sym);
  shdr->sh_link = (unsigned short)strtabscn_index;
  shdr->sh_info = 2; // index of first non-local symbol

  // Create .shstrtab
  if ((scn = elf_newscn(e)) == NULL)
    errx(EXIT_FAILURE, "elf_newscn() failed: %s", elf_errmsg(-1));

  if ((data = elf_newdata(scn)) == NULL)
    errx(EXIT_FAILURE, "elf_newdata() failed: %s", elf_errmsg(-1));

  data->d_align = 1;
  data->d_buf = shstrtab;
  data->d_off = 0LL;
  data->d_size = sizeof(shstrtab);
  data->d_type = ELF_T_BYTE;
  data->d_version = EV_CURRENT;

  if ((shdr = elf64_getshdr(scn)) == NULL)
    errx(EXIT_FAILURE, "elf64_getshdr() failed: %s", elf_errmsg(-1));

  shdr->sh_name = 7;
  shdr->sh_type = SHT_STRTAB;
  shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
  shdr->sh_entsize = 0;

  ehdr->e_shstrndx = (unsigned short)elf_ndxscn(scn);

  // Write ELF

  /* if (elf_update(e, ELF_C_NULL) < 0) */
  /*   errx(EXIT_FAILURE, "elf_update(NULL) failed: %s", elf_errmsg(-1)); */

  if (elf_update(e, ELF_C_WRITE) < 0)
    errx(EXIT_FAILURE, "elf_update() failed: %s", elf_errmsg(-1));

  elf_end(e);
  close(fd);
}
