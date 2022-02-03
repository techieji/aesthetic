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


inline void lex(char* inp) { s = inp; }
inline void pushback(struct Lexed l) { pushback_ptr = l; }

struct Lexed get_token(void) {
	if (pushback_ptr != NULL) {
		struct Lexed pb = *pushback_ptr;
		pushback_ptr = NULL;
		return pb;
	}
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

struct Lexed nop(void) {
	struct Lexed l;
	l.type = NOP;
	return l;
}

// Execution

struct Lexed lookup(struct Env e, char* k) {
	for (int i = 0; i < ENV_SIZE; i++)
		if ((e.vars + i)->name == k)
			return (e.vars + i)->value;
	if (e.parent != NULL)
		return lookup(*e.parent, k);
	return nop();
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

struct ParseTree single(struct Lexed l) {
	struct ParseTree pt = { l, 0, NULL };
	return pt;
}

struct ParseTree parse(void) {
	struct Lexed tok = get_token();
	switch (tok.type) {
		case SYM: case NUM: case STR: return single(tok);
		case UTC:
			if (tok.c == '(') return parse_expr();
		default: return single(nop());
	}
}

struct ParseTree parse_expr(void) {
	struct Lexed tok;
	struct BranchList full, *loop = &full;
	while ((tok = get_token()).type != UTC || tok.c == ')') {
		pushback(tok);
		loop->here = parse();
		loop->last = 0;
		loop->next = (struct BranchList*)malloc(sizeof(struct BranchList));
		loop = loop->next;
	}
	loop->last = 1;
	struct ParseTree pt = { full.here };  // continue
}

int main() {
	lex("(define test (+ 1 2 \"asdf\"))");
	struct Lexed tok;
	while ((tok = get_token()).type != END) print_token(tok);
	return 0;
}
