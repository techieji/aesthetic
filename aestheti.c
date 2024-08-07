#include "aestheti.h"

/* * * * * *
 * HELPERS *
 * * * * * */

void print_value(struct Value* tree) {
    switch (tree->type) {
        case SYM: case STR: case ERROR: printf("%s", tree->s); return;
        case FLOAT: printf("%lf", tree->f); return;
        case INT: printf("%d", tree->i); return;
        case BOOL: printf("%s", tree->b ? "true" : "false"); return;
        case NIL: printf("nil"); return;
        case PAIR:
            printf("(");
            print_value(tree->car);
            while ((tree = tree->cdr)->type == PAIR) { printf(" "); print_value(tree->car); }
            if (tree->type != NIL) { printf(" . "); print_value(tree); }
            printf(")");
            return;
        case CFN:
            printf("[compiled function at %p]", tree->cfn);
            return;
        case CMACRO:
            printf("[compiled macro at %p]", tree->cfn);
            return;
        case FN:
            printf("[user-defined function]");
            return;
        default: printf("[undefined]");
    }
}

struct Value* all_values = NULL;

void register_value(struct Value* v) {
    if (all_values == NULL) {
        all_values = malloc(sizeof(struct Value));
        all_values->type = NIL;
    }
    struct Value* temp = malloc(sizeof(struct Value));
    temp->type = PAIR;
    temp->car = v;
    temp->cdr = all_values;
    all_values = temp;
}

struct Value* construct(enum Type type, ...) {
    struct Value* v = malloc(sizeof(struct Value));
    v->type = type;
    register_value(v);
    va_list l;
    va_start(l, type);
    switch (type) {
        case INT:
            v->i = va_arg(l, int);
            break;
        case FLOAT:
            v->f = va_arg(l, double);
            break;
        case BOOL:
            v->b = va_arg(l, int);
            break;
        case SYM:
        case STR:
        case ERROR:
            v->s = va_arg(l, char*);
            break;
        case FN:
            v->cbr = va_arg(l, struct Value*);
        case PAIR:
            if (type != FN) v->cbr = construct(NIL);
            v->car = va_arg(l, struct Value*);
            v->cdr = va_arg(l, struct Value*);
            break;
        case CFN:
            v->cfn = va_arg(l, struct Value*(*)(struct Value* list));
            break;
        case CMACRO:
            v->cmac = va_arg(l, struct Value*(*)(struct Value* list, struct Value** env));
            break;
        default:
    }
    va_end(l);
    return v;
}

void destruct(struct Value** kv) {
    struct Value* v = *kv;
    if (v == NULL) return;
    if (v->type == SYM || v->type == STR || v->type == ERROR) free((char*)v->s);
    free(v);
    *kv = NULL;
}

struct Value* construct_triple(struct Value* car, struct Value* cbr, struct Value* cdr) {
    struct Value* pair = construct(PAIR, car, cdr);
    pair->cbr = cbr;
    return pair;
}

struct Value* construct_error(char* fmt, ...) {
    char* buf = malloc(100 * sizeof(char));
    va_list l;
    va_start(l, fmt);
    vsprintf(buf, fmt, l);
    va_end(l);
    return construct(ERROR, buf);
}

void chain(int len, ...) {
    if (len == 0) return;
    va_list l;
    va_start(l, len);
    struct Value* prev = va_arg(l, struct Value*);
    for (int i = 1; i < len; i++)
        prev = prev->cdr = va_arg(l, struct Value*);
    prev->cdr = construct(NIL);
    va_end(l);
}

char* extract_string(char* s, int i) {
    char* str = malloc((i + 1) * sizeof(char));
    strncpy(str, s, i);
    str[i] = '\0';
    return str;
}

struct Value* reverse(struct Value* l) {
    struct Value* newl = construct(NIL);
    while (l->type != NIL) {
        newl = construct(PAIR, l->car, newl);
        l = l->cdr;
    }
    return newl;
}

bool is_extended_alpha(char c) { return isalpha(c) || isdigit(c) || (strchr("+-.*/<=>!?:$%_&~^", c) != NULL); }

/* * * * * *
 *  LEXER  *
 * * * * * */

struct Value* parse_string(char** s) {
    (*s)++;
    int i;
    for (i = 0; (*s)[i] != '"'; i++);   // TODO handle escapes and EOF
    struct Value* token = construct(STR, extract_string(*s, i));
    *s += i + 1;
    return token;
}

struct Value* parse_number(char** s) {
    struct Value* token;
    int i = 0;
    if ((**s) == '-') i++;
    while (isdigit((*s)[++i]));
    if ((*s)[i] != '.')
        token = construct(INT, atoi(extract_string(*s, i)));
    else {
        while (isdigit((*s)[++i]));
        token = construct(FLOAT, atof(extract_string(*s, i)));
    }
    *s += i;
    return token;
}

struct Value* parse_symbol(char** s) {
    int i = 0;
    while (is_extended_alpha((*s)[++i]));
    struct Value* token = construct(SYM, extract_string(*s, i));
    *s += i;
    return token;
}

struct Value* parse_char(char** s) {
    (*s)++;
    switch (*(*s - 1)) {
        case '(': return construct(OPEN);
        case ')': return construct(CLOSE);
        case '\'': return construct(QUOTE);
        case '\0': return construct(END);
    }
    (*s)--;
    return NULL;
}

static struct Value* pushback = NULL;

struct Value* next_token(char** s) {
    while (isspace(**s)) (*s)++;
    struct Value* res;
    if (pushback != NULL) {
        res = pushback;
        pushback = NULL;
        return res;
    }
    if ((res = parse_char(s)) != NULL) return res;
    if (**s == '"')  return parse_string(s);
    if (isdigit(**s) || **s == '-') return parse_number(s);
    return parse_symbol(s);
}

struct Value* peek_token(char** s) {
    pushback = next_token(s);
    return pushback;
}

/* * * * * *
 * PARSER  *
 * * * * * */

jmp_buf parse_error;

struct Value* parse(char** s) {
    struct Value* tok = next_token(s);
    switch (tok->type) {
        case CLOSE: return tok;
        case OPEN:
            struct Value* l = construct(NIL);
            while ((tok = parse(s))->type != CLOSE) l = construct(PAIR, tok, l);
            return reverse(l);
        case QUOTE:
            return construct(PAIR, construct(SYM, strdup("quote")), construct(PAIR, parse(s), construct(NIL)));
        case END:
            longjmp(parse_error, 1);
            return NULL;
        default:
            return tok;
    }
}

/* * * * * *
 *  EVAL   *
 * * * * * */
// Environments are lists of pairs: '((k1 . v1) (k2 . v2) ...)
// Functions are pairs: '((arg1 arg2 arg3 ...) . (<code>))        TODO add support for closures

struct Value* fix_null(struct Value* ptr) { return ptr != NULL ? ptr : construct_error("INTERNAL FUNCTION ERROR."); }

struct Value* eval(struct Value* t, struct Value** env) {
    if (t == NULL) return construct(NIL);
    switch (t->type) {
        case FLOAT: case INT: case STR: case ERROR:
            return t;
        case SYM:
            for (struct Value* v = *env; v->type != NIL; v = v->cdr)
                //if (equal_values(v->car, t))
                if (v->car->type == t->type && strcmp(v->car->s, t->s) == 0)
                    return v->cbr;
            return construct_error("SYMBOL %s IS NOT BOUND.", t->s);
        case PAIR:
            struct Value* fn = eval(t->car, env);
            if (fn->type == CMACRO)
                return fix_null(fn->cmac(t->cdr, env));

            struct Value* l = construct(PAIR, construct(END), construct(NIL));    // Dummy starting element
            struct Value* tmp;
            for (struct Value* v = t->cdr; v->type != NIL; v = v->cdr) {
                if ((tmp = eval(v->car, env))->type == ERROR) return tmp;
                l = construct(PAIR, tmp, l);
            }
            l = reverse(l);
            switch (fn->type) {
                case CFN:
                    return fix_null(fn->cfn(l->cdr));
                case FN:
                    struct Value* new_env = fn->cbr;
                    for (struct Value* var = fn->car; var->type != NIL; var = var->cdr)
                        new_env = construct_triple(var->car, (l = l->cdr)->car, new_env);
                    return eval(fn->cdr, &new_env);
                default:
                    return construct_error("NOT A FUNCTION.");
            }
        default:
            return construct_error("INVALID OBJECT. YOU ARE BEING A BIT GOOFY.");
    }
}

/* * * * * * * * * * *
 * GARBAGE COLLECTOR *
 * * * * * * * * * * */

extern struct Value* all_values;

void reset(void) {
    for (struct Value* ptr = all_values; ptr->type != NIL; ptr = ptr->cdr) ptr->mark = false;
}

void mark(struct Value* v) {
    if (v->mark) return;
    v->mark = true;
    if (v->type == PAIR || v->type == FN) {
            mark(v->car);
            mark(v->cbr);
            mark(v->cdr);
    }
}

void sweep(void) {
    struct Value *prev, *cur, *curelem;
    prev = all_values;       // FIXME bug: first element of list can never be garbage collected
    cur = prev;
    while ((cur = cur->cdr)->type != NIL) {
        if (!(curelem = cur->car)->mark) {
            prev->cdr = cur->cdr;
            //printf("Swept: ");
            //print_value(cur->car);
            //printf("\n");
            destruct(&cur->car);
            cur = prev;
        } else
            prev = cur;
    }
}

void gc(struct Value* env) {
    reset();
    mark(env);
    sweep();
}

/* * * * * * * * * * *
 *        MAIN       *
 * * * * * * * * * * */

struct Value* run_string(char* s, struct Value** env) {
    char** ptr = &s;
    struct Value* res = construct(NIL);
    while (peek_token(ptr)->type != END && res->type != ERROR) {
        struct Value* tree = setjmp(parse_error) == 0 ? parse(ptr) : construct_error("EOF ERROR.");
        res = eval(tree, env);
    }
    next_token(ptr);
    return res;
}

struct Value* load(struct Value* args, struct Value** env) {
    FILE* fp = fopen(args->car->s, "r");
    if (fp == NULL) return construct_error("COULD NOT OPEN FILE.");
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    char* s = malloc(size * sizeof(char));
    rewind(fp);
    fread(s, sizeof(char), size, fp);
    return run_string(s, env);
}

struct Value* load_c(struct Value* args) {
    char* s;
    void* so = dlopen(args->car->s, RTLD_NOW | RTLD_NODELETE);
    if ((s = dlerror())) return construct_error(s);
    struct Value* ret = construct(args->cdr->car->i, dlsym(so, args->cdr->cdr->car->s));
    if ((s = dlerror())) return construct_error(s);
    dlclose(so);
    if ((s = dlerror())) return construct_error(s);
    return ret;
}

#define DECL(name, type, cfn) construct_triple(construct(SYM, name), construct(type, cfn), NULL)
#define GVAR(name, ...) construct_triple(construct(SYM, name), construct(__VA_ARGS__), NULL)

struct Value* get_stdlib(void) {
    struct Value* env;
    env = construct_triple(construct(SYM, "env"), NULL, NULL);
    env->cbr = env;
    chain(3,           // UPDATE THIS WHEN ADDING NEW DECLARATIONS
        env,
        DECL("load", CMACRO, load),
        DECL("load-c", CFN, load_c)
    );
    return env;
}

__attribute__((force_align_arg_pointer))
int run() {
    char* s = malloc(100 * sizeof(char));
    struct Value* env = get_stdlib();
    load(construct(PAIR, construct(STR, strdup("bootstrap.scm")), construct(NIL)), &env);
    gc(env);
    while (true) {
        printf("> ");
        fgets(s, 100, stdin);
        if (feof(stdin)) break;
        print_value(run_string(s, &env));
        printf("\n");
        gc(env);         // TODO doesn't work with quotation (symbols?)
    }
    return 0;
}

#ifdef DL_LOADER
const char dl_loader[] __attribute__((section(".interp"))) = DL_LOADER;
#else
int main() { return run(); }
#endif
