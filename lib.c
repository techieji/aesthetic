#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include "lib.h"

void print_token(struct Value* tok) {
    switch (tok->type) {
        case OPEN: puts("OPEN"); return;
        case CLOSE: puts("CLOSE"); return;
        case QUOTE: puts("QUOTE"); return;
        case SYM: printf("SYM\t%s\n", tok->s); return;
        case FLOAT: printf("FLOAT\t%lf\n", tok->f); return;
        case INT: printf("INT\t%d\n", tok->i); return;
        case STR: printf("STR\t%s\n", tok->s); return;
        case END: puts("END"); return;
        default: puts("[not a token]");
    }
}

void print_tree(struct Value* tree, int indent_level) {
    if (tree->type == PAIR)
        for (struct Value* ptr = tree; ptr->type != NIL; ptr = ptr->cdr)
            print_tree(ptr->car, indent_level + 1);
    else {
        for (int i = 0; i < indent_level; i++) printf("  ");
        print_token(tree);
    }
}

void print_value(struct Value* tree) {
    switch (tree->type) {
        case SYM: case STR: case ERROR: printf("%s", tree->s); return;
        case FLOAT: printf("%lf", tree->f); return;
        case INT: printf("%d", tree->i); return;
        case BOOL: printf("%s", tree->b ? "true" : "false"); return;
        case NIL: printf("nil"); return;
        case PAIR:
            printf("(");
            print_value(tree->car);
            while ((tree = tree->cdr)->type == PAIR) {
                printf(" ");
                print_value(tree->car);
            }
            if (tree->type != NIL) {
                printf(" . ");
                print_value(tree);
            }
            printf(")");
            return;
        case CFN:
            printf("[compiled function at %p]", tree->cfn);
            return;
        case FN:
            printf("[user-defined function]");
            return;
        default: printf("[undefined]");
    }
}

/*
int snprint_value(char* buf, size_t maxn, struct Value* tree) {
    if (maxn == 0) return 0;
    int n = 0;
    switch (tree->type) {
        case SYM: case ERROR: return snprintf(buf, maxn "%s", tree->s);
        case STR: return snprintf(buf, maxn, "\"%s\"", tree->s);
        case FLOAT: return snprintf(buf, maxn, "%lf", tree->f);
        case INT: return snprintf(buf, maxn, "%d", tree->i);
        case NIL: return snprintf(buf, maxn, "nil");
        case PAIR:
            n += snprintf(buf + n, "(");
            n += snprint_value(buf + n, tree->car);
            while ((tree = tree->cdr)->type == PAIR) {
                n += sprintf(buf + n, " ");
                n += sprint_value(buf + n, tree->car);
            }
            if (tree->type != NIL) {
                n += sprintf(buf + n, " . ");
                n += sprint_value(buf + n, tree);
            }
            n += sprintf(buf + n, ")");
            return n;
        case CFN:
            return sprintf(buf, "[compiled function at %p]", tree->cfn);
        case FN:
            return sprintf(buf, "[user-defined function]");
        default: return printf(buf, "[undefined]");
    }
}
*/

struct Value* all_values = NULL;

void register_value(struct Value* v) {
    if (all_values == NULL) {
        all_values = malloc(sizeof(struct Value));
        all_values->type = NIL;
    }
    struct Value* temp = malloc(sizeof(struct Value));
    temp->type = PAIR;
    temp->car = v;
    temp->cdr = all_values;
    all_values = temp;
}

struct Value* construct(enum Type type, ...) {
    struct Value* v = malloc(sizeof(struct Value));
    v->type = type;
    register_value(v);
    va_list l;
    va_start(l, type);
    switch (type) {
        case OPEN:
        case CLOSE:
        case NIL:
        case QUOTE:
        case END:
            return v;
        case INT:
            v->i = va_arg(l, int);
            break;
        case FLOAT:
            v->f = va_arg(l, double);
            break;
        case BOOL:
            v->b = va_arg(l, int);
            break;
        case SYM:
        case STR:
        case ERROR:
            v->s = va_arg(l, char*);
            break;
        case FN:
            v->cbr = va_arg(l, struct Value*);
        case PAIR:
            if (type != FN) v->cbr = construct(NIL);
            v->car = va_arg(l, struct Value*);
            v->cdr = va_arg(l, struct Value*);
            break;
        case CFN:
            v->cfn = va_arg(l, struct Value*(*)(struct Value* list));
            break;
        case CMACRO:
            v->cmac = va_arg(l, struct Value*(*)(struct Value* list, struct Value** env));
            break;
    }
    va_end(l);
    return v;
}

void destruct(struct Value* v) {
    if (v->type == SYM || v->type == STR || v->type == ERROR) free(v->s);
    free(v);
}

struct Value* construct_triple(struct Value* car, struct Value* cbr, struct Value* cdr) {
    struct Value* pair = construct(PAIR, car, cdr);
    pair->cbr = cbr;
    return pair;
}

struct Value* construct_error(char* fmt, ...) {
    char* buf = malloc(100 * sizeof(char));
    va_list l;
    va_start(l, fmt);
    vsprintf(buf, fmt, l);
    va_end(l);
    return construct(ERROR, buf);
}

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

char* extract_string(char* s, int i) {
    char* str = malloc((i + 1) * sizeof(char));
    strncpy(str, s, i);
    str[i] = '\0';
    return str;
}

struct Value* reverse(struct Value* l) {
    struct Value* newl = construct(NIL);
    while (l->type != NIL) {
        newl = construct(PAIR, l->car, newl);
        l = l->cdr;
    }
    return newl;
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

bool is_extended_alpha(char c) {
    return isalpha(c) || isdigit(c) || (strchr("+-.*/<=>!?:$%_&~^", c) != NULL);
}

/* * * * * * * * * * * * * * * *
 * STDLIB INITIALIZATION       *
 * * * * * * * * * * * * * * * */

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
    struct Value* k = args->car;
    struct Value* v = eval(args->cdr->car, env);
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
struct Value* equal(struct Value* vs) {
    return construct(BOOL, equal_values(vs->car, vs->cdr->car));
}

#define DECL(name, type, cfn) construct_triple(construct(SYM, name), construct(type, cfn), NULL)
#define GVAR(name, ...) construct_triple(construct(SYM, name), construct(__VA_ARGS__), NULL)

struct Value* get_stdlib(void) {
    struct Value* env;
    env = construct_triple(construct(SYM, "globals"), NULL, NULL);
    env->cbr = env;
    chain(17,           // UPDATE THIS WHEN ADDING NEW DECLARATIONS
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
        DECL("neg", CFN, neg)
    );
    return env;
}
