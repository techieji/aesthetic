#include "a.h"

void print_token(struct Lexed tok) {
	switch (tok.type) {
		case SYM: printf("SYM(%s)\n", tok.s); break;
		case STR: printf("STR(%s)\n", tok.s); break;
		case NUM: printf("NUM(%f)\n", tok.n); break;
		case END: printf("END\n"); break;
		case UTC: printf("`%c`\n", tok.c); break;
		case NOP: printf("NOP\n"); break;
		default: printf("UNKNOWN\n"); break;
	}
}


inline static void lex(char* inp) { s = inp; }

struct Lexed get_token() {
	int n;
	sscanf(s, " %n", &n);
	s += n;
	n = 0;
	struct Lexed tok;
	if (*s == '\0') { tok.type = END; return tok; }
	else if (strchr(SPECIAL_CHARS, *s) != NULL) { tok.c = *s++; tok.type = UTC; }
	else if (sscanf(s, "%f%n", &tok.n, &n)) tok.type = NUM;
	else if (sscanf(s, "\"%[^\"]\"%n", (tok.s = malloc(STR_SIZE)), &n)) tok.type = STR;
	else if (sscanf(s, "%s%n", (tok.s = malloc(SYM_SIZE)), &n)) tok.type = SYM;
	else puts("Error");
	s += n;
	return tok;
}

// Execution

struct Lexed lookup(struct Env e, char* k) {
	for (int i = 0; i < ENV_SIZE; i++)
		if ((e.vars + i)->name == k)
			return (e.vars + i)->value;
	if (e.parent != NULL)
		return lookup(*e.parent, k);
	struct Lexed nop;
	nop.type = NOP;
	return nop;
}

void define(struct Env e, struct Var v) {
	*(e.vars + e.size) = v;
}

struct Env child(struct Env e) {
	struct Env c;
	c.parent = &e;
	c.size = 0;
	return c;
}

struct Lexed run(struct Env e) {
	struct Lexed tok = get_token();
	switch (tok.type) {
		case SYM: return lookup(e, tok.s);
		case STR: case NUM: return tok;
		case END: case NOP: {
			struct Lexed nop;
			nop.type = NOP;
		return nop;
		}
		case UTC: {
			return tok;   // Delete later
		}
	}
	return tok;   // Delete later
}

int main() {
	lex("(define test (+ 1 2 \"asdf\"))");
	struct Lexed tok;
	while ((tok = get_token()).type != END) print_token(tok);
	return 0;
}
