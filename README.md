# Aesthetic Lisp

In this day, software tends to be gargantuan, wasting space and computational power
to simply reach the baseline. This is often justified by invoking various forms of Moore's Law: why should we care, as long as computing resources keep expanding? Ignoring the obvious
counterarguments, I claim that the complexity of such pieces of code hides the beauty of the code itself, especially to those learning the language; if the only examples of real world programs
are utterly illegible due to their size, how can students learn through practice?

To that end, Aesthetic Lisp allows a solution. In under 400 lines of code, a complete Lisp interpreter has been written, with clearly delineated sections containing a separate lexer, parser,
code executor, and garbage collector. All extraneous functionality, including library functions such as arithmetic operators, special forms, and constants have been factored out, instead being
loaded in by a dynamic bootstrapping script written in Aesthetic Lisp; this allows for Aesthetic Lisp to be easily adapted as a DSL, its legibility and compositional nature
allowing any potential user to simply take the portions they need and leave out the rest.

## Features

 - Dynamic C library loading
 - Garbage collector
 - Executable library
 - Tail-call recursion (coming soon, hopefully)
