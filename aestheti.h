#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <setjmp.h>
#include <stdarg.h>
#include <dlfcn.h>

enum Type {
    OPEN, CLOSE, QUOTE, END,              // Purely lexical
    SYM, FLOAT, INT, STR,                 // Both lexical and language
    BOOL, PAIR, FN, CFN, CMACRO, NIL,     // Purely language
    ERROR
};
struct Value {
    enum Type type;
    union {
        char* s;
        double f;
        int i;
        bool b;
        struct {
            struct Value* car;    // FN: list of arguments
            struct Value* cbr;    // FN: environment (lmao wtf is this cursed code)
            struct Value* cdr;    // FN: code
        };
        struct Value* (*cfn)(struct Value* list);
        struct Value* (*cmac)(struct Value* list, struct Value** env);
    };
    bool mark;
};

// API
struct Value* next_token(char**);
struct Value* parse(char**);
struct Value* run_string(char*, struct Value**);
struct Value* get_stdlib(void);
struct Value* eval(struct Value*, struct Value**);

// Helpers
void print_value(struct Value*);
struct Value* construct(enum Type, ...);
void destruct(struct Value**);
struct Value* construct_triple(struct Value*, struct Value*, struct Value*);
struct Value* construct_error(char*, ...);
char* extract_string(char*, int);
bool is_extended_alpha(char);
struct Value* reverse(struct Value*);
