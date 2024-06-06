#!/usr/bin/php
<?php

////    T O O L S (getch/ungetch)
$last_c = NULL;
function ungetc ($c)
{
    global $last_c;
    $last_c = $c;
}
function getc ()
{
    global $last_c;
    if ($last_c !== NULL) {
        $c = $last_c;
        $last_c = NULL;
    }
    else
        $c = fgetc(STDIN);
    return $c;
}

////    R E A D E R    ////
function read_symbol ($symbol)
{
    while (($c = getc()) !== FALSE && strpos("()[]; \t\r\n", $c) === FALSE)
        $symbol .= $c;
    ungetc($c);
    return $symbol;
}

function eat_wspace_comm ()
{
    while (($c = getc()) !== FALSE && strpos("; \t\r\n", $c) !== FALSE)
        if ($c === ';') {
            do $c = getc();
            while ($c !== FALSE && $c !== "\n" && $c !== "\r");
            ungetc($c);
        }
    ungetc($c);
}

function read_list ()
{
    $list = [];
    eat_wspace_comm();
    while (($c = getc()) !== ')' && $c !== ']') {
        if ($c === FALSE)
            trigger_error('read_list error: missing closing paren', E_USER_ERROR);
        $list[] = $c === '(' || $c === '[' ? read_list() : read_symbol($c);
        eat_wspace_comm();
    }
    return $list;
}

function read_expr ()
{
    eat_wspace_comm();
    $c = getc();
    if ($c === ')' || $c === ']')
        trigger_error('read_expr error: too many parens', E_USER_ERROR);
    return
        $c === FALSE ? FALSE :
        ($c === '(' || $c === '[' ? read_list() :
         read_symbol($c));
}

////    E V A L U A T O R    ////
function eval_expr ($expr, $local_env=NULL)
{
    return
        is_string($expr) ? $local_env($expr) :
        (is_array($expr) && count($expr) == 3 && $expr[0] === 'lambda' ?
         function ($arg) use ($expr, $local_env) {
             return eval_expr($expr[2],
                              function ($symbol) use ($arg, $expr, $local_env) {
                                  return $symbol === $expr[1][0] ? $arg : $local_env($symbol);
                              });
         } :
         (is_array($expr) && count($expr) == 2 ?
          eval_expr($expr[0], $local_env)(eval_expr($expr[1], $local_env)) :
          "*eval-error*"));
}

////    P R I N T E R    ////
function format_list ($list)
{
    $str = '';
    foreach ($list as $k => $lvalue)
        $str .= is_array($lvalue) ? format_list($lvalue) : $lvalue.' ';
    return '('.rtrim($str).') ';
}

function print_expr ($data)
{
    print(is_callable($data) ? '#<lambda>' :
          (is_array($data) ? format_list($data) :
           $data));
    print("\n");
}

////    M A I N   L O O P    ////
function repl ()
{
    while (($expr = read_expr()) !== FALSE) {
        print_expr(eval_expr($expr,
                             function ($env) { return $env; }));
    }
}

repl();
