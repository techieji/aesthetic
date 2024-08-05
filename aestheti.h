#pragma once
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
            struct Value* cbr;    // lmao wtf am I writing
                                  // FN: environment
            struct Value* cdr;    // FN: code
        };
        struct Value* (*cfn)(struct Value* list);
        struct Value* (*cmac)(struct Value* list, struct Value** env);
    };
    bool mark;
};

struct Value* next_token(char**);
struct Value* parse(char**);
struct Value* run_string(char*, struct Value**);
struct Value* eval(struct Value*, struct Value**);
