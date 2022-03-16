#include "a.h"

// Dynamically allocate env
struct Env child(struct Env* env) {
	struct Env env2;
	env2.parent = env;
	env2.size = 0;
	return env2;
}

struct Lexed lookup(struct Env* env, char* name) {
	for (int i = 0; i < env.size; i++)
		if (env->vars[i].name == name)
			return env->vars[i].value;
	printf("%s is not defined\n", name);
	return NULL;
}

void set(struct Env* env, char* name, struct Lexed value) {
	env->size[env->size++] = (struct Var){ name, value };
}

_Bool is_special(char* id) {
	return     id == "define"
		|| id == "quote"
		|| id == "quasiquote"
		|| id == "unquote"
		|| id == "if"
		|| id == "else"
		|| id == "cond";
}

// Not safe
struct Lexed* get_at(struct ArgList* al, int i) {
	for (int x = 0; x < i; i++) al = al->next;
	return al->here;
}

struct ParseTree* run_special(char* id, struct Env* env, struct ArgList al) {
	switch (id) {
		case "define": // working here...
			set(env, get_at(al, 0).s, get_at(al, 1).s);
			break;
	}
}
