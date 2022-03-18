#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
	_Bool is_single;
	struct Lexed* single;
	struct ParseTree* node;
	struct BranchList* branches;
};

struct BranchList {
	_Bool last;
	struct ParseTree* here;
	struct BranchList* next;
};

struct ParseTree* single(struct Lexed);
struct ParseTree* parse(void);
struct ParseTree* parse_expr(void);
struct ParseTree* topnode(struct ParseTree*, char*);

/* Execution */

// Environments

struct Assoc {
	char* key;
	struct Value* val;
	struct Assoc* next;
};

struct Env {
	struct Assoc vars;
	struct Env* parent;
};

void assoc_bind(struct Assoc*, char*, struct Value*);
struct Value* assoc_get(struct Assoc*, char*);
void bind(struct Env*, char*, struct Value*);
struct Value* get(struct Env*, char*);   // Looks up from upper envs as well

// Actual execution

struct Function;
struct Pair;

enum ValueType { VSTR, VNUM, VFUN, VPAIR, VARGNAME };
struct Value {
	enum ValueType type;
	union {
		char* s;
		float n;
		struct Function* fn;
		struct Pair* pr;
	};
};

struct ArgList {
	struct Value* here;
	struct ArgList* next;
};

struct Function {
	char* s;
	struct ArgList* al;
	struct ParseTree* pt;
};

struct Pair {
	struct Value* e1;
	struct Value* e2;
};

struct Value run(struct Env*, struct ParseTree*);
struct Value run_special_form(struct Env*, char*, struct ArgList*);
struct Value run_tree(struct Env*, struct ParseTree*);

/* Logging */

void print_token(struct Lexed);
void print_parsetree(struct ParseTree*, char*);
