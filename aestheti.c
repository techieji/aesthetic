#include "aestheti.h"
#include "stdlib.h"

bool value_equal(struct Value* v1, struct Value* v2) {
  return cfn_equal(value_list(2, v1, v2))->b;
}

struct Value* idx(struct ValueList* vl, int n) {
  return cfn_idx(value_list(2, list_to_value(vl), number((float)n)));   // Might hit performance
}

struct ValueList* append(struct ValueList* vl, struct Value* v) {
  return cfn_append(value_list(2, list_to_value(vl), v))->l;
}

MAKER(number, float, NUM, n)
MAKER(symbol, char*, SYM, s)
MAKER(string, char*, STR, s)
MAKER(boolean, bool, BOOL, b)
MAKER(nil, struct ValueList*, NIL, l)    // fix
MAKER(list_to_value, struct ValueList*, LIST, l)

struct ValueList* vvalue_list(int n, va_list a) {
  if (n == 0) return NULL;
  struct ValueList* l = malloc(sizeof(struct ValueList*)), *p = l;
  *l = (struct ValueList){ va_arg(a, struct Value*), NULL };
  for (int i = 1; i < n; i++) {
    p->next = malloc(sizeof(struct ValueList));
    p = p->next;
    p->here = va_arg(a, struct Value*);
  }
  p->next = NULL;
  return l;
}

struct ValueList* value_list(int n, ...) {
  va_list args;
  va_start(args, n);
  return vvalue_list(n, args);
}

int find(struct ValueList* vl, struct Value* v) {
  for (int i = 0; vl != NULL; vl = vl->next, i++)
    if (value_equal(vl->here, v))
      return i;
  return -1;
}

struct Value* dict_get(struct ValueDict* vd, struct Value* v) {
  int i = find(vd->keys, v);
  if (i < 0)
    return NULL;
  return idx(vd->values, i);
}

struct ValueDict* dict_set(struct ValueDict* vd, struct Value* k, struct Value* v) {
  if (vd == NULL) {
    vd = malloc(sizeof(struct ValueDict));
    vd->keys = value_list(1, k);
    vd->values = value_list(1, v);
  } else {
    append(vd->keys, k);
    append(vd->values, v);
  }
  return vd;
}

struct Value* env_get(struct ValueEnv* ve, struct Value* v) {
  if (ve == NULL) {
    print_value(*v, false, "", stderr);
    fprintf(stderr, " is not defined\n");
    return NULL;
  }
  struct Value* r = dict_get(ve->d, v);
  if (r == NULL) return env_get(ve->upper, v);    // Can probably make iterative
  else return r;
}

struct ValueEnv* env_set(struct ValueEnv* ve, struct Value* k, struct Value* v) {
  ve->d = dict_set(ve->d, k, v);     // Support redefining variables in higher scopes
  return ve;
}

struct ValueEnv* child(struct ValueEnv* ve) {
  struct ValueEnv* e = malloc(sizeof(struct ValueEnv));
  e->d = NULL;
  e->upper = ve;
  return e;
}

// Lexing

struct Lexer* lex(char* s) {
  struct Lexer* l = malloc(sizeof(struct Lexer));
  *l = (struct Lexer){ s };
  return l;
}

struct Lexer* pushback(struct Lexer* l, struct Value* tok) {
  l->backlog = tok;
  return l;
}

// FIXME: memory leak in string and symbol malloc
struct Value* get_token(struct Lexer* l) {
  if (l->backlog != NULL) {
    struct Value* v = l->backlog;
    l->backlog = NULL;
    return v;
  }
  int n;
  sscanf(l->s, " %n", &n);
  l->s += n;
  n = 0;
  struct Value* v = malloc(sizeof(struct Value));
  if (*l->s == '\0') v->type = NIL;
  else if (strchr("()'`,", *l->s)) { v->type = CHAR; v->c = *(l->s++); }
  else if (sscanf(l->s, "%f%n", &v->n, &n)) v->type = NUM;
  else if (sscanf(l->s, "\"%[^\"]\"%n", (v->s = malloc(STR_SIZE)), &n)) v->type = STR;
  else if (sscanf(l->s, "%[^()'`, \t\n]%n", (v->s = malloc(SYM_SIZE)), &n)) v->type = SYM;
  else fprintf(stderr, "Syntax Error: Could not parse at\n  %s\n  ^\n", l->s);
  l->s += n;
  return v;
}

struct Value* parse(struct Lexer* l) {
  struct Value* v = get_token(l);
  switch  (v->type) {
    case SYM: case NUM: case STR: return v;
    case CHAR:
      switch (v->c) {
        case '(': return parse_expr(l);
        case '\'': return list_to_value(value_list(2, string("quote"), parse(l)));
        case '`': return list_to_value(value_list(2, string("quasiquote"), parse(l)));
        case ',': return list_to_value(value_list(2, string("unquote"), parse(l)));
        default: fprintf(stderr, "Unrecognized character: %c\n", v->c);
                 // ^ Shouldn't ever reach here
      }
    default: fprintf(stderr, "Unrecognized token type: %i\n", v->type);
  }
}

struct Value* parse_expr(struct Lexer* l) {
  struct Value* tok;
  struct ValueList* r = NULL;
  while (!((tok = get_token(l))->type == CHAR && tok->c == ')')) {
    r = append(r, parse(pushback(l, tok)));
  }
  return list_to_value(r);
}

struct Value* call(struct Function* fn, struct ValueList* args, struct ValueEnv* ve) {
  struct ValueEnv* e = child(ve);
  for (struct ValueList* arg = fn->argnames; arg != NULL; args = args->next, arg = arg->next)
    env_set(e, arg->here, args->here);
  return run(fn->code, e);
}

struct Value* run(struct Value* tree, struct ValueEnv* e) {
  if (tree->type == SYM) return env_get(e, tree);
  if (tree->type != LIST) return tree;
  struct Value* fn = run(tree->l->here, e);
  struct ValueList* l = NULL;
  // Check type of fn
  switch (fn->fn->type) {
    case NORMAL: case CFN:
      for (struct ValueList* p = tree->l->next; p != NULL; p = p->next)
        l = append(l, run(p->here, e));
      break;
    case MACRO: case CMAC:
      l = tree->l->next;
  }
  switch (fn->fn->type) {
    case NORMAL: case MACRO:
      return call(fn->fn, l, e);
    case CFN:
      return fn->fn->cfn(l);
    case CMAC:
      return fn->fn->mfn(l, e);
  }
}

void print_value(struct Value v, bool quoted, char* end, FILE* fout) {
  switch (v.type) {
    case NUM: fprintf(fout, "%f%s", v.n, end); break;
    case STR: fprintf(fout, "%s%s", v.s, end); break;
    case SYM:
      if (!quoted) putc('\'', fout);
      fprintf(fout, "%s%s", v.s, end); break;
    case CHAR: fprintf(fout, "%c%s", v.c, end); break;
    case BOOL: fprintf(fout, v.b ? "true%s" : "false%s", end); break;
    case LIST:
      if (!quoted) putc('\'', fout);
      putc('(', stdout);
      for (struct ValueList* vl = v.l; vl != NULL; vl = vl->next)
        print_value(*vl->here, true, " ", fout);
      fprintf(fout, "\b)%s", end);
      break;
    case DICT:
      fprintf(fout, "DICTIONARY%s", end);     // Finish
    case FN:
      fprintf(fout, "FUNCTION%s", end);   // Finish
    case NIL:
      break;
    default:
      fprintf(fout, "UNKNOWN%s", end);   // Finish
  }
}

struct Value* run_string(char* s, struct ValueEnv* ve) {
  struct Lexer* l = lex(s);
  struct Value* pt = parse(l);
  return run(pt, ve);
}

void repl(struct ValueEnv* ve) {
  char* buf = malloc(sizeof(char) * 200);
  puts("Repl started");
  while (true) {
    if (fgets(buf, 200, stdin) == NULL) break;
    printf("=> ");
    print_value(*run_string(buf, ve), false, "\n", stdout);
  }
}

#include "tests.c"

int main() {
  if (1) RUN_TESTS();
  else {
    struct ValueEnv* e = get_stdlib();
    repl(e);
    run_string("(+ 1 1)", e);
  }
  return 0;
}
