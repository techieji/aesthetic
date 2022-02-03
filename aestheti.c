#include "a.h"

char* s;
struct Lexed* pushback_ptr = NULL;

void lex(char* inp) { s = inp; }
void pushback(struct Lexed l) { pushback_ptr = &l; }

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

// Execution

struct ParseTree parse(void) {
	struct Lexed tok = get_token();
	print_token(tok);
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
	while (!((tok = get_token()).type == UTC && tok.c == ')')) {
		pushback(tok);
		loop->here = parse();
		loop->last = 0;
		loop->next = (struct BranchList*)malloc(sizeof(struct BranchList));
		loop = loop->next;
	}
	print_token(tok);
	loop->last = 1;
	struct ParseTree pt = { 0, NULL, &full.here, full.next };
	return pt;
}

int main() {
	lex("(define test (+ 1 2 \"asdf\"))");
	//struct Lexed tok;
	//while ((tok = get_token()).type != END) print_token(tok);
	puts("Lexed");
	struct ParseTree pt = parse();
	puts("Parsed");
	print_parsetree(pt, "");
	return 0;
}
