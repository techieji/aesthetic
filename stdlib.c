#include "aestheti.h"

struct Value* _fn_to_value(struct Value* fn(struct ArgList*)) {
  struct Value* v = malloc(sizeof(struct Value));
  v->type = VFUN;
  struct Function* f = malloc(sizeof(struct Function));
  f->type = PRIMITIVE;
  f->cfn = fn;
  v->fn = f;
  return v;
}

struct Value* cfn_add(struct ArgList* al) {
  struct Value* v1 = al->here;
  struct Value* v2 = al->next->here;    // Handle null pointer (functionize)
  CHECK_TYPE(v1, VNUM);
  CHECK_TYPE(v2, VNUM);
  struct Value* vp = malloc(sizeof(struct Value));
  vp->type = VNUM;
  vp->n = v1->n + v2->n;
  return vp;
}   // That's a lot of code!

struct Env* get_base_stdlib(void) {
  struct Env* e = child(NULL);
  bind(e, "+", _fn_to_value(cfn_add));
  return e;
}
