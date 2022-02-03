#define SPECIAL_CHARS "()"
#define ENV_SIZE 16

#include <stddef.h>
int printf(const char*, ...);
int sscanf(const char*, const char*, ...);
int puts(const char*);
char* strchr(const char*, int);
void* malloc(size_t size);

struct Function;

enum TokenType { SYM, STR, NUM, UTC, END, NOP, FUN };

struct Lexed {
	enum TokenType type;
	union {
		float n;
		char* s;
		char c;
		struct Function;
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
	struct Lexed node;
	struct BranchList branches;
};

struct BranchList {
	_Bool last;
	struct ParseTree here;
	struct BranchList* next;
};

struct ArgList;

struct Function {
	char* name;
	struct ArgList;
	struct ParseTree pt;
};

struct ArgList {
	_Bool last;
	char* here;
	struct ArgList* next;
};

char* s;
struct Lexed* pushback_ptr = NULL;

inline void lex(char*);
inline void pushback(struct Lexed);
struct Lexed get_token(void);
struct Lexed lookup(struct Env, char*);
struct Lexed nop(void);
void define(struct Env, struct Var);
struct Env child(struct Env);

struct ParseTree single(struct Lexed);
struct ParseTree parse(void);
struct ParseTree parse_expr(void);
