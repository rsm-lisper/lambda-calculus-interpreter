# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>

/*******************
      T O O L S
********************/

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

/***   D A T A   /   D A T A   T Y P E S   ***/

enum TDataTypes {
  D_SYMBOL,
  D_PAIR
};

typedef struct _tdata TData, *PData;
struct _tdata {
  enum TDataTypes type;
  union {
    char *symbol;
    PNode pair;
  } data;
};

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

PData print_symbol (PData s)
{
  assert(is_symbol(s));
  fputs(s->data.symbol, stdout);
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
  PNode n = make_node(first, next);
  p->type = D_PAIR;
  p->data.pair = n;
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

void del_pair_tree (PData p)
{
  assert(is_pair(p));
  if (is_pair(pfirst(p)))
    del_pair_tree(pfirst(p));
  if (is_pair(pnext(p)))
    del_pair_tree(pnext(p));
  del_pair(p);
}

PData print_pair_tree (PData p)
{
  PData p0 = p;
  int i = 0;

  if (!is_pair(p))
    return p;

  putchar('(');
  while (is_pair(p)) {
    if (i) putchar(' ');
    else i = 1;

    if (is_symbol(pfirst(p)))
      print_symbol(pfirst(p));
    else {   /* pair */
      if (pfirst(p) == NULL)
        fputs("()", stdout);
      else
        print_pair_tree(pfirst(p));
    }
    p = pnext(p);
  }
  putchar(')');
  
  return p0;
}

int read_pair_tree (PData *pread)
{
  PData p = *pread = NULL, sub_p;
  int c, r, i = 0;
  
  eat_wspace_comment();
  while ((c = getchar()) != ')' && c != ']') {
    if (c == EOF) {
      fputs("ERROR! read_pair_tree(): missing closing paren\n", stderr);
      return -1;
    }
    if (c == '(' || c == '[') {
      if ((r = read_pair_tree(&sub_p)) < 0)
        return r;
      if (*pread == NULL)
        *pread = p = make_pair(sub_p, NULL);
      else {
        p->data.pair->next = make_pair(sub_p, NULL);
        p = p->data.pair->next;
      }
    }
    else {
      ungetchar(c);
      if (*pread == NULL)
        *pread = p = make_pair(read_symbol(), NULL);
      else {
        p->data.pair->next = make_pair(read_symbol(), NULL);
        p = pnext(p);
      }
    }
    i++;
    eat_wspace_comment();
  }
  return i;
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


/***************
      L C I
****************/

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
PData eval_expr (PData expr, PData env)
{
  if (is_pair(expr) && is_symbol(pfirst(expr)) &&
      sequal(make_symbol("quote"), pfirst(expr))) {
    return pnext(expr);
  }
  
  if (is_symbol(expr)) {
    /* */
    
  }
  else {
    if (list_length(expr) == 3 && is_symbol(pfirst(expr)) &&
        sequal(make_symbol("lambda"), pfirst(expr))) {
      /* */
    
    }

    if (list_length(expr) == 2) {
      /* */

    }
  }

  if (env != NULL) expr = env; /* just to trick Wall for now */
  fputs("ERROR! eval_expr(): unrecognised expression\n", stderr);
  return expr;
  /*
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
  */
}


/***   P R I N T E R   ***/
PData print_expr (PData expr)
{
  if (expr == NULL) {
    puts("[NULL]\n");
    return NULL;
  }
  if (is_symbol(expr))
    print_symbol(expr);
  else {
    if (pfirst(expr) == NULL)
      fputs("()", stdout);
    else
      print_pair_tree(expr);
  }
  putchar('\n');
  return expr;
}

void del_expr (PData expr)
{
  if (is_symbol(expr))
    del_symbol(expr);
  else
    del_pair_tree(expr);
}

/***   M A I N   L O O P  ***/
int main ()
{
  PData expr;
  while ((expr = read_expr()) != NULL) {
    print_expr(eval_expr(expr, NULL));
    del_expr(expr);
  }
  return 0;
}
