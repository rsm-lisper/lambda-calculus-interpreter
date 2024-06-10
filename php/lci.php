#!/usr/bin/php
<?php

/*****************************************/
/***   L O W   L E V E L   T O O L S   ***/
/*****************************************/


/***   C H A R   /   S T R I N G   ***/

$last_c = NULL;
function ungetchar ($c)
{
    global $last_c;
    $last_c = $c;
}
function getchar ()
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

function eat_wspace_comment ()
{
    while (($c = getchar()) !== FALSE && strchr("; \t\r\n", $c) !== FALSE) {
        if ($c === ';') {
            do $c = getchar();
            while ($c !== FALSE && $c !== "\n" && $c !== "\r");
            ungetchar($c);
        }
    }
    ungetchar($c);
}


/***   N O D E   ***/

function &make_node (&$first, &$next)
{
    $r = array('first' => $first, 'next' => $next);
    return $r;
}

function &is_node (&$node)
{
    if (is_array($node) &&
        array_key_exists('first', $node) &&
        array_key_exists('next', $node))
        return $node;
    return FALSE;
}
function &nfirst (&$node)
{
    assert(is_node($node));
    return $node['first'];
}
function &nnext (&$node)
{
    assert(is_node($node));
    return $node['next'];
}
function &set_nfirst (&$node, &$value)
{
    assert(is_node($node));
    $node['first'] = &$value;
    return $node;
}
function &set_nnext (&$node, &$value)
{
    assert(is_node($node));
    $node['next'] = &$value;
    return $node;
}


/*********************************************/
/***   D A T A   /   D A T A   T Y P E S   ***/
/*********************************************/

define ("D_SYMBOL", 0);
define ("D_PAIR", 1);
define ("D_CLOSURE", 2);

/***   S Y M B O L   ***/

function &is_symbol (&$d)
{
    if (is_array($d) &&
        array_key_exists('type', $d) &&
        array_key_exists('symbol', $d) &&
        $d['type'] === D_SYMBOL) {
        return $d;
    }
    else {
        $f = FALSE;
        return $f;
    }
}

function &svalue (&$d)
{
    assert(is_symbol($d));
    return $d['symbol'];
}

function sequal (&$sa, &$sb)
{
    assert(is_symbol($sa) && is_symbol($sb));
    return svalue($sa) === svalue($sb);
}

function &make_symbol ($str)
{
    $r = array('type' => D_SYMBOL, 'symbol' => $str);
    return $r;
}

function &print_symbol (&$s, $stream)
{
    assert(is_symbol($s));
    fwrite($stream, $s['symbol']);
    return $s;
}

function &read_symbol ()
{
    $s = "";
    while (($c = getchar()) !== FALSE &&
           strchr("()[]; \t\r\n", $c) === FALSE) {
        $s .= $c;
    }
    ungetchar($c);
    return make_symbol($s);
}


/***   P A I R   ***/

function &is_pair (&$d)
{
    if (is_array($d) && array_key_exists('type', $d) &&
        array_key_exists('pair', $d) && $d['type'] === D_PAIR) {
        return $d;
    }
    else {
        $f = FALSE;
        return $f;
    }
}

function &make_pair (&$first, &$next)
{
    $r = array('type' => D_PAIR, 'pair' => make_node($first, $next));
    return $r;
}

function &pfirst (&$p)
{
    assert(is_pair($p));
    return nfirst($p['pair']);
}

function &pnext (&$p)
{
    assert(is_pair($p));
    return nnext($p['pair']);
}

function &set_pfirst (&$p, &$val)
{
    assert(is_pair($p));
    set_nfirst($p['pair'], $val);
    return $p;
}

function &set_pnext (&$p, &$val)
{
    assert(is_pair($p));
    set_nnext($p['pair'], $val);
    return $p;
}

function &print_pair_tree (&$p, $stream)
{
    $p0 = &$p;
    $i = 0;

    if (!is_pair($p))
        return $p;

    fwrite($stream, '(');
    while (is_pair($p)) {
        if ($i) fwrite($stream, ' ');
        else $i = 1;
        $curr = &pfirst($p);
        if (is_symbol($curr))
            print_symbol($curr, $stream);
        else if (is_closure($curr))
            print_closure($curr, $stream);
        else if (is_pair($curr))
            print_pair_tree($curr, $stream);
        else {
            assert($curr === NULL);
            fwrite($stream, "()");
        }
        $p = &pnext($p);
    }
    if (is_symbol($p)) {
        if ($i) fwrite($stream, ' ');
        else $i = 1;

        fwrite($stream, ". ");
        print_symbol($p, $stream);
    }
    fwrite($stream, ')');
  
    return $p0;
}

function &read_pair_tree ()
{
    $pread = NULL;
  
    eat_wspace_comment();
    while (($c = getchar()) !== ')' && $c !== ']') {
        if ($c === FALSE) {
            fwrite(STDERR, "ERROR! read_pair_tree(): missing closing paren\n");
            return $r = -1;
        }
        if ($c === '(' || $c === '[') {
            $sub_p = &read_pair_tree();
            if ($sub_p !== NULL && !is_pair($sub_p))
                return $sub_p;
            $np = &make_pair($sub_p, $NULL);
            if ($pread === NULL) {
                $pread = &$np;
                $p = &$np;
            }
            else {
                set_pnext($p, $np);
                $p = &pnext($p);
            }
        }
        else {
            ungetchar($c);
            $np = &make_pair(read_symbol(), $NULL);
            if ($pread === NULL) {
                $pread = &$np;
                $p = &$pread;
            }
            else {
                set_pnext($p, $np);
                $p = &pnext($p);
            }
        }
        eat_wspace_comment();
    }
    return $pread;
}


/***   C L O S U R E   ***/

function &is_closure (&$d)
{
    if (is_array($d) && array_key_exists('type', $d) &&
        array_key_exists('closure', $d) && $d['type'] === D_CLOSURE) {
        return $d;
    }
    else {
        $f = FALSE;
        return $f;
    }
}

function &make_closure (&$lambda, &$env)
{
    $r = array('type' => D_CLOSURE, 'closure' => make_node($lambda, $env));
    return $r;
}

function &closure_lambda (&$cl)
{
    assert(is_closure($cl));
    return nfirst($cl['closure']);
}

function &closure_arg_name (&$cl)
{
    assert(is_closure($cl));
    return pfirst(pfirst(pnext(closure_lambda($cl))));
}

function &closure_body (&$cl)
{
    assert(is_closure($cl));
    return pfirst(pnext(pnext(closure_lambda($cl))));
}

function &closure_env (&$cl)
{
    assert(is_closure($cl));
    return nnext($cl['closure']);
}

function &print_closure (&$cl, $stream)
{
    assert(is_closure($cl));
    fwrite($stream, "#<closure (");
    print_symbol(closure_arg_name($cl), $stream);
    fwrite($stream, ") ");
    print_data(closure_body($cl), $stream);
    fwrite($stream, " | ");
    print_data(closure_env($cl), $stream);
    fwrite($stream, ">");
    return $cl;
}


/***   D A T A   T O O L S   ***/

function &print_data (&$d, $stream)
{
    if (is_symbol($d))
        print_symbol($d, $stream);
    else if (is_closure($d))
        print_closure($d, $stream);
    else if (is_pair($d))
        print_pair_tree($d, $stream);
    else if ($d === NULL)
        fwrite($stream, "()");
    return $d;
}

function list_length (&$list)
{
    $len = 0;
    assert(is_pair($list));
    while ($list != NULL) {
        $len++;
        $list = &pnext($list);
    }
    return $len;
}

function &assoc_add (&$assoc_list, &$key, &$value)
{
    return make_pair(make_pair($key, $value), $assoc_list);
}

function &assoc_lookup (&$assoc_list, &$key)
{
    while ($assoc_list) {
        if (sequal(pfirst(pfirst($assoc_list)), $key))
            return pfirst($assoc_list);
        $assoc_list = &pnext($assoc_list);
    }
    $f = FALSE;
    return $f;
}


/****************************************************************/
/***   L A M B D A   C A L C U L U S   I N T E R P R E T E R  ***/
/****************************************************************/

function &read_expr ()
{
    eat_wspace_comment();
    $c = getchar();
    if ($c === FALSE)
        return NULL;
    if ($c === ')' || $c === ']') {
        fwrite(STDERR, "ERROR! read_expr(): too many parens\n");
        return $NULL;
    }
    if ($c === '(' || $c === '[') {
        return read_pair_tree();
    }
    else {
        ungetchar($c);
        return read_symbol();
    }
}


function &print_expr (&$expr)
{
    if ($expr !== NULL)
        print_data($expr, STDOUT);
    else
        print ("null");
    printf("\n");
    return $expr;
}


function &is_lambda (&$expr)
{
    if (is_pair($expr) && list_length($expr) == 3 &&
        is_symbol(pfirst($expr)) &&
        sequal(pfirst($expr), make_symbol('lambda'))) {
        return $expr;
    }
    else {
        $f = FALSE;
        return $f;
    }
}


function &eval_expr (&$expr, &$env)
{
    if ($expr === NULL)
        return NULL;
    if (is_symbol($expr)) {
        $kval = &assoc_lookup($env, $expr);
        if ($kval !== FALSE)
            $r = &pnext($kval);
        else
            $r = &$expr;
        return $r;
    }
    if (is_lambda($expr)) {
        return make_closure($expr, $env);
    }
    if (is_pair($expr) && list_length($expr) == 2) {
        $cl = &eval_expr(pfirst($expr), $env);
        $arg = &eval_expr(pfirst(pnext($expr)), $env);
        if (!is_closure($cl)) {
            fwrite(STDERR, "ERROR! eval_expr(): expression is not a closure: ");
            print_data($cl, STDERR);
            fwrite(STDERR, "\n\n");
            return NULL;
        }
        $new_env = &assoc_add(closure_env($cl), closure_arg_name($cl), $arg);
        $new_expr = &eval_expr(closure_body($cl), $new_env);
        return $new_expr;
    }

    fwrite(STDERR, "ERROR! eval_expr(): unrecognised expression: ");
    print_data($expr, STDERR);
    fwrite(STDERR, "\n\n");
    return NULL;
}


function main ()
{
    print_expr(eval_expr(read_expr(), $NULL));
    return 0;
}

main();
