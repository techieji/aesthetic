#include "../aestheti.h"

void chain(int len, ...) {
    if (len == 0) return;
    va_list l;
    va_start(l, len);
    struct Value* prev = va_arg(l, struct Value*);
    for (int i = 1; i < len; i++)
        prev = prev->cdr = va_arg(l, struct Value*);
    prev->cdr = construct(NIL);
    va_end(l);
}

bool equal_values(struct Value* v1, struct Value* v2) {
    if (v1->type != v2->type) return false;
    switch (v1->type) {
        case OPEN:
        case CLOSE:
        case NIL:
        case QUOTE:
        case END:
            return true;
        case SYM:
        case STR:
        case ERROR:
            return strcmp(v1->s, v2->s) == 0;
        case INT:
        case BOOL:
        case FLOAT:    // Is this legal and valid? IDK
        case CFN:
        case CMACRO:
            return v1->i == v2->i;
        case PAIR:     // May not work for very long lists
        case FN:
            return equal_values(v1->car, v2->car) && equal_values(v1->car, v2->car) && equal_values(v1->cbr, v2->cbr);
    }
    return false;
}

struct Value* add_two(struct Value* v1, struct Value* v2) {
    if (v1->type == INT) {
        if (v2->type == INT) return construct(INT, v1->i + v2->i);
        if (v2->type == FLOAT) return construct(FLOAT, v1->i + v2->f);
    }
    if (v1->type == FLOAT) {
        if (v2->type == INT) return construct(FLOAT, v1->f + v2->i);
        if (v2->type == FLOAT) return construct(FLOAT, v1->f + v2->f);
    }
    printf("TYPE ERROR IN ADD_TWO. INDUCED SEGFAULT.");
    return NULL;

}

// TODO add accessors to type check
struct Value* add(struct Value* args) {
    struct Value* v = args->car;
    for (struct Value* l = args->cdr; l->type != NIL; l = l->cdr) {
        v = add_two(v, l->car);
    }
    return v;
}
struct Value* neg(struct Value* args) { return construct(INT, -args->car->i); }

struct Value* exit_(struct Value* args) {
    if (args->type == NIL) exit(0);
    exit(args->car->i);
    return NULL;
}

struct Value* define(struct Value* args, struct Value** env) {
    bool cond = (args->car->type == PAIR);
    struct Value* k = cond ? args->car->car : args->car;
    struct Value* v = cond ? construct(FN, *env, args->car->cdr, args->cdr->car) : eval(args->cdr->car, env);
    *env = construct_triple(k, v, *env);
    return v;
}

struct Value* lambda(struct Value* args, struct Value** env) { return construct(FN, *env, args->car, args->cdr->car); }

struct Value* get_env(struct Value* args, struct Value** env) { return *env; }
struct Value* quote(struct Value* args, struct Value** env) { return args->car; }
struct Value* if_(struct Value* args, struct Value** env) { return eval((eval(args->car, env))->b ? args->cdr->car : args->cdr->cdr->car, env); }

struct Value* car(struct Value* args) { return args->car->car; }
struct Value* cbr(struct Value* args) { return args->car->cbr; }
struct Value* cdr(struct Value* args) { return args->car->cdr; }

struct Value* display(struct Value* v) { print_value(v->car); return construct(NIL); }
struct Value* equal(struct Value* vs) { return construct(BOOL, equal_values(vs->car, vs->cdr->car)); }

struct Value* load(struct Value* args, struct Value** env) {
    FILE* fp = fopen(eval(args->car, env)->s, "r");
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    char* s = malloc(size * sizeof(char));
    rewind(fp);
    fread(s, sizeof(char), size, fp);
    return run_string(s, env);
}

struct Value* load_c_fn(struct Value* args) {
    char* s;
    void* so = dlopen(args->car->s, RTLD_NOW | RTLD_NODELETE);
    if ((s = dlerror())) { puts(s); return NULL; }
    struct Value* ret = construct(CFN, dlsym(so, args->cdr->car->s));
    if ((s = dlerror())) { puts(s); return NULL; }
    dlclose(so);
    if ((s = dlerror())) { puts(s); return NULL; }
    return ret;
}

#define DECL(name, type, cfn) construct_triple(construct(SYM, name), construct(type, cfn), NULL)
#define GVAR(name, ...) construct_triple(construct(SYM, name), construct(__VA_ARGS__), NULL)

struct Value* get_stdlib(void) {
    struct Value* env;
    env = construct_triple(construct(SYM, "globals"), NULL, NULL);
    env->cbr = env;
    chain(15,           // UPDATE THIS WHEN ADDING NEW DECLARATIONS
        env,
        DECL("+", CFN, add),
        DECL("exit", CFN, exit_),
        DECL("define", CMACRO, define),
        DECL("lambda", CMACRO, lambda),
        DECL("get-env", CMACRO, get_env),
        DECL("quote", CMACRO, quote),
        DECL("if", CMACRO, if_),
        DECL("car", CFN, car),
        DECL("cbr", CFN, cbr),
        DECL("cdr", CFN, cdr),
        DECL("=", CFN, equal),
        DECL("display", CFN, display),
        GVAR("nil", NIL),
        GVAR("true", BOOL, true),
        GVAR("false", BOOL, false),
        DECL("neg", CFN, neg),
        DECL("load", CMACRO, load),
        DECL("c-fn", CFN, load_c_fn)
    );
    return env;
}
