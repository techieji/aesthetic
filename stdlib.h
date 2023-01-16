#pragma once
#include "aestheti.h"

// Set the following macros to enable or disable various functionalities
#define CORE 1
#define COREEXT 1
#define ARITH 1

// Utilities for creating functions
struct Value* fn_to_value(struct Value* (*)(struct ValueList*));
struct Value* mac_to_value(struct Value* (*)(struct ValueList*, struct ValueEnv*));

// Core
struct Value* cmac_if(struct ValueList*);
struct Value* cmac_lambda(struct ValueList*);
struct Value* cfn_display(struct ValueList*);

// CoreExt

struct Value* cmac_define(struct ValueList*, struct ValueEnv*);

// Arith
struct Value* cfn_add(struct ValueList*);
struct Value* cfn_sub(struct ValueList*);
