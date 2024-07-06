#!/usr/bin/php
<?php

/*****************************************/
/***   L O W   L E V E L   T O O L S   ***/
/*****************************************/

$NULL = NULL;
$FALSE = FALSE;

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


/*********************************************/
/***   D A T A   /   D A T A   T Y P E S   ***/
/*********************************************/

define ("D_SYMBOL", 0);
define ("D_PAIR", 1);
define ("D_CLOSURE", 2);

/***   S Y M B O L   ***/

function &is_symbol (&$data)
{
    if (is_array($data) &&
        array_key_exists('type', $data) &&
        array_key_exists('symbol', $data) &&
        $data['type'] === D_SYMBOL)

        return $data;
    return $FALSE;
}

function &svalue (&$symbol)
{
    assert(is_symbol($symbol));
    return $symbol['symbol'];
}

function sequal (&$sa, &$sb)
{
    assert(is_symbol($sa));
    assert(is_symbol($sb));
    return svalue($sa) === svalue($sb);
}

function &make_symbol ($str)
{
    $r = array(
        'type' => D_SYMBOL,
        'symbol' => $str
    );
    return $r;
}

function &print_symbol (&$symbol, $stream)
{
    assert(is_symbol($symbol));
    fwrite($stream, $symbol['symbol']);
    return $symbol;
}

function symbol2str (&$symbol)
{
    assert(is_symbol($symbol));
    return $symbol['symbol'];
}

function &read_symbol ()
{
    $str = "";
    while (($c = getchar()) !== FALSE && strchr("()[]; \t\r\n", $c) === FALSE)
        $str .= $c;
    ungetchar($c);
    return make_symbol($str);
}


/***   P A I R   ***/

function &is_pair (&$data)
{
    if (is_array($data) &&
        array_key_exists('type', $data) &&
        array_key_exists('first', $data) &&
        array_key_exists('next', $data) &&
        $data['type'] === D_PAIR)

        return $data;
    return $FALSE;
}

function &make_pair (&$first, &$next)
{
    assert(is_valid_data(first));
    assert(is_valid_data(next));
    $pair = array(
        'type' => D_PAIR,
        'first' => $first,
        'next' => $next
    );
    return $pair;
}

function &pfirst (&$pair)
{
    assert(is_pair($pair));
    return $pair['first'];
}

function &pnext (&$pair)
{
    assert(is_pair($pair));
    return $pair['next'];
}

function &set_pfirst (&$pair, &$first)
{
    assert(is_pair($pair));
    assert(is_valid_data($pair));
    $pair['first'] = &$first;
    return $first;
}

function &set_pnext (&$pair, &$next)
{
    assert(is_pair($pair));
    assert(is_valid_data($pair));
    $pair['next'] = &$next;
    return $next;
}

function &print_pair_tree (&$pair, $stream)
{
    $p0 = &$pair;
    $i = 0;

    if (!is_pair($pair))
        return $pair;

    fwrite($stream, '(');
    while (is_pair($pair)) {
        if ($i) fwrite($stream, ' ');
        else $i = 1;
        $curr = &pfirst($pair);
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
        $pair = &pnext($pair);
    }
    if (is_symbol($pair)) {
        if ($i) fwrite($stream, ' ');
        else $i = 1;
        fwrite($stream, ". ");
        print_symbol($pair, $stream);
    }
    elseif (is_closure($pair)) {
        if ($i) fwrite($stream, ' ');
        else $i = 1;
        fwrite($stream, ". ");
        print_closure($pair, $stream);
    }
    fwrite($stream, ')');
  
    return $p0;
}

function pair_tree2str (&$pair)
{
    $i = 0;
    $ret = '';

    if (!is_pair($pair))
        return $ret;

    while (is_pair($pair)) {
        if ($i) $ret .= ' ';
        else $i = 1;
        $curr = &pfirst($pair);
        if (is_symbol($curr))
            $ret .= symbol2str($curr);
        else if (is_closure($curr))
            $ret .= closure2str($curr);
        else if (is_pair($curr))
            $ret .= pair_tree2str($curr);
        else {
            assert($curr === NULL);
            $ret .= '()';
        }
        $pair = &pnext($pair);
    }
    if (is_symbol($pair)) {
        if ($i) $ret .= ' ';
        else $i = 1;
        $ret .= '. '.symbol2str($pair);
    }
    else if (is_closure($pair)) {
        if ($i) $ret .= ' ';
        else $i = 1;
        $ret .= '. '.closure2str($pair);
    }
  
    return '('.$ret.')';
}

function &read_pair_tree ()
{
    $pread = NULL;
  
    eat_wspace_comment();
    while (($c = getchar()) !== ')' && $c !== ']') {
        if ($c === FALSE) {
            fwrite(STDERR, "ERROR! read_pair_tree(): missing closing paren\n");
            $r = -1;
            return $r;
        }
        if ($c === '(' || $c === '[') {
            $sub_p = &read_pair_tree();
            if ($sub_p === -1)
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


/***   C L O S U R E   A N D   L A M B D A   ***/

function &is_lambda (&$data)
{
    if (is_list($data) && list_length($data) == 3 &&
        sequal(list_nth($data, 0), make_symbol('lambda')) &&
        is_list(list_nth($data, 1)) &&
        (list_length(list_nth($data, 1)) == 1 ||
         list_length(list_nth($data, 1)) == 0))

        return $data;
    return $FALSE;
}

function &lambda_arg_name (&$lambda)
{
    assert(is_lambda($lambda));
    $arg = list_nth($lambda, 1);
    assert(is_list($arg));
    if (list_length($arg) == 1)
        return list_nth($arg, 0);
    return $FALSE;
}

function &lambda_body (&$lambda)
{
    assert(is_lambda($lambda));
    if (list_length($lambda) == 3)
        return list_nth($lambda, 2);
    return $FALSE;
}

function &is_closure (&$data)
{
    if (is_array($data) &&
        array_key_exists('type', $data) &&
        array_key_exists('arg_name', $data) &&
        array_key_exists('body', $data) &&
        array_key_exists('env', $data) &&
        $data['type'] === D_CLOSURE)

        return $data;
    return $FALSE;
}

function &make_closure (&$lambda, &$env)
{
    assert(is_lambda($lambda));
    assert(is_list($env));
    $cl = array(
        'type' => D_CLOSURE,
        'arg_name' => lambda_arg_name($lambda),
        'body' => lambda_body($lambda),
        'env' => $env
    );
    return $cl;
}

function &closure_arg_name (&$cl)
{
    assert(is_closure($cl));
    return $cl['arg_name'];
}

function &closure_body (&$cl)
{
    assert(is_closure($cl));
    return $cl['body'];
}

function &closure_env (&$cl)
{
    assert(is_closure($cl));
    return $cl['env'];
}

function &print_closure (&$cl, $stream)
{
    assert(is_closure($cl));
    fwrite($stream, "#<closure (");
    if (closure_arg_name($cl))
        print_symbol(closure_arg_name($cl), $stream);
    fwrite($stream, ") ");
    print_data(closure_body($cl), $stream);
    fwrite($stream, " >");
    return $cl;
}

function closure2str (&$cl)
{
    assert(is_closure($cl));
    return "#<closure (" .
        (closure_arg_name($cl) ? symbol2str(closure_arg_name($cl)) : "") .
        ") " . data2str(closure_body($cl)) . " >";
}


/***   L I S T   A N D   D A T A   T O O L S   ***/

function &is_valid_data (&$data)
{
    return is_symbol($data) || is_pair($data) || is_closure($data);
}

function &print_data (&$data, $stream)
{
    if (is_symbol($data))
        print_symbol($data, $stream);
    else if (is_closure($data))
        print_closure($data, $stream);
    else if (is_pair($data))
        print_pair_tree($data, $stream);
    else {
        assert($data === NULL);
        fwrite($stream, "()");
    }
    return $data;
}

function data2str (&$data)
{
    $ret = '';
    if (is_symbol($data))
        $ret = symbol2str($data);
    else if (is_closure($data))
        $ret = closure2str($data);
    else if (is_pair($data))
        $ret = pair_tree2str($data);
    else {
        assert($data === NULL);
        $ret = '()';
    }
    return $ret;
}

function is_list (&$data)
{
    while (is_pair($data))
        $data = &pnext($data);
    return $data === NULL;
}

function list_length (&$list)
{
    assert(is_list($list));
    $len = 0;
    while ($list !== NULL) {
        $len++;
        $list = &pnext($list);
    }
    return $len;
}

function &list_nth (&$list, $n)
{
    assert(is_list($list));
    $ret = &$list;
    while ($n--) {
        assert($ret !== NULL);
        $ret = &pnext($ret);
    }
    assert($ret);
    return pfirst($ret);
}

function &assoc_add (&$alist, &$key, &$value)
{
    assert(is_list($alist));
    assert(is_symbol($key));
    assert(is_valid_data($value));
    return make_pair(make_pair($key, $value), $alist);
}

function &assoc_lookup (&$alist, &$key)
{
    assert(is_list($alist));
    assert(is_symbol($key));
    while ($alist !== NULL) {
        if (sequal(pfirst(pfirst($alist)), $key))
            return pfirst($alist);
        $alist = &pnext($alist);
    }
    return $FALSE;
}


/****************************************************************/
/***   L A M B D A   C A L C U L U S   I N T E R P R E T E R  ***/
/****************************************************************/

function &read_expr ()
{
    eat_wspace_comment();
    $c = getchar();
    if ($c === FALSE)
        return $NULL;
    if ($c === ')' || $c === ']') {
        fwrite(STDERR, "ERROR! read_expr(): too many parens\n");
        return $NULL;
    }
    if ($c === '(' || $c === '[') {
        $expr = &read_pair_tree();
        if ($expr === -1)
            return $NULL;
        else
            return $expr;
    }
    else {
        ungetchar($c);
        return read_symbol();
    }
}


function &print_expr (&$expr)
{
    print data2str($expr)."\n";
    return $expr;
}


function &eval_expr (&$expr, &$env)
{
    $ret = NULL;
    $op = '(unknown)';
    
    if ($expr === NULL) {
        $ret = NULL;
        $op = 'null';
    }
    else if (is_symbol($expr)) {
        $kval = &assoc_lookup($env, $expr);
        $op = 'env-lookup';
        if ($kval) $ret = &pnext($kval);
        else       $ret = &$expr;
    }
    else if (is_lambda($expr)) {
        $ret = &make_closure($expr, $env);
        $op = 'lambda';
    }
    else if (is_list($expr) &&
             (list_length($expr) == 2 || list_length($expr) == 1)) {
        $last_cl = &list_nth($expr, 0);
        $cl = NULL;
        while (is_symbol($cl = &eval_expr($last_cl, $env)) && $cl != $last_cl)
            $last_cl = &$cl;
        if (!is_closure($cl)) {
            fwrite(STDERR, "ERROR! eval_expr(): expression is not a closure: ");
            print_data($cl, STDERR);
            fwrite(STDERR, "\n\n");
            return $NULL;
        }
        if (list_length($expr) == 1)
            $new_env = &$env;
        else {
            $last_arg = &list_nth($expr, 1);
            $arg = NULL;
            while (is_symbol($arg = &eval_expr($last_arg, $env)) && $arg != $last_arg)
                $last_arg = &$arg;
            $new_env = &assoc_add(closure_env($cl), closure_arg_name($cl), $arg);
        }
        $ret = &eval_expr(closure_body($cl), $new_env);
        $op = 'application';
    }
    else {
        fwrite(STDERR, "ERROR! eval_expr(): unrecognised expression: ");
        print_data($expr, STDERR);
        fwrite(STDERR, "\n\n");
        return $NULL;
    }
    /*
    fwrite(STDERR, "EXPR: ");
    print_data($expr, STDERR);
    fwrite(STDERR, "\nENV: ");
    print_data($env, STDERR);
    fwrite(STDERR, "\nCASE: ");
    fwrite(STDERR, $op);
    fwrite(STDERR, "\nRET: ");
    print_data($ret, STDERR);
    fwrite(STDERR, "\n\n");
    */
    return $ret;
}


function main ()
{
    print_expr(eval_expr(read_expr(), $NULL));
}

main();
