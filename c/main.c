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

void eat_wspace_comment ()
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

PNode makeNode (void *first, void *next)
{
  PNode node = malloc(sizeof(TNode));
  assert(node != NULL);
  node->first = first;
  node->next = next;
  return node;
}

void delNode (PNode node)
{
  free(node);
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

PData isSymbol (PData data)
{
  if (data && data->type == D_SYMBOL)
    return data;
  return NULL;
}

PData makeSymbol (char *string)
{
  PData symbol = malloc(sizeof(TData));
  assert(symbol != NULL);
  symbol->type = D_SYMBOL;
  symbol->data.symbol = string;
  return symbol;
}

void delSymbol (PData symbol)
{
  assert(isSymbol(symbol));
  free(symbol);
}

PData printSymbol (PData symbol)
{
  assert(isSymbol(symbol));
  fputs(symbol->data.symbol, stdout);
  return symbol;
}

# define MAXSYMBOL 256

PData readSymbol ()
{
  char symbol[MAXSYMBOL];
  int c, i = 0;

  while (i < MAXSYMBOL && (c = getchar()) != EOF && strchr("()[]; \t\r\n", c) == NULL)
    symbol[i++] = c;
  if (i == MAXSYMBOL)
    i--;
  else
    ungetchar(c);
  symbol[i] = '\0';

  return makeSymbol(strdup(symbol));
}

PData isPair (PData data)
{
  if (data && data->type == D_PAIR)
    return data;
  return NULL;
}

PData makePair (PData first, PData next)
{
  PData pair = malloc(sizeof(TData));
  assert(pair != NULL);
  PNode node = makeNode(first, next);
  pair->type = D_PAIR;
  pair->data.pair = node;
  return pair;
}

void delPair (PData pair)
{
  assert(isPair(pair));
  delNode(pair->data.pair);
  free(pair);
}

PData pairFirst (PData pair)
{
  assert(isPair(pair));
  return pair->data.pair->first;
}

PData pairNext (PData pair)
{
  assert(isPair(pair));
  return pair->data.pair->next;
}

void delPairTree (PData pair)
{
  assert(isPair(pair));
  if (isPair(pairFirst(pair)))
    delPairTree(pairFirst(pair));
  if (isPair(pairNext(pair)))
    delPairTree(pairNext(pair));
  delPair(pair);
}

PData printPairTree (PData pair)
{
  PData p0 = pair;
  int i = 0;

  if (!isPair(pair))
    return pair;

  putchar('(');
  while (isPair(pair)) {
    if (i) putchar(' ');
    else i = 1;

    if (isSymbol(pairFirst(pair)))
      printSymbol(pairFirst(pair));
    else {   /* pair */
      if (pairFirst(pair) == NULL)
        fputs("()", stdout);
      else
        printPairTree(pairFirst(pair));
    }
    pair = pairNext(pair);
  }
  putchar(')');
  
  return p0;
}

int readPairTree (PData *pairs_read)
{
  PData pair = *pairs_read = NULL, sub_pair;
  int c, r, i = 0;
  
  eat_wspace_comment();
  while ((c = getchar()) != ')' && c != ']') {
    if (c == EOF) {
      printf("ERROR! readPairTree: missing closing paren\n");
      return -1;
    }
    if (c == '(' || c == '[') {
      if ((r = readPairTree(&sub_pair)) < 0)
        return r;
      if (*pairs_read == NULL)
        *pairs_read = pair = makeNode(NODETYPE_NODE, sub_pair, NULL);
      else {
        pair->next = makeNode(NODETYPE_NODE, sub_pair, NULL);
        pair = pair->next;
      }
    }
    else {
      ungetchar(c);
      if (*pairs_read == NULL)
        *pairs_read = pair = makeNode(NODETYPE_SYMBOL, readSymbol(), NULL);
      else {
        pair->next = makeNode(NODETYPE_SYMBOL, readSymbol(), NULL);
        pair = pair->next;
      }
    }
    i++;
    eat_wspace_comment();
  }

  return i;
}

int listLength (PData list)
{
  int len = 0;
  assert(isPair(list));
  while (list != NULL) {
    len++;
    list = pairNext(list);
  }
  return len;
}


/***************
      L C I
****************/

/***   R E A D E R   ***/
PNode read_expr ()
{
  PNode expr, list;
  int c;
  
  eat_wspace_comment();
  c = getchar();
  if (c == EOF)
    return NULL;
  if (c == ')' || c == ']') {
    printf("read_expr error: too many parens\n");
    return NULL;
  }
  if (c == '(' || c == '[') {
      if (readNodes(&list) < 0)
        return NULL;
    expr = makeNode(NODETYPE_NODE, list, NULL);
  }
  else {
    ungetchar(c);
    expr = makeNode(NODETYPE_SYMBOL, readSymbol(), NULL);
  }
  return expr;
}


/***   E V A L U A T O R   ***/
PNode print_expr (PNode expr);

PNode eval_expr (PNode expr, PNode env)
{
  if (listLength(expr) > 1 &&
      expr->type == NODETYPE_SYMBOL &&
      strcmp("quote", expr->data) == 0) {
    return expr->next;
  }
  /*
  if (expr->type == NODETYPE_SYMBOL)
    return local_env(expr->data);
  */

  if (listLength(expr) == 3 && expr->type == NODETYPE_SYMBOL && strcmp("lambda", expr->data) == 0) {

  }

  if (listLength(expr) == 2) {

  }

  fputs("eval_expr error: unrecognised expression:\n", stdout);
  print_expr(expr);
  return env;
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
PNode print_expr (PNode expr)
{
  if (expr == NULL) {
    fputs("[NULL]\n", stdout);
    return NULL;
  }
  if (expr->type == NODETYPE_SYMBOL)
    printSymbol(expr->data);
  else {         /* NODETYPE_NODE */
    if (expr->data == NULL)
      fputs("()", stdout);
    else
      printNodes(expr->data);
  }
  putchar('\n');
  return expr;
}


/***   M A I N   L O O P  ***/
int main ()
{
  PNode expr;
  while ((expr = read_expr()) != NULL) {
    if (expr->type == NODETYPE_SYMBOL)
      printf("%s\n", (char*) expr->data);
    else
      print_expr(eval_expr(expr->data, NULL));
    delNodes(expr);
  }
  return 0;
}
