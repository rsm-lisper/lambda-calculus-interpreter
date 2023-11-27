
# Lambda Calculus Interpreter

Lambda Calculus Interpreters implemented in couple different languages. Just for fun.

Project in its early stage.

# Valid Lambda Calculus Expression

Expression can be a **name** to identify an abstraction point, a **function** to introduce an abstraction or a **function application** to specialize an abstraction.

<expression> ::= <name> | <function> | <application>

## Name

Any sequence of non-special characters. Special characters are: ` `, `(`, `)`, `[`, `]`, `;`. For example:

```
10 some 7-and-not-5 :other: 0.0.111 -->
```

## Function

Lambda function is an abstraction over a lambda expression and has the form:

<function> ::= (lambda (<name>) <body>)

where:

<body> ::= <expression>

For example:
```scheme
(lambda (x) x)

(lambda (first) (lambda (second) first))

(lambda (func) (lambda (arg) (func arg)))
```

## Application

A function application has the form:

<application> ::= (<function-expression> <argument-expression>)

where:

<function-expression> ::= <expression>
<argument-expression ::= <expression>

For example:
```scheme
((lambda (x) x) 100)

(((lambda (f) (lambda (n) f)) <first>) <next>)
```

## Syntax extensions

- `[` and `]` can be used interchangeably with `(` and `)`
- any text between `;` and the end of line is ignored

