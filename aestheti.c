#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPECIAL_CHARS "()"
#define STR_SIZE 128
#define SYM_SIZE 32

enum TokenType { SYM, STR, NUM, UTC, END };
struct Lexed {
	enum TokenType type;
	union { float n; char* s; char c; };
};

void print_token(struct Lexed tok) {
	switch (tok.type) {
		case SYM: printf("SYM(%s)\n", tok.s); break;
		case STR: printf("STR(%s)\n", tok.s); break;
		case NUM: printf("NUM(%f)\n", tok.n); break;
		case END: printf("END\n"); break;
		case UTC: printf("`%c`\n", tok.c); break;
		default: printf("UNKNOWN\n"); break;
	}
}

static char* s;

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

int main() {
	lex("(define test (+ 1 2 \"asdf\"))");
	struct Lexed tok;
	while ((tok = get_token()).type != END) print_token(tok);
	return 0;
}
