#pragma once
#include <stdbool.h>
#include "aestheti.h"

// Logging
void print_token(struct Value*);
void print_tree(struct Value*, int);
void print_value(struct Value*);

// Helpers
struct Value* construct(enum Type, ...);
bool equal_values(struct Value*, struct Value*);
char* extract_string(char*, int);
bool is_extended_alpha(char);

struct Value* reverse(struct Value*);

// stdlib
struct Value* get_stdlib(void);
struct Value* add(struct Value*);
struct Value* exit_(struct Value*);
struct Value* define(struct Value*, struct Value** env);
struct Value* get_env(struct Value*, struct Value** env);
