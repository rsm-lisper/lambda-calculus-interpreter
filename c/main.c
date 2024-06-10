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


/***   N O D E   ***/

typedef struct _tnode TNode, *PNode;
struct _tnode {
  void *first;
  void *next;
};

PNode make_node (void *first, void *next)
{
  PNode node = malloc(sizeof(TNode));
  assert(node != NULL);
  node->first = first;
  node->next = next;
  return node;
}

void *nfirst (PNode node)
{
  assert(node != NULL);
  return node->first;
}
void *nnext (PNode node)
{
  assert(node != NULL);
  return node->next;
}
void *set_nfirst (PNode node, void *first)
{
  assert(node != NULL);
  return node->first = first;
}
void *set_nnext (PNode node, void *next)
{
  assert(node != NULL);
  return node->next = next;
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
    PNode pair;
    PNode closure;
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
  pair->data.pair = make_node(first, next);
  return pair;
}

PData pfirst (PData pair)
{
  assert(is_pair(pair));
  return nfirst(pair->data.pair);
}

PData pnext (PData pair)
{
  assert(is_pair(pair));
  return nnext(pair->data.pair);
}

PData set_pfirst (PData pair, PData first)
{
  assert(is_pair(pair));
  return set_nfirst(pair->data.pair, first);
}

PData set_pnext (PData pair, PData next)
{
  assert(is_pair(pair));
  return set_nnext(pair->data.pair, next);
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


/***   C L O S U R E   ***/

PData is_closure (PData data)
{
  if (data && data->type == D_CLOSURE)
    return data;
  return NULL;
}

PData make_closure (PData lambda, PData env)
{
  PData cl = malloc(sizeof(TData));
  assert(cl != NULL);
  cl->type = D_CLOSURE;
  cl->data.closure = make_node(lambda, env);
  return cl;
}

PData closure_lambda (PData cl)
{
  assert(is_closure(cl));
  return nfirst(cl->data.closure);
}

PData closure_arg_name (PData cl)
{
  assert(is_closure(cl));
  return pfirst(pfirst(pnext(closure_lambda(cl))));
}

PData closure_body (PData cl)
{
  assert(is_closure(cl));
  return pfirst(pnext(pnext(closure_lambda(cl))));
}

PData closure_env (PData cl)
{
  assert(is_closure(cl));
  return nnext(cl->data.closure);
}

PData print_data();
PData print_closure (PData cl, FILE *stream)
{
  assert(is_closure(cl));
# ifdef LCI_DEBUG
  fprintf(stream, "#<closure %p (", cl);*/
# else
  fputs("#<closure (", stream);
# endif
  print_symbol(closure_arg_name(cl), stream);
  fputs(") ", stream);
  print_data(closure_body(cl), stream);
  fputs(" | ", stream);
  print_data(closure_env(cl), stream);
  fputs(">", stream);
  return cl;
}


/***   D A T A   T O O L S   ***/

PData print_data (PData data, FILE *stream)
{
  if (is_symbol(data))
    print_symbol(data, stream);
  else if (is_closure(data))
    print_closure(data, stream);
  else if (is_pair(data))
    print_pair_tree(data, stream);
  else if (data == NULL)
    fputs("()", stream);
  return data;
}

int list_length (PData list)
{
  int len = 0;
  assert(is_pair(list));
  while (list != NULL) {
    len++;
    list = pnext(list);
  }
  return len;
}

PData assoc_add (PData assoc_list, PData key, PData value)
{
  return make_pair(make_pair(key, value), assoc_list);
}

PData assoc_lookup (PData assoc_list, PData key)
{
  while (assoc_list) {
    if (sequal(pfirst(pfirst(assoc_list)), key))
      return pfirst(assoc_list);
    assoc_list = pnext(assoc_list);
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


PData is_lambda (PData expr)
{
  if (is_pair(expr) && list_length(expr) == 3 &&
      is_symbol(pfirst(expr)) &&
      sequal(pfirst(expr), make_symbol("lambda")))
    return expr;
  return NULL;
}


PData eval_expr (PData expr, PData env)
{
  if (expr == NULL)
    return NULL;
  if (is_symbol(expr)) {
    PData kval = assoc_lookup(env, expr);
    return kval ? pnext(kval) : expr;
  }
  if (is_lambda(expr)) {
    return make_closure(expr, env);
  }
  if (is_pair(expr) && list_length(expr) == 2) {
    PData cl = eval_expr(pfirst(expr), env);
    PData arg = eval_expr(pfirst(pnext(expr)), env);
    if (!is_closure(cl)) {
      fputs("ERROR! eval_expr(): expression is not a closure: ", stderr);
      print_data(cl, stderr);
      fputs("\n\n", stderr);
      return NULL;
    }
    PData new_env = assoc_add(closure_env(cl), closure_arg_name(cl), arg);
    PData new_expr = eval_expr(closure_body(cl), new_env);
    return new_expr;
  }

  fputs("ERROR! eval_expr(): unrecognised expression: ", stderr);
  print_data(expr, stderr);
  fputs("\n\n", stderr);
  return NULL;
}


int main ()
{
  print_expr(eval_expr(read_expr(), NULL));
  return 0;
}
