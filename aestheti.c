#include "aestheti.h"

/* Utility */

struct Lexed nop(void) {
	struct Lexed l;
	l.type = NOP;
	return l;
}

struct ParseTree* single(struct Lexed l) {
	struct Lexed* lt = malloc(sizeof(struct Lexed));
	memcpy(lt, &l, sizeof(struct Lexed));
	struct ParseTree* pt = malloc(sizeof(struct ParseTree));
	*pt = (struct ParseTree){ 1, lt, NULL, NULL };
	return pt;
}

struct BranchList* single_list(struct ParseTree* pt) {
	struct BranchList* bl = malloc(sizeof(struct BranchList));
	struct BranchList* last = malloc(sizeof(struct BranchList));
	*bl = (struct BranchList){ 0, pt, last };
	*last = (struct BranchList){ 1, NULL, NULL };
	return bl;
}

struct ParseTree* topnode(struct ParseTree* pt, char* node_name) {
	struct Lexed name;
	name.type = SYM;
	name.s = node_name;
	struct ParseTree* t = malloc(sizeof(struct ParseTree));
	*t = (struct ParseTree){ 0, NULL, single(name), single_list(pt)};
	return t;
}

// Logging

void print_token(struct Lexed tok) {
	switch (tok.type) {
		case SYM: printf("SYM(%s)\n", tok.s); break;
		case STR: printf("STR(%s)\n", tok.s); break;
		case NUM: printf("NUM(%f)\n", tok.n); break;
		case END: printf("END\n"); break;
		case UTC: printf("CHAR `%c`\n", tok.c); break;
		case NOP: printf("NOP\n"); break;
		default: printf("UNKNOWN\n"); break;
	}
}

void print_parsetree(struct ParseTree* pt, char* indent) {
	if (pt->is_single) {
		fputs(indent, stdout);
		print_token(*pt->single);
	} else {
		print_parsetree(pt->node, indent);
		int indentlen = strlen(indent);
		char* newindent = malloc(indentlen + 2);
		strcpy(newindent, indent);
		strcpy(newindent + indentlen, "  ");
		struct BranchList* b = pt->branches;
		while (!b->last) {
			print_parsetree(b->here, newindent);
			b = b->next;
		}
	}
}

/* Logic */

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
			switch (current.c) {
				case '(': return parse_expr();
				case '\'': return topnode(parse(), "quote");
				case '`': return topnode(parse(), "quasiquote");
				case ',': return topnode(parse(), "unquote");
			}
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

// Execution

void assoc_bind(struct Assoc* a, char* k, struct Value* v) {
	while (a->next != NULL) a = a->next;
	a->key = k;
	a->val = v;
	struct Assoc* next = malloc(sizeof(struct Assoc));
	next->next = NULL;
	a->next = next;
}

struct Value* assoc_get(struct Assoc* a, char* k) {
	while (a->next != NULL)
		if (a->key == k)
			return a->value;
	return NULL;
}

void bind(struct Env* e, char* k, struct Value* v) {
	assoc_bind(&e->vars, k, v);
}

struct Value* get(struct Env* e, char* k) {
	struct Value* ptr;
	do
		if ((ptr = assoc_get(e->vars, k)) != NULL)
			return ptr;
	while ((e = e->parent) != NULL);  // Check logic
}

// Main

int main() {
	char* s = "(define test `(+ 1 ,2 \"asdf\"))";
	puts(s);
	lex(s);
	// update_token();
	// while (current.type != END) { print_token(current); update_token(); }
	// return 0;
	struct ParseTree* pt = parse();
	puts("Parsed");
	print_parsetree(pt, "");
	return 0;
}
