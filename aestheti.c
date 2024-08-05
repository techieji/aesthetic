#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <setjmp.h>
#include "aestheti.h"
#include "lib.h"

#include <stdio.h>

/* * * * * *
 *  LEXER  *
 * * * * * */

struct Value* parse_string(char** s) {
    (*s)++;
    int i;
    for (i = 0; (*s)[i] != '"'; i++);   // TODO handle escapes and EOF
    struct Value* token = construct(STR, extract_string(*s, i));
    *s += i + 1;
    return token;
}

struct Value* parse_number(char** s) {
    struct Value* token;
    int i = 0;
    while (isdigit((*s)[++i]));
    if ((*s)[i] != '.')
        token = construct(INT, atoi(extract_string(*s, i)));
    else {
        while (isdigit((*s)[++i]));
        token = construct(FLOAT, atof(extract_string(*s, i)));
    }
    *s += i;
    return token;
}

struct Value* parse_symbol(char** s) {
    int i = 0;
    while (is_extended_alpha((*s)[++i]));
    struct Value* token = construct(SYM, extract_string(*s, i));
    *s += i;
    return token;
}

struct Value* parse_char(char** s) {
    (*s)++;
    switch (*(*s - 1)) {
        case '(': return construct(OPEN);
        case ')': return construct(CLOSE);
        case '\'': return construct(QUOTE);
        case '\0': return construct(END);
    }
    (*s)--;
    return NULL;
}

static struct Value* pushback = NULL;

struct Value* next_token(char** s) {
    while (isspace(**s)) (*s)++;
    struct Value* res;
    if (pushback != NULL) {
        res = pushback;
        pushback = NULL;
        return res;
    }
    if ((res = parse_char(s)) != NULL) return res;
    if (**s == '"')  return parse_string(s);
    if (isdigit(**s)) return parse_number(s);
    return parse_symbol(s);
}

struct Value* peek_token(char** s) {
    pushback = next_token(s);
    return pushback;
}

/* * * * * *
 * PARSER  *
 * * * * * */

jmp_buf parse_error;

struct Value* parse(char** s) {
    struct Value* tok = next_token(s);
    switch (tok->type) {
        case CLOSE: return tok;
        case OPEN:
            struct Value* l = construct(NIL);
            while ((tok = parse(s))->type != CLOSE) l = construct(PAIR, tok, l);
            return reverse(l);
        case QUOTE:
            return construct(PAIR, construct(SYM, "quote"), construct(PAIR, parse(s), construct(NIL)));
        case END:
            longjmp(parse_error, 1);
            return NULL;
        default:
            return tok;
    }
}

/* * * * * *
 *  EVAL   *
 * * * * * */
// Environments are lists of pairs: '((k1 . v1) (k2 . v2) ...)
// Functions are pairs: '((arg1 arg2 arg3 ...) . (<code>))        TODO add support for closures

struct Value* fix_null(struct Value* ptr) { return ptr != NULL ? ptr : construct_error("INTERNAL FUNCTION ERROR."); }

struct Value* eval(struct Value* t, struct Value** env) {
    if (t == NULL) return construct(NIL);
    switch (t->type) {
        case FLOAT: case INT: case STR: case ERROR:
            return t;
        case SYM:
            for (struct Value* v = *env; v->type != NIL; v = v->cdr)
                if (equal_values(v->car, t))
                    return v->cbr;
            return construct_error("SYMBOL %s IS NOT BOUND.", t->s);
        case PAIR:
            struct Value* fn = eval(t->car, env);
            if (fn->type == CMACRO)
                return fix_null(fn->cmac(t->cdr, env));

            struct Value* l = construct(PAIR, construct(END), construct(NIL));    // Dummy starting element
            struct Value* tmp;
            for (struct Value* v = t->cdr; v->type != NIL; v = v->cdr) {
                if ((tmp = eval(v->car, env))->type == ERROR) return tmp;
                l = construct(PAIR, tmp, l);
            }
            l = reverse(l);
            switch (fn->type) {
                case CFN:
                    return fix_null(fn->cfn(l->cdr));
                case FN:
                    struct Value* new_env = fn->cbr;
                    for (struct Value* var = fn->car; var->type != NIL; var = var->cdr)
                        new_env = construct_triple(var->car, (l = l->cdr)->car, new_env);
                    return eval(fn->cdr, &new_env);
                default:
                    return construct_error("NOT A FUNCTION.");
            }
        default:
            return construct_error("INVALID OBJECT. YOU ARE BEING A BIT GOOFY.");
    }
}

/* * * * * * * * * * *
 * GARBAGE COLLECTOR *
 * * * * * * * * * * */

extern struct Value* all_values;

void reset(void) {
    for (struct Value* ptr = all_values; ptr->type != NIL; ptr = ptr->cdr) ptr->mark = false;
}

void mark(struct Value* v) {
    if (v->mark) return;
    v->mark = true;
    if (v->type == PAIR || v->type == FN) {
            mark(v->car);
            mark(v->cbr);
            mark(v->cdr);
    }
}

void sweep(void) {
    struct Value *prev, *cur, *curelem;
    prev = all_values;       // FIXME bug: first element of list can never be garbage collected
    cur = prev;
    while ((cur = cur->cdr)->type != NIL) {
        if (!(curelem = cur->car)->mark) {
            prev->cdr = cur->cdr;
            destruct(cur->car);
            cur = prev;
        } else
            prev = cur;
    }
}

void gc(struct Value* env) {
    reset();
    mark(env);
    sweep();
}

/* * * * * * * * * * *
 *        MAIN       *
 * * * * * * * * * * */

struct Value* run_string(char* s, struct Value** env) {
    char** ptr = &s;
    struct Value* res = construct(NIL);
    while (peek_token(ptr)->type != END && res->type != ERROR) {
        struct Value* tree = setjmp(parse_error) == 0 ? parse(ptr) : construct_error("EOF ERROR.");
        res = eval(tree, env);
    }
    next_token(ptr);
    return res;
}

int main() {
    char* s = malloc(100 * sizeof(char));
    struct Value* env = get_stdlib();
    while (!feof(stdin)) {
        printf("> ");
        fgets(s, 100, stdin);
        print_value(run_string(s, &env));
        printf("\n");
        // gc(env);         // TODO doesn't work with quotation (symbols?)
    }
    return 0;
}
