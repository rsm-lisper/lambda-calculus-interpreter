# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>

/*****************************************/
/***   L O W   L E V E L   T O O L S   ***/
/*****************************************/

/***   C H A R   /   S T R I N G   ***/

# define ungetchar(C) ungetc((C), stdin)

char *strdup (const char *str)
{
  char *ret = malloc(strlen(str) + 1);
  assert(ret != NULL);
  strcpy(ret, str);
  return ret;
}

void eat_wspace_comment (void)
{
  int c;
  while ((c = getchar()) != EOF && strchr("; \t\r\n", c) != NULL)
    if (c == ';') {
      do c = getchar();
      while (c != EOF && c != '\n' && c != '\r');
      ungetchar(c);
    }
  ungetchar(c);
}


/*********************************************/
/***   D A T A   /   D A T A   T Y P E S   ***/
/*********************************************/

enum TDataTypes {
  D_SYMBOL,
  D_PAIR,
  D_CLOSURE
};

typedef struct _tdata TData, *PData;
struct _tdata {
  enum TDataTypes type;
  union {
    char *symbol;
    struct {
      PData first;
      PData next;
    } pair;
    struct {
      PData arg_name;
      PData body;
      PData env;
      PData first_name;
    } closure;
  } data;
};


/***   S Y M B O L   ***/

PData is_symbol (PData data)
{
  if (data && data->type == D_SYMBOL)
    return data;
  return NULL;
}

char *svalue (PData data)
{
  assert(is_symbol(data));
  return data->data.symbol;
}

int sequal (PData sa, PData sb)
{
  assert(is_symbol(sa) && is_symbol(sb));
  return strcmp(svalue(sa), svalue(sb)) == 0;
}

PData make_symbol (char *str)
{
  PData symbol = malloc(sizeof(TData));
  assert(symbol != NULL);
  symbol->type = D_SYMBOL;
  symbol->data.symbol = str;
  return symbol;
}

PData print_symbol (PData symbol, FILE *stream)
{
  assert(is_symbol(symbol));
  fputs(symbol->data.symbol, stream);
  return symbol;
}

# define MAXSYMBOL 256

PData read_symbol (void)
{
  char str[MAXSYMBOL];
  int c, i = 0;

  while (i < MAXSYMBOL && (c = getchar()) != EOF &&
         strchr("()[]; \t\r\n", c) == NULL) {
    str[i++] = c;
  }
  if (i == MAXSYMBOL)
    i--;
  else
    ungetchar(c);
  str[i] = '\0';

  return make_symbol(strdup(str));
}


/***   P A I R   ***/

PData is_pair (PData data)
{
  if (data && data->type == D_PAIR)
    return data;
  return NULL;
}

PData make_pair (PData first, PData next)
{
  PData pair = malloc(sizeof(TData));
  assert(pair != NULL);
  pair->type = D_PAIR;
  pair->data.pair.first = first;
  pair->data.pair.next = next;
  return pair;
}

PData pfirst (PData pair)
{
  assert(is_pair(pair));
  return pair->data.pair.first;
}

PData pnext (PData pair)
{
  assert(is_pair(pair));
  return pair->data.pair.next;
}

PData set_pfirst (PData pair, PData first)
{
  assert(is_pair(pair));
  pair->data.pair.first = first;
  return first;
}

PData set_pnext (PData pair, PData next)
{
  assert(is_pair(pair));
  pair->data.pair.next = next;
  return next;
}

PData is_closure();
PData print_closure();
PData print_pair_tree (PData pair, FILE *stream)
{
  PData p0 = pair;
  int i = 0;

  if (!is_pair(pair))
    return pair;

  fputc('(', stream);
  while (is_pair(pair)) {
    if (i) fputc(' ', stream);
    else i = 1;
    PData curr = pfirst(pair);
    if (is_symbol(curr))
      print_symbol(curr, stream);
    else if (is_closure(curr))
      print_closure(curr, stream);
    else if (is_pair(curr))
      print_pair_tree(curr, stream);
    else {
      assert(curr == NULL);
      fputs("()", stream);
    }
    pair = pnext(pair);
  }
  if (pair && is_symbol(pair)) {
    if (i) fputc(' ', stream);
    else i = 1;

    fputs(". ", stream);
    print_symbol(pair, stream);
  }
  fputc(')', stream);
  
  return p0;
}

int read_pair_tree (PData *pread)
{
  PData pair = *pread = NULL;
  int c, i = 0;
  
  eat_wspace_comment();
  while ((c = getchar()) != ')' && c != ']') {
    if (c == EOF) {
      fputs("ERROR! read_pair_tree(): missing closing paren\n", stderr);
      return -1;
    }
    if (c == '(' || c == '[') {
      PData sub_p;
      int r;
      if ((r = read_pair_tree(&sub_p)) < 0)
        return r;
      if (*pread == NULL)
        *pread = pair = make_pair(sub_p, NULL);
      else {
        pair = set_pnext(pair, make_pair(sub_p, NULL));
      }
    }
    else {
      ungetchar(c);
      if (*pread == NULL)
        *pread = pair = make_pair(read_symbol(), NULL);
      else {
        pair = set_pnext(pair, make_pair(read_symbol(), NULL));
      }
    }
    i++;
    eat_wspace_comment();
  }
  return i;
}


/***   C L O S U R E   A N D   L A M B D A   ***/

int is_list ();
PData list_nth ();
int list_length ();
PData is_lambda (PData data)
{
  if (data && is_list(data) && list_length(data) == 3 &&
      sequal(list_nth(data, 0), make_symbol("lambda")) &&
      is_list(list_nth(data, 1)) &&
      (list_length(list_nth(data, 1)) == 1 ||
       list_length(list_nth(data, 1)) == 0))
    return data;
  return NULL;
}

PData lambda_arg_name (PData lambda)
{
  assert(is_lambda(lambda));
  PData arg = list_nth(lambda, 1);
  assert(is_list(arg));
  if (list_length(arg) == 1)
    return list_nth(arg, 0);
  return NULL;
}

PData lambda_body (PData lambda)
{
  assert(is_lambda(lambda));
  if (list_length(lambda) == 3)
    return list_nth(lambda, 2);
  return NULL;
}

PData is_closure (PData data)
{
  if (data && data->type == D_CLOSURE)
    return data;
  return NULL;
}

PData make_closure (PData lambda, PData env, PData first_name)
{
  assert(is_lambda(lambda));
  PData cl = malloc(sizeof(TData));
  assert(cl);
  cl->type = D_CLOSURE;
  cl->data.closure.arg_name = lambda_arg_name(lambda);
  cl->data.closure.body = lambda_body(lambda);
  cl->data.closure.env = env;
  cl->data.closure.first_name = first_name;
  return cl;
}

PData set_closure_first_name (PData cl, PData first_name)
{
  assert(is_closure(cl));
  assert(is_symbol(first_name));
  cl->data.closure.first_name = first_name;
  return cl;
}

PData closure_first_name (PData cl)
{
  assert(is_closure(cl));
  return cl->data.closure.first_name;
}

PData closure_arg_name (PData cl)
{
  assert(is_closure(cl));
  return cl->data.closure.arg_name;
}

PData closure_body (PData cl)
{
  assert(is_closure(cl));
  return cl->data.closure.body;
}

PData closure_env (PData cl)
{
  assert(is_closure(cl));
  return cl->data.closure.env;
}


PData print_data();
PData print_closure (PData cl, FILE *stream)
{
  assert(is_closure(cl));
  fputs("#<closure ", stream);
  if (closure_first_name(cl)) {
    print_symbol(closure_first_name(cl), stream);
    fputc(' ', stream);
  }
  fputs("(", stream);
  if (closure_arg_name(cl))
    print_symbol(closure_arg_name(cl), stream);
  fputs(") ", stream);
  print_data(closure_body(cl), stream);
# ifdef TRACE
  fputs(" | ", stream);
  print_data(closure_env(cl), stream);
# endif
  fputs(" >", stream);
  return cl;
}


/***   L I S T S   A N D   D A T A   T O O L S   ***/

PData print_data (PData data, FILE *stream)
{
  if (is_symbol(data))
    print_symbol(data, stream);
  else if (is_closure(data))
    print_closure(data, stream);
  else if (is_pair(data))
    print_pair_tree(data, stream);
  else {
    assert(data == NULL);
    fputs("()", stream);
  }
  return data;
}

int is_list (PData list)
{
  PData lt = list;
  while (is_pair(lt))
    lt = pnext(lt);
  return lt == NULL;
}

int list_length (PData list)
{
  assert(is_list(list));
  int len = 0;
  while (list) {
    len++;
    list = pnext(list);
  }
  return len;
}

PData list_nth (PData list, unsigned int n)
{
  assert(is_list(list));
  PData ret = list;
  while (n--) {
    assert(ret);
    ret = pnext(ret);
  }
  assert(ret);
  return pfirst(ret);
}

PData assoc_add (PData alist, PData key, PData value)
{
  return make_pair(make_pair(key, value), alist);
}

PData assoc_lookup (PData alist, PData key)
{
  assert(is_list(alist));
  assert(is_symbol(key));
  while (alist) {
    if (sequal(pfirst(pfirst(alist)), key))
      return pfirst(alist);
    alist = pnext(alist);
  }
  return NULL;
}


/****************************************************************/
/***   L A M B D A   C A L C U L U S   I N T E R P R E T E R  ***/
/****************************************************************/

PData read_expr (void)
{
  PData expr;
  int c;
  
  eat_wspace_comment();
  c = getchar();
  if (c == EOF)
    return NULL;
  if (c == ')' || c == ']') {
    fputs("ERROR! read_expr(): too many parens\n", stderr);
    return NULL;
  }
  if (c == '(' || c == '[') {
    if (read_pair_tree(&expr) < 0)
      return NULL;
    else
      return expr;
  }
  else {
    ungetchar(c);
    expr = read_symbol();
  }
  return expr;
}


PData print_expr (PData expr)
{
  print_data(expr, stdout);
  fputc('\n', stdout);
  return expr;
}


PData eval_expr (PData expr, PData env, unsigned int parent_id)
{
  PData ret;
  static unsigned int global_id = 0;
  global_id++;
  unsigned int id = parent_id;
  id = global_id;

# ifdef TRACE
  if (id == 1)
    fputs("(\n", stderr);
# endif
  
  if (expr == NULL) {
    ret = NULL;
  }
  else if (is_symbol(expr)) {
    PData kval = assoc_lookup(env, expr);
    if (!kval)
      ret = expr;
    else {
      if (is_closure(pnext(kval)) &&
          closure_first_name(pnext(kval)) == NULL)
        set_closure_first_name(pnext(kval), expr);
      ret = pnext(kval);
    }
  }
  else if (is_lambda(expr)) {
    ret = make_closure(expr, env, NULL);
  }
  else if (is_list(expr) &&
           (list_length(expr) == 2 || list_length(expr) == 1)) {
    if (list_length(expr) == 1) {
      PData last_cl = list_nth(expr, 0), cl = NULL;
      while (is_symbol(cl = eval_expr(last_cl, env, id)) && cl != last_cl)
        last_cl = cl;
      if (!is_closure(cl)) {
        fputs("ERROR! eval_expr(): expression is not a closure: ", stderr);
        print_data(cl, stderr);
        fputs("\n\n", stderr);
        ret = NULL;
      }
      ret = eval_expr(closure_body(cl), env, id);
    }
    else {
      PData last_cl = list_nth(expr, 0), cl = NULL;
      while (is_symbol(cl = eval_expr(last_cl, env, id)) &&
             cl != last_cl) {
        last_cl = cl;
      }
      PData last_arg = list_nth(expr, 1), arg = NULL;
      while (is_symbol(arg = eval_expr(last_arg, env, id)) &&
             arg != last_arg) {
        last_arg = arg;
      }
      if (!is_closure(cl)) {
        fputs("ERROR! eval_expr(): expression is not a closure: ", stderr);
        print_data(cl, stderr);
        fputs("\n\n", stderr);
        ret = NULL;
      }
      PData new_env = assoc_add(closure_env(cl), closure_arg_name(cl), arg);
      ret = eval_expr(closure_body(cl), new_env, id);
    }
  }
  else {
    fputs("ERROR! eval_expr(): unrecognised expression: ", stderr);
    print_data(expr, stderr);
    fputs("\n\n", stderr);
    ret = NULL;
  }

# ifdef TRACE
  fprintf(stderr, "((id . %d) (parent-id . %d) (expr ", id, parent_id);
  print_data(expr, stderr);
  fputs(") (env ", stderr);
  print_data(env, stderr);
  fputs(") (return ", stderr);
  print_data(ret, stderr);
  fputs("))\n", stderr);
  if (id == 1)
    fputs(")\n", stderr);
# endif

  return ret;
}


int main ()
{
  print_expr(eval_expr(read_expr(), NULL, 0));
  return 0;
}
