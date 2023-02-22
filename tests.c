#include "testr.h"

TESTR() {
    struct ValueEnv* e = get_stdlib();
    // Language features
    TEST(run_string("1", e)->type == NUM);
    TEST(run_string("\"Testing\"", e)->type == STR);
    TEST(run_string("+", e)->type == FN);   // Lookup
    run_string("(define a 5)", e);
    TEST(value_equal(run_string("a", e), number(5.0)));
    // Stdlib
    TEST(value_equal(run_string("(+ 1 1)", e), number(2.0)));
    TEST(run_string("(lambda (x) x)", e)->type == FN);
    TEST(value_equal(run_string("((lambda (x) x) 5)", e), number(5.0)));
    TEST(value_equal(run_string("((lambda (x y) (+ x y)) 1 2)", e), number(3.0)));
    // Internals
    ;
}