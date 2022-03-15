#include "a.h"

char* s;
struct Lexed current; 
_Bool backlog = 0;

// Lexing

void lex(char* inp) { s = inp; }
void pushback() { backlog = 1; }

void update_token(void) {
	if (backlog) {
		backlog = 0;
	} else {
		int n;
		sscanf(s, " %n", &n);
		s += n;
		n = 0;
		struct Lexed tok;
		if (*s == '\0') { tok.type = END; current = tok; }
		else if (strchr(SPECIAL_CHARS, *s) != NULL) { tok.c = *s++; tok.type = UTC; }
		else if (sscanf(s, "%f%n", &tok.n, &n)) tok.type = NUM;
		else if (sscanf(s, "\"%[^\"]\"%n", (tok.s = malloc(STR_SIZE)), &n)) tok.type = STR;
		else if (sscanf(s, "%s%n", (tok.s = malloc(SYM_SIZE)), &n)) tok.type = SYM;
		else puts("Error");
		s += n;
		current = tok;
	}
}

// Parsing

struct ParseTree* parse(void) {
	update_token();
	switch (current.type) {
		case SYM: case NUM: case STR: return single(current);
		case UTC:
			if (current.c == '(') return parse_expr();
		default: return single(nop());
	}
}

struct ParseTree* parse_expr(void) {
	struct BranchList full, *loop = &full;
	update_token();
	while (!(current.type == UTC && current.c == ')')) {
		pushback();
		loop->here = parse();
		loop->last = 0;
		loop->next = (struct BranchList*)malloc(sizeof(struct BranchList));
		loop = loop->next;
		update_token();
	}
	loop->last = 1;
	struct ParseTree* pt = malloc(sizeof(struct ParseTree));
	*pt = (struct ParseTree){ 0, NULL, full.here, full.next };
	return pt;
}

int main() {
	char* s = "(define test '1)";
	puts(s);
	// lex(s);
	// update_token();
	// while (current.type != END) { print_token(current); update_token(); }
	// return 0;
	//
	// struct ParseTree* pt = parse();
	// puts("Parsed");
	// print_parsetree(pt, "");
	char* new = preprocess_str(s);
	puts(s);
	return 0;
}
