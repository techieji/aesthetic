#pragma once
#include "../aestheti.h"

#define FN_DECL(name) struct Value* name(struct Value*);
#define MC_DECL(name) struct Value* name(struct Value*, struct Value**);

FN_DECL(add);
FN_DECL(neg);
FN_DECL(exit_);
FN_DECL(car);
FN_DECL(cbr);
FN_DECL(cdr);
FN_DECL(display);
FN_DECL(equal);

MC_DECL(define);
MC_DECL(lambda);
MC_DECL(get_env);
MC_DECL(quote);
