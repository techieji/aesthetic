#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include <stdnoreturn.h>

#define STR_SIZE 128
#define SYM_SIZE 16    // Small variables

// Rename
#define MAKER(name, arg_type, enum_type, field) struct Value* name (arg_type s) {\
                                       struct Value* v = malloc(sizeof(struct Value));\
                                       v->type = enum_type; v-> field = s;\
                                       return v; }

// Values

struct ValueList;
struct ValueDict;
struct ValueEnv;
struct Function;

enum ValueType { NUM, STR, SYM, CHAR, BOOL, LIST, DICT, ENV, NIL, FN };
struct Value {
  enum ValueType type;
  union {
    float n;
    char* s;
    char c;
    bool b;
    struct ValueList* l;
    struct ValueDict* d;
    struct ValueEnv* e;
    struct Function* fn;
  };
};

struct ValueList {
  struct Value* here;
  struct ValueList* next;
};

struct ValueDict {
  struct ValueList* keys;
  struct ValueList* values;
};

struct ValueEnv {
  struct ValueDict* d;
  struct ValueEnv* upper;
};

enum FunctionType { NORMAL, CFN, MACRO, CMAC };
struct Function {
  enum FunctionType type;
  struct ValueList* argnames;   // Can this be removed/moved into the union?
  union {
    struct Value* code;
    struct Value* (*cfn)(struct ValueList*);
    struct Value* (*mfn)(struct ValueList*, struct ValueEnv*);
  };
};

bool value_equal(struct Value*, struct Value*);

struct Value* number(float);   // Turn into a single macro (20 loc)
struct Value* symbol(char*);
struct Value* string(char*);
struct Value* boolean(bool);
struct Value* nil(struct ValueList*);
struct Value* list_to_value(struct ValueList*);

struct ValueList* append(struct ValueList*, struct Value*);
struct ValueList* value_list(int i, ...);
struct ValueList* vvalue_list(int i, va_list l);
int find(struct ValueList*, struct Value*);
struct Value* idx(struct ValueList*, int);

struct Value* dict_get(struct ValueDict*, struct Value*);
struct ValueDict* dict_set(struct ValueDict*, struct Value*, struct Value*);
struct Value* env_get(struct ValueEnv*, struct Value*);
struct ValueEnv* env_set(struct ValueEnv*, struct Value*, struct Value*);
struct ValueEnv* child(struct ValueEnv*);

// Lexer

struct Lexer {
  char* s;      // String being parsed
  struct Value* backlog;    // Token previously parsed
};

struct Lexer* lex(char*);
struct Lexer* pushback(struct Lexer*, struct Value*);
struct Value* get_token(struct Lexer*);

// Parsing

struct Value* parse(struct Lexer*);
struct Value* parse_expr(struct Lexer*);

// Running

struct Value* call(struct Function*, struct ValueList*, struct ValueEnv*);
struct Value* run(struct Value*, struct ValueEnv*);
struct ValueEnv* get_stdlib(void);

// Logging

#define log_value(v) print_value(*v, true, "\n", stdout)
void print_value(struct Value, bool, char*, FILE*);
