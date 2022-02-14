#include "a.h"

void print_token(struct Lexed tok) {
	// printf("%i\n", tok.type);
	// return;
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
