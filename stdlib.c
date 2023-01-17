#include "stdlib.h"

struct Value* fn_to_value(struct Value* (*cfn)(struct ValueList*)) {
  struct Value* v = malloc(sizeof(struct Value));
  v->type = FN;
  v->fn = malloc(sizeof(struct Function));
  v->fn->type = CFN;
  v->fn->cfn = cfn;
  return v;
}

struct Value* mac_to_value(struct Value* (*mfn)(struct ValueList*, struct ValueEnv*)) {
  struct Value* v = malloc(sizeof(struct Value));
  v->type = FN;
  v->fn = malloc(sizeof(struct Function));
  v->fn->type = CMAC;
  v->fn->mfn = mfn;
  return v;
}

// Core

struct Value* cfn_display(struct ValueList* vl) {
  print_value(*idx(vl, 0), false, "\n", stdout);
  return idx(vl, 0);   // Return NIL!!
}

struct Value* cmac_lambda(struct ValueList* vl, struct ValueEnv* ve) {
  struct Value* v = malloc(sizeof(struct Value));
  v->type = FN;
  v->fn = malloc(sizeof(struct Function));
  v->fn->type = NORMAL;
  v->fn->argnames = vl->here->l;
  v->fn->code = vl->next->here;
  return v;
}

// CoreExt

struct Value* cmac_define(struct ValueList* vl, struct ValueEnv* ve) {
  struct Value* v1 = idx(vl, 0);
  struct Value* v2 = idx(vl, 1);
  assert(v1->type == SYM);
  env_set(ve, v1, run(v2, ve));
}

// Arith

struct Value* cfn_add(struct ValueList* vl) {
  struct Value* v1 = idx(vl, 0);
  struct Value* v2 = idx(vl, 1);
  assert(v1->type == NUM && v2->type == NUM);
  return number(v1->n + v2->n);
}

struct ValueEnv* get_stdlib(void) {
  struct ValueEnv* e = child(NULL);
#if CORE
  env_set(e, symbol("display"), fn_to_value(cfn_display));
  env_set(e, symbol("lambda"), mac_to_value(cmac_lambda));
#endif
#if COREEXT
  env_set(e, symbol("define"), mac_to_value(cmac_define));
#endif
#if ARITH
  env_set(e, symbol("+"), fn_to_value(cfn_add));
#endif
  return e;
}
