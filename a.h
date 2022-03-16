#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SPECIAL_CHARS "()'`,"
#define STR_SIZE 32
#define SYM_SIZE 16
#define ENV_SIZE 16

enum TokenType { SYM, STR, NUM, UTC, END, NOP, FUN };

struct Function;

struct Lexed {
	enum TokenType type;
	union {
		float n;
		char* s;
		char c;
		struct Function* f;
	};
};

struct Var {
	char* name;
	struct Lexed value;
};

struct Env {
	struct Var vars[ENV_SIZE];
	struct Env* parent;
	int size;
};

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

struct ArgList {
	_Bool last;
	struct Lexed* here;
	struct ArgList* next;
};

struct Function {
	char* name;
	struct ArgList args;
	struct ParseTree pt;
};

extern char* s;
extern struct Lexed* pushback_ptr;

void lex(char*);
void pushback();
void update_token(void);
struct Lexed lookup(struct Env, char*);
struct Lexed nop(void);

struct Env child(struct Env*);
struct Lexed lookup(struct Env*, char*);
void set(struct Env* env, char* name, struct Lexed value);

struct ParseTree* single(struct Lexed);
struct ParseTree* parse(void);
struct ParseTree* parse_expr(void);
struct ParseTree* topnode(struct ParseTree*, char*);

void print_token(struct Lexed);
void print_parsetree(struct ParseTree*, char*);
