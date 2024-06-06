# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>

/*********************/
/***   T O O L S   ***/
/*********************/


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


void del_node (PNode n)
{
  free(n);
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
PData is_symbol (PData d)
{
  if (d && d->type == D_SYMBOL)
    return d;
  return NULL;
}

char *svalue (PData d)
{
  assert(is_symbol(d));
  return d->data.symbol;
}

int sequal (PData sa, PData sb)
{
  assert(is_symbol(sa) && is_symbol(sb));
  return strcmp(svalue(sa), svalue(sb)) == 0;
}

PData make_symbol (char *str)
{
  PData s = malloc(sizeof(TData));
  assert(s != NULL);
  s->type = D_SYMBOL;
  s->data.symbol = str;
  return s;
}

void del_symbol (PData s)
{
  assert(is_symbol(s));
  free(s);
}

PData print_symbol (PData s, FILE *stream)
{
  assert(is_symbol(s));
  fputs(s->data.symbol, stream);
  return s;
}

# define MAXSYMBOL 256

PData read_symbol (void)
{
  char s[MAXSYMBOL];
  int c, i = 0;

  while (i < MAXSYMBOL && (c = getchar()) != EOF &&
         strchr("()[]; \t\r\n", c) == NULL) {
    s[i++] = c;
  }
  if (i == MAXSYMBOL)
    i--;
  else
    ungetchar(c);
  s[i] = '\0';

  return make_symbol(strdup(s));
}


/***   P A I R   ***/
PData is_pair (PData d)
{
  if (d && d->type == D_PAIR)
    return d;
  return NULL;
}

PData make_pair (PData first, PData next)
{
  PData p = malloc(sizeof(TData));
  assert(p != NULL);
  p->type = D_PAIR;
  p->data.pair = make_node(first, next);
  return p;
}

void del_pair (PData p)
{
  assert(is_pair(p));
  del_node(p->data.pair);
  free(p);
}

PData pfirst (PData p)
{
  assert(is_pair(p));
  return p->data.pair->first;
}

PData pnext (PData p)
{
  assert(is_pair(p));
  return p->data.pair->next;
}

PData set_pfirst (PData p, PData val)
{
  assert(is_pair(p));
  return p->data.pair->first = val;
}

PData set_pnext (PData p, PData val)
{
  assert(is_pair(p));
  return p->data.pair->next = val;
}

void del_pair_tree (PData p)
{
  assert(is_pair(p));
  if (is_pair(pfirst(p)))
    del_pair_tree(pfirst(p));
  if (is_pair(pnext(p)))
    del_pair_tree(pnext(p));
  del_pair(p);
}

PData is_closure();
PData print_closure();
PData print_pair_tree (PData p, FILE *stream)
{
  PData p0 = p;
  int i = 0;

  if (!is_pair(p))
    return p;

  fputc('(', stream);
  while (is_pair(p)) {
    if (i) fputc(' ', stream);
    else i = 1;
    PData curr = pfirst(p);
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
    p = pnext(p);
  }
  if (p && is_symbol(p)) {
    if (i) fputc(' ', stream);
    else i = 1;

    fputs(". ", stream);
    print_symbol(p, stream);
  }
  fputc(')', stream);
  
  return p0;
}

int read_pair_tree (PData *pread)
{
  PData p = *pread = NULL;
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
        *pread = p = make_pair(sub_p, NULL);
      else {
        set_pnext(p, make_pair(sub_p, NULL));
        p = pnext(p);
      }
    }
    else {
      ungetchar(c);
      if (*pread == NULL)
        *pread = p = make_pair(read_symbol(), NULL);
      else {
        set_pnext(p, make_pair(read_symbol(), NULL));
        p = pnext(p);
      }
    }
    i++;
    eat_wspace_comment();
  }
  return i;
}

/***   C L O S U R E   ***/
PData is_closure (PData d)
{
  if (d && d->type == D_CLOSURE)
    return d;
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

void del_closure (PData cl)
{
  assert(is_closure(cl));
  del_node(cl->data.closure);
  free(cl);
}

PData closure_lambda (PData cl)
{
  assert(is_closure(cl));
  return cl->data.closure->first;
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
  return cl->data.closure->next;
}

PData print_data();
PData print_closure (PData cl, FILE *stream)
{
  assert(is_closure(cl));
  fprintf(stream, "#<closure %p (", cl);
  print_symbol(closure_arg_name(cl), stream);
  fputs(") ", stream);
  print_data(closure_body(cl), stream);
  fputs(" | ", stream);
  print_data(closure_env(cl), stream);
  fputs(">", stream);
  return cl;
}

/***   D A T A   T O O L S   ***/
PData print_data (PData d, FILE *stream)
{
  if (is_symbol(d))
    print_symbol(d, stream);
  else if (is_closure(d))
    print_closure(d, stream);
  else if (is_pair(d))
    print_pair_tree(d, stream);
  else if (d == NULL)
    fputs("()", stream);
  return d;
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


/***   R E A D E R   ***/
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


/***   E V A L U A T O R   ***/
int is_lambda (PData expr)
{
  return (is_pair(expr) && list_length(expr) == 3 &&
          is_symbol(pfirst(expr)) &&
          sequal(pfirst(expr), make_symbol("lambda")));
}

PData eval_expr (PData expr, PData env)
{
  printf("{");
  print_data(expr, stdout);
  printf(", ");
  print_data(env, stdout);
  printf("}\n");
  
  PData ret = NULL;
  
  if (is_symbol(expr)) {
    PData kval = assoc_lookup(env, expr);
    ret = kval ? pnext(kval) : expr;
  }
  else if (is_lambda(expr)) {
    ret = make_closure(expr, env);
  }
  else if (is_pair(expr) && list_length(expr) == 2) {
    PData cl = eval_expr(pfirst(expr), env);
    PData arg = eval_expr(pfirst(pnext(expr)), env);
    if (!is_closure(cl)) {
      fputs("ERROR! eval_expr(): expression is not a closure:\n", stderr);
      print_data(cl, stderr);
      ret = NULL;
    }
    else {
      PData new_env = assoc_add(closure_env(cl), closure_arg_name(cl), arg);
      PData new_expr = eval_expr(closure_body(cl), new_env);
      del_pair(new_env);
      ret = new_expr;
    }
  }
  else {
    fputs("ERROR! eval_expr(): unrecognised expression:\n", stderr);
    print_data(expr, stderr);
    ret = NULL;
  }
  printf("EVAL: ");
  print_data(expr, stdout);
  printf("\n env: ");
  print_data(env, stdout);
  printf("\n => ");
  print_data(ret, stdout);
  printf("\n\n");

  return ret;
}


/***   P R I N T E R   ***/
PData print_expr (PData expr, FILE *stream)
{
  print_data(expr, stream);
  fputc('\n', stream);
  return expr;
}


/***   M A I N   L O O P  ***/
void del_expr (PData expr)
{
  if (is_symbol(expr))
    del_symbol(expr);
  else if (is_closure(expr))
    del_closure(expr);
  else if (is_pair(expr))
    del_pair_tree(expr);
  else
    assert(expr == NULL);
}

int main ()
{
  PData expr;
  while ((expr = read_expr()) != NULL) {
    print_expr(eval_expr(expr, NULL), stdout);
    del_expr(expr);
  }
  return 0;
}
