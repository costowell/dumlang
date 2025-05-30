#ifndef _SCOPE_H
#define _SCOPE_H

#include "hashmap.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct _scope {
  uint32_t stacksize;
  hashmap_t map;
} scope_t;

typedef struct _scope_var {
  int32_t position;
  uint8_t size;
  bool immutable;
} scope_var_t;

scope_t *scope_init();
const scope_var_t *scope_insert(scope_t *scope, char *str, uint8_t size);
const scope_var_t *scope_insert_immutable(scope_t *scope, char *str,
                                          uint8_t size, bool immutable);
const scope_var_t *scope_get(scope_t *scope, char *str);
scope_t *scope_clone(scope_t *scope);
bool scope_remove(scope_t *scope, char *str);

#endif
