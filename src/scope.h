#ifndef _SCOPE_H
#define _SCOPE_H

#include "hashmap.h"

#include <stdint.h>

typedef struct _scope {
    uint32_t stacksize;
    hashmap_t map;
} scope_t;

typedef struct _scope_var {
    uint32_t position;
    uint8_t size;
} scope_var_t;

scope_t *scope_init();
const scope_var_t *scope_insert(scope_t *scope, char *str, uint8_t size);
const scope_var_t *scope_get(scope_t *scope, char *str);
scope_t *scope_clone(scope_t *scope);

#endif
