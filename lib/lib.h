#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <dlfcn.h>
#include "../aestheti.h"

// Logging
// void print_token(struct Value*);
// void print_tree(struct Value*, int);
void print_value(struct Value*);

// Helpers
struct Value* construct(enum Type, ...);
void destruct(struct Value*);
struct Value* construct_triple(struct Value*, struct Value*, struct Value*);
struct Value* construct_error(char*, ...);
//bool equal_values(struct Value*, struct Value*);
char* extract_string(char*, int);
bool is_extended_alpha(char);

struct Value* reverse(struct Value*);

// stdlib
struct Value* get_stdlib(void);
