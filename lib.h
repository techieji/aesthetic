#pragma once
#include <stdbool.h>
#include "aestheti.h"

void print_token(struct Value*);
void print_tree(struct Value*, int);
void print_value(struct Value*);

struct Value* construct(enum Type, ...);
bool equal_values(struct Value*, struct Value*);
char* extract_string(char*, int);
bool is_extended_alpha(char);

struct Value* reverse(struct Value*);
struct Value* add(struct Value*);
struct Value* get_stdlib(void);
