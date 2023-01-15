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

// Execution helpers

// Environment managing

void assoc_bind(struct Assoc* a, char* k, struct Value* v) {
	while (a->next != NULL) a = a->next;
	a->key = k;
	a->val = v;
	struct Assoc* next = malloc(sizeof(struct Assoc));
	next->next = NULL;
	a->next = next;
}

struct Value* assoc_get(struct Assoc* a, char* k) {
  for (; a != NULL; a = a->next)
		if (strcmp(a->key, k) == 0)
			return a->val;
    else
      printf("%s != %s\n", a->key, k);
	return NULL;
}

void bind(struct Env* e, char* k, struct Value* v) {
	assoc_bind(e->vars, k, v);
}

struct Value* get(struct Env* e, char* k) {
	struct Value* ptr;
	do
		if ((ptr = assoc_get(e->vars, k)) != NULL)
			return ptr;
	while ((e = e->parent) != NULL);  // Check logic
  fprintf(stderr, "Variable `%s` not found\n", k);
  exit(1);
}

struct Env* child(struct Env* parent) {
	struct Env* child = malloc(sizeof(struct Env));
	struct Assoc* a = malloc(sizeof(struct Assoc));
	a->next = NULL;
  child->vars = a;
  child->parent = parent;
  // *child = { a, parent };
	return child;
}

void print_value(struct Value* v) {
  switch (v->type) {
    case VSTR: printf("%s\n", v->s); return;
    case VNUM: printf("%f\n", v->n); return;
    default: puts("OTHER");
  }
}

// Execution

struct ArgList* _al_append(struct ArgList* al, struct Value* v) {
  al->here = v;
  al->next = malloc(sizeof(struct ArgList));
  return al->next;
}

struct Value* lexed_to_value(struct Lexed* l, struct Env* e) {
  struct Value* v = malloc(sizeof(struct Value));
  switch (l->type) {
    case SYM:
      free(v);
      return get(e, l->s);
    case STR:
      v->s = l->s;
      v->type = VSTR;
      return v;
    case NUM:
      v->n = l->n;
      v->type = VNUM;
      return v;
    default:
      fprintf(stderr, "Lexed value cannot be converted: %i\n", l->type);
      exit(1);
  }
}

struct Value* run(struct Env* e, struct ParseTree* pt) {
  if (pt->is_single) {
    //return lexed_to_value(pt->single, e);
    return lexed_to_value(pt->single, e);
  }
  struct Value* f = run(e, pt->node);
  if (f->type == VMAC) {   // Finish
  } else if (f->type == VFUN) {
    struct ArgList* al = malloc(sizeof(struct ArgList));
    al->here = NULL;
    al->next = NULL;
    struct ArgList* im = al;
    for (struct BranchList* bl = pt->branches; bl->next != NULL; bl = bl->next) {  // TODO: Fix branchlist structure
      im = _al_append(im, run(e, bl->here));
    }
    im->next = NULL;   // Check logic
    if (f->fn->type == PRIMITIVE) {
      return f->fn->cfn(al);
    //} else if (f->fn->type == NORMAL) {
    //  struct Env* ec = child(e);
    //  ;
    } else perror("???");
  } else {
    perror("Uncallable object called");    // Improve error messages
  };
}

// Main

int main() {
	// char* s = "(define test `(+ 1 ,2 \"asdf\"))";
  char* s = "(+ (+ 1 2) 2)";
	puts(s);
	lex(s);
	struct ParseTree* pt = parse();
	puts("Parsed");
	print_parsetree(pt, "");
  struct Env* e = get_base_stdlib();
  print_value(run(e, pt));
	return 0;
}
