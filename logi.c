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
