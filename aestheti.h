#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define SPECIAL_CHARS "()'`,"
#define STR_SIZE 32
#define SYM_SIZE 16

extern char* s;    // String containing source code

/* Lexing */

extern struct Lexed* pushback_ptr;

enum TokenType { SYM, STR, NUM, UTC, END, NOP };

struct Lexed {
	enum TokenType type;
	union {
		float n;
		char* s;
		char c;
	};
};

void lex(char*);
void pushback();
void update_token(void);
struct Lexed nop(void);

/* Parsing */

struct BranchList;

struct ParseTree {
	bool is_single;
	struct Lexed* single;
	struct ParseTree* node;
	struct BranchList* branches;
};

struct BranchList {
	bool last;
	struct ParseTree* here;
	struct BranchList* next;
};

struct ParseTree* single(struct Lexed);
struct ParseTree* parse(void);
struct ParseTree* parse_expr(void);
struct ParseTree* topnode(struct ParseTree*, char*);
struct ParseTree* branchn(struct ParseTree*, int);

/* Execution */

// Environments

struct Assoc {
	char* key;
	struct Value* val;
	struct Assoc* next;
};

struct Env {
	struct Assoc* vars;
	struct Env* parent;
};

void assoc_bind(struct Assoc*, char*, struct Value*);
struct Value* assoc_get(struct Assoc*, char*);
void bind(struct Env*, char*, struct Value*);
struct Value* get(struct Env*, char*);   // Looks up from upper envs as well
struct Env* child(struct Env*);

// Actual execution

struct Function;
struct Macro;
struct Pair;

enum ValueType { VSTR, VNUM, VFUN, VMAC, VPAIR, VARGNAME };
struct Value {
	enum ValueType type;
	union {
		char* s;
		float n;
		struct Function* fn;
    struct Macro* mac;
		struct Pair* pr;
	};
};

void print_value(struct Value* v);

#define CHECK_TYPE(EXPR, TYPE) do { if ((EXPR)->type != TYPE) {\
                                fprintf(stderr, "TypeError: %s is not expected type %s", #EXPR, #TYPE);\
                                exit(1); \
                               } } while (0)

struct ArgList {
	struct Value* here;
	struct ArgList* next;
};

struct ArgList* _al_append(struct ArgList*, struct Value*);

enum FunctionType { PRIMITIVE, NORMAL };
struct Function {
  enum FunctionType type;
	struct ArgList* al;
  union {
    struct ParseTree* pt;
    struct Value* (*cfn)(struct ArgList*);
  };
};

struct Macro {
  enum FunctionType type;
  struct ArgList* al;
  union {
    struct ParseTree* pt;
    struct ParseTree(*cfn)(struct BranchList*, struct Env* e);
  };
};     // Could probably merge with Functions

struct Pair {
	struct Value* e1;
	struct Value* e2;
};

/* Execution */

struct Value* lexed_to_value(struct Lexed*, struct Env*);
struct Value* run(struct Env*, struct ParseTree*);

struct Env* get_base_stdlib(void);

/* Logging */

void print_token(struct Lexed);
void print_parsetree(struct ParseTree*, char*);
