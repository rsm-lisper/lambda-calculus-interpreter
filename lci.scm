#!/usr/bin/guile \
-e main -s
!#
;; -*- mode: scheme -*-

(define (read-exp)
  (read (current-input-port)))

(define (eval-exp s-exp env)
  (cond [(symbol? s-exp) (env s-exp)]
	[(and (list? s-exp) (eq? 'lambda (car s-exp)))
	 (lambda (arg)
	   (eval-exp (caddr s-exp)
		     (lambda (y)
		       (if (eq? y (caadr s-exp))
			   arg
			   (env y)))))]
	[(and (list? s-exp) (= 2 (length s-exp)))
	 ((eval-exp (car s-exp) env) (eval-exp (cadr s-exp) env))]
	[else '*eval-error*]))

(define (print-exp x)
  (display (if (procedure? x) "#<lambda>"
               x))
  (newline))

(define (repl s-exp)
  (cond [(eof-object? s-exp) '()]
        [else
         (print-exp
          (eval-exp s-exp
                    (lambda (e) e)))
         (repl (read-exp))]))

(define (main args)
  (repl (read-exp)))
