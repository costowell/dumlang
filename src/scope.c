#include "scope.h"

scope_t *scope_init() {
  scope_t *scope = calloc(1, sizeof(scope_t));
  scope->stacksize = 8; // By default 8 because we push rbp for every function
  if (hashmap_create(8, &scope->map) != 0)
    return NULL;
  return scope;
}

const scope_var_t *scope_insert(scope_t *scope, char *str, uint8_t size) {
  const scope_var_t *elm =
      hashmap_get(&scope->map, str, (unsigned int)strlen(str));
  if (elm != NULL) {
    return NULL;
  }
  scope_var_t *scope_var = malloc(sizeof(scope_var_t));
  scope_var->size = size;
  scope_var->position = (int32_t)(-scope->stacksize);

  scope->stacksize += size;
  hashmap_put(&scope->map, str, (unsigned int)strlen(str), scope_var);

  return scope_var;
}

bool scope_remove(scope_t *scope, char *str) {
  return hashmap_remove(&scope->map, str, (unsigned int)strlen(str)) == 0;
}

const scope_var_t *scope_get(scope_t *scope, char *str) {
  const scope_var_t *elm =
      hashmap_get(&scope->map, str, (unsigned int)strlen(str));
  return elm;
}

scope_t *scope_clone(scope_t *scope) {
  scope_t *dup = malloc(sizeof(scope_t));
  return memcpy(dup, scope, sizeof(scope_t));
}
