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

struct Value* cmac_if(struct ValueList* vl, struct ValueEnv* ve) {
  struct Value* b = idx(vl, 0);
  switch (b->type) {
    NUM: if (b->n != 0) goto tjmp; else goto fjmp;
    STR: if (b->s != NULL) goto tjmp; else goto fjmp;
    SYM: goto tjmp;
    LIST: if (b->l != NULL) goto tjmp; else goto fjmp;
    FN: goto tjmp;
    NIL: goto fjmp;
  }
  tjmp: return run(idx(vl, 1), ve);
  fjmp: return run(idx(vl, 2), ve);
}

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

struct Value* cfn_equal(struct ValueList* vl) {
  struct Value* v1 = idx(vl, 0);
  struct Value* v2 = idx(vl, 1);
  if (v1->type != v2->type) return false;
}

// Arith

struct Value* cfn_add(struct ValueList* vl) {
  struct Value* v1 = idx(vl, 0);
  struct Value* v2 = idx(vl, 1);
  assert(v1->type == NUM && v2->type == NUM);
  return number(v1->n + v2->n);
}

struct Value* cfn_sub(struct ValueList* vl) {
  struct Value* v1 = idx(vl, 0);
  struct Value* v2 = idx(vl, 1);
  assert(v1->type == NUM && v2->type == NUM);
  return number(v1->n - v2->n);
}

struct Value* cfn_mul(struct ValueList* vl) {
  struct Value* v1 = idx(vl, 0);
  struct Value* v2 = idx(vl, 1);
  assert(v1->type == NUM && v2->type == NUM);
  return number(v1->n * v2->n);
}

struct Value* cfn_div(struct ValueList* vl) {
  struct Value* v1 = idx(vl, 0);
  struct Value* v2 = idx(vl, 1);
  assert(v1->type == NUM && v2->type == NUM);
  return number(v1->n / v2->n);
}

struct ValueEnv* get_stdlib(void) {
  struct ValueEnv* e = child(NULL);
#if CORE
  env_set(e, symbol("display"), fn_to_value(cfn_display));
  env_set(e, symbol("if"), mac_to_value(cmac_if));
  env_set(e, symbol("lambda"), mac_to_value(cmac_lambda));
#endif
#if COREEXT
  env_set(e, symbol("define"), mac_to_value(cmac_define));
  env_set(e, symbol("true"), boolean(true));
  env_set(e, symbol("false"), boolean(false));
#endif
#if ARITH
  env_set(e, symbol("+"), fn_to_value(cfn_add));
  env_set(e, symbol("-"), fn_to_value(cfn_sub));
  env_set(e, symbol("*"), fn_to_value(cfn_mul));
  env_set(e, symbol("/"), fn_to_value(cfn_div));
#endif
  return e;
}
