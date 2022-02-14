#include "a.h"

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

struct Lexed nop(void) {
	struct Lexed l;
	l.type = NOP;
	return l;
}

struct ParseTree* single(struct Lexed l) {
	struct ParseTree* pt = malloc(sizeof(struct ParseTree));
	*pt = (struct ParseTree){ 1, &l, NULL, NULL };
	return pt;
}
