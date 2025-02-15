#ifndef _CODEGEN_H
#define _CODEGEN_H

#include "parse.h"
#include "hashmap.h"

typedef struct _scope_t {
    hashmap_t nametomem;
    char *memtoname[64];
    struct _scope_t *next;
} scope_t;

void gen_object(function_t **funcs, const char *file);

#endif // _CODEGEN_H
