#define SPECIAL_CHARS "()"
#define STR_SIZE 128
#define SYM_SIZE 32
#define ENV_SIZE 16

#include <stddef.h>
int printf(const char*, ...);
int sscanf(const char*, const char*, ...);
int puts(const char*);
char* strchr(const char*, int);
void* malloc(size_t size);

enum TokenType { SYM, STR, NUM, UTC, END, NOP };

struct Lexed {
	enum TokenType type;
	union { float n; char* s; char c; };
};

struct Var {
	char name[SYM_SIZE];
	struct Lexed value;
};

struct Env {
	struct Var vars[ENV_SIZE];
	struct Env* parent;
	int size;
};

static char* s;

inline static void lex(char*);
struct Lexed get_token(void);
struct Lexed lookup(struct Env, char*);
void define(struct Env, struct Var);
struct Env child(struct Env);
struct Lexed run(struct Env);
