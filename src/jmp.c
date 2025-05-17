#include "jmp.h"
#include "codegen.h"

#define UNCOND_JMP_SIZE 5
#define COND_JMP_SIZE 6

jmptab_t *jmptab_init() {
  jmptab_t *tab = malloc(sizeof(jmptab_t));
  tab->len = 0;
  memset(tab->tab, 0, sizeof(tab->tab));
  return tab;
}

void jmptab_free(jmptab_t *tab) { free(tab); }

void jmptab_insert(jmptab_t *tab, size_t loc, jmp_target_t target,
                   opcode_t op) {
  jmp_t jmp = {
      .target = target,
      .loc = loc,
      .op = op,
  };
  tab->tab[tab->len++] = jmp;
}

void jmptab_eval(jmptab_t *tab, jmp_target_t target, size_t value) {
  size_t curpos = text_get_pos();
  for (size_t i = 0; i < tab->len; ++i) {
    if (tab->tab[i].target == target) {
      text_set_pos(tab->tab[i].loc);
      size_t size = COND_JMP_SIZE;
      if (tab->tab[i].op == J_REL32)
        size = UNCOND_JMP_SIZE;
      write_jmp(tab->tab[i].op, (int32_t)(value - tab->tab[i].loc - size));
    }
  }
  text_set_pos(curpos);
}
