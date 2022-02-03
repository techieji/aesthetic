#define SPECIAL_CHARS "()"
#define STR_SIZE 32
#define SYM_SIZE 16
#define ENV_SIZE 16

#include <stddef.h>
int printf(const char*, ...);
int sscanf(const char*, const char*, ...);
int puts(const char*);
char* strchr(const char*, int);
void* malloc(size_t size);

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
	struct ParseTree here;
	struct BranchList* next;
};

struct ArgList {
	_Bool last;
	char* here;
	struct ArgList* next;
};

struct Function {
	char* name;
	struct ArgList;
	struct ParseTree pt;
};

char* s;
struct Lexed* pushback_ptr = NULL;

void lex(char*);
void pushback(struct Lexed);
struct Lexed get_token(void);
struct Lexed lookup(struct Env, char*);
struct Lexed nop(void);
void define(struct Env, struct Var);
struct Env child(struct Env);

struct ParseTree single(struct Lexed);
struct ParseTree parse(void);
struct ParseTree parse_expr(void);
