#pragma once
#include <stdbool.h>
#include "aestheti.h"

void print_token(struct Value*);
void print_tree(struct Value*, int);
void print_value(struct Value*);

//struct Value* singleton(enum Type);
//struct Value* pair(struct Value*, struct Value*);
struct Value* reverse(struct Value*);

struct Value* construct(enum Type, ...);
bool equal_values(struct Value*, struct Value*);
char* extract_string(char*, int);
bool is_extended_alpha(char);
