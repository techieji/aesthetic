
((load-c "lib/libstd.so" 12 "define") define (load-c "lib/libstd.so" 12 "define"))
(define CFN 11)
(define CMACRO 12)
(define (get-lib-fn s) (load-c "lib/libstd.so" CFN s))
(define (get-lib-mac s) (load-c "lib/libstd.so" CMACRO s))

(define + (get-lib-fn "add"))
(define exit (get-lib-fn "exit_"))
(define car (get-lib-fn "car"))
(define cbr (get-lib-fn "cbr"))
(define cdr (get-lib-fn "cdr"))
(define display (get-lib-fn "display"))
(define = (get-lib-fn "equal"))

(define lambda (get-lib-mac "lambda"))
(define get-env (get-lib-mac "get_env"))
(define quote (get-lib-mac "quote"))