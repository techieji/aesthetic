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

struct ParseTree {
	struct Lexed node;
	int branchlen;
	struct ParseTree* branches;
};

struct Function {
	char* name;
	int arglen;
	char** args;
	struct ParseTree pt;
};

static char* s;

inline static void lex(char*);
struct Lexed get_token(void);
struct Lexed lookup(struct Env, char*);
void define(struct Env, struct Var);
struct Env child(struct Env);

struct ParseTree single(struct Lexed);
struct ParseTree parse();
