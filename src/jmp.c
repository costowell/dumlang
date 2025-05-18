#include "jmp.h"
#include "codegen.h"
#include <stdio.h>

#define UNCOND_JMP_SIZE 5
#define COND_JMP_SIZE 6

jmptab_t *jmptab_init() {
  jmptab_t *tab = malloc(sizeof(jmptab_t));
  tab->first = NULL;
  tab->last = NULL;
  return tab;
}

void jmptab_free(jmptab_t *tab) {
  jmp_t *root = tab->first;
  while (root != NULL) {
    jmp_t *tmp = root->next;
    free(root);
    root = tmp;
  }
  free(tab);
}

void jmptab_insert(jmptab_t *tab, size_t loc, jmp_target_t target,
                   opcode_t op) {
  jmp_t *jmp = malloc(sizeof(jmp_t));
  jmp->target = target;
  jmp->loc = loc;
  jmp->op = op;
  jmp->next = NULL;

  if (tab->last)
    tab->last->next = jmp;
  if (!tab->first)
    tab->first = jmp;
  jmp->prev = tab->last;
  tab->last = jmp;
}

void jmptab_remove(jmptab_t *tab, jmp_t *jmp) {
  if (jmp->prev)
    jmp->prev->next = jmp->next;
  else
    tab->first = jmp->next;
  if (jmp->next)
    jmp->next->prev = jmp->prev;
  else
    tab->last = jmp->prev;
  free(jmp);
}

void jmptab_eval(jmptab_t *tab, jmp_target_t target, size_t value) {
  size_t curpos = text_get_pos();
  jmp_t *jmp = tab->first;
  while (jmp != NULL) {
    if (jmp->target == target) {
      text_set_pos(jmp->loc);
      size_t size = COND_JMP_SIZE;
      if (jmp->op == J_REL32)
        size = UNCOND_JMP_SIZE;
      write_jmp(jmp->op, (int32_t)(value - jmp->loc - size));

      jmp_t *tmp = jmp->next;
      jmptab_remove(tab, jmp);
      jmp = tmp;

    } else {
      jmp = jmp->next;
    }
  }
  text_set_pos(curpos);
}

void jmptab_print(jmptab_t *tab) {
  jmp_t *jmp = tab->first;
  while (jmp != NULL) {
    printf("%d: %ld\n", jmp->target, jmp->loc);
    jmp = jmp->next;
  }
}

void jmptab_merge(jmptab_t *dest, jmptab_t *src) {
  jmp_t *jmp = src->first;
  while (jmp != NULL) {
    jmptab_insert(dest, jmp->loc, jmp->target, jmp->op);
    jmp = jmp->next;
  }
}
