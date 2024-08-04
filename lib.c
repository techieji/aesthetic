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
        case SYM: case STR: printf("%s", tree->s); return;
        case FLOAT: printf("%lf", tree->f); return;
        case INT: printf("%d", tree->i); return;
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
        default: puts("[undefined]");
    }
}

struct Value* singleton(enum Type type) {
    struct Value* token = malloc(sizeof(struct Value));
    token->type = type;
    return token;
}

struct Value* boolean(bool b) {
    struct Value* token = malloc(sizeof(struct Value));
    token->type = BOOL;
    token->b = b;
    return token;
}

struct Value* construct(enum Type type, ...) {
    struct Value* v = malloc(sizeof(struct Value));
    v->type = type;
    va_list l;
    va_start(l, type);
    switch (type) {
        case OPEN:
        case CLOSE:
        case NIL:
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
        case SYM: case STR:
            v->s = va_arg(l, char*);
            break;
        case PAIR:
        case FN:     // TODO update when adding closures
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

char* extract_string(char* s, int i) {
    char* str = malloc((i + 1) * sizeof(char));
    strncpy(str, s, i);
    str[i] = '\0';
    return str;
}

struct Value* pair(struct Value* v1, struct Value* v2) {
    struct Value* pair = malloc(sizeof(struct Value));
    pair->type = PAIR;
    pair->car = v1;
    pair->cdr = v2;
    return pair;
}

struct Value* reverse(struct Value* l) {
    struct Value* newl = singleton(NIL);
    while (l->type != NIL) {
        newl = pair(l->car, newl);
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
        case END:
            return true;
        case SYM:
        case STR:
            return strcmp(v1->s, v2->s) == 0;
        case INT:
        case BOOL:
        case FLOAT:    // Is this legal and valid? IDK
        case CFN:
        case CMACRO:
            return v1->i == v2->i;
        case PAIR:     // May not work for very long lists
        case FN:
            return equal_values(v1->car, v2->car) && equal_values(v1->car, v2->car);
    }
    return false;
}

bool is_extended_alpha(char c) {
    return isalpha(c) || isdigit(c) || (strchr("+-.*/<=>!?:$%_&~^", c) != NULL);
}

/* * * * * * * * * * * * * * * *
 * STDLIB INITIALIZATION       *
 * * * * * * * * * * * * * * * */

struct Value* add(struct Value* l) {
    return construct(l->car->i + l->cdr->car->i);     // TODO add accessors
}

struct Value* get_stdlib(void) {
    struct Value* env = singleton(NIL);
    env = construct(PAIR,
            construct(PAIR, construct(SYM, "+"), construct(CFN, add)),
            construct(NIL));
    return env;
}
