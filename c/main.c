# include <stdio.h>
# include <string.h>
# include <stdlib.h>

/*******************
      T O O L S
********************/

/***   C H A R / S T R I N G   ***/

# define ungetchar(C) ungetc((C), stdin)

char *strdup (const char *str)
{
  char *ret = malloc(strlen(str) + 1);
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

enum TNodeTypes {
  NODETYPE_NODE,
  NODETYPE_SYMBOL
};

typedef struct _tnode TNode, *PNode;
struct _tnode {
  enum TNodeTypes type;
  void *data;
  PNode next;
};

PNode makeNode (enum TNodeTypes type, void *data, PNode next)
{
  PNode node = malloc(sizeof(TNode));
  if (node == NULL)
    return NULL;
  node->type = type;
  node->data = data;
  node->next = next;
  return node;
}

void delNodes (PNode node)
{
  if (node == NULL)
    return;
  if (node->next != NULL)
    delNodes(node->next);
  if (node->type == NODETYPE_NODE)
    delNodes(node->data);
  else
    free(node->data);
  free(node);
}

int listLength (PNode list)
{
  int len = 0;
  while (list != NULL) {
    len++;
    list = list->next;
  }
  return len;
}

char *printSymbol (char *symbol)
{
  fputs(symbol, stdout);
  return symbol;
}

PNode printNodes (PNode node)
{
  PNode n0 = node;
  int n = 0;
  if (node == NULL)
    return NULL;

  putchar('(');
  while (node != NULL) {
    if (n) putchar(' ');
    else n = 1;
      
    if (node->type == NODETYPE_SYMBOL)
      printSymbol(node->data);
    else {         /* NODETYPE_NODE */
      if (node->data == NULL)
        fputs("()", stdout);
      else
        printNodes(node->data);
    }
    node = node->next;
  }
  putchar(')');
  return n0;
}

# define MAXSYMBOL 256

char *readSymbol ()
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
  return strdup(symbol);
}

int readNodes (PNode *nodes_read)
{
  PNode node = *nodes_read = NULL, sub_node;
  int c, r, i = 0;
  
  eat_wspace_comment();
  while ((c = getchar()) != ')' && c != ']') {
    if (c == EOF) {
      printf("read_list error: missing closing paren\n");
      return -1;
    }
    if (c == '(' || c == '[') {
      if ((r = readNodes(&sub_node)) < 0)
        return r;
      if (*nodes_read == NULL)
        *nodes_read = node = makeNode(NODETYPE_NODE, sub_node, NULL);
      else {
        node->next = makeNode(NODETYPE_NODE, sub_node, NULL);
        node = node->next;
      }
    }
    else {
      ungetchar(c);
      if (*nodes_read == NULL)
        *nodes_read = node = makeNode(NODETYPE_SYMBOL, readSymbol(), NULL);
      else {
        node->next = makeNode(NODETYPE_SYMBOL, readSymbol(), NULL);
        node = node->next;
      }
    }
    i++;
    eat_wspace_comment();
  }

  return i;
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
