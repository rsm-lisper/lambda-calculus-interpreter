# include <stdio.h>
# include <string.h>
# include <stdlib.h>

/*******************
      T O O L S
********************/

/***   N O D E   ***/
# include <stdio.h>
# include <stdlib.h>

enum TNodeTypes {
  NODETYPE_NODE,
  NODETYPE_SYMBOL
};

struct TNode {
  enum TNodeTypes type;
  void *data;
  struct TNode *next;
};

struct TNode *makeNode (enum TNodeTypes type, void *data, struct TNode *next)
{
  struct TNode *node = malloc(sizeof(struct TNode));
  if (node == NULL)
    return NULL;
  node->type = type;
  node->data = data;
  node->next = next;
  return node;
}

void delNodes (struct TNode *node)
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

void list_del (struct TNode *list)
{
  struct TNode *next;
  while (list != NULL) {
    next = list->next;
    free(list);
    list = next;
  }
}

struct TNode *list_reverse (struct TNode *nodes)
{
  struct TNode *rev = NULL, *n0 = nodes;
  while (nodes != NULL) {
    rev = makeNode(nodes->type, nodes->data, rev);
    nodes = nodes->next;
  }
  list_del(n0);
  return rev;
}

void dumpNode (struct TNode *node)
{
  printf("node %p:\n- data: %p\n- next: %p\n", node, node->data, node->next);
}

void pPrintNode (struct TNode *node);

void pprintNode (struct TNode *node)
{
  switch (node->type) {
  case NODETYPE_NODE:
    pPrintNode(node->data);
    break;
  case NODETYPE_SYMBOL:
    printf("\"%s\" ", (char*) node->data);
    break;
  }
  if (node->next != NULL)
    pprintNode(node->next);
}

void pPrintNode (struct TNode *node)
{
  printf("(");
  pprintNode(node);
  printf(")");
}


/***   C H A R / S T R I N G   ***/

# define ungetchar(C) ungetc((C), stdin)

char *strdup (const char *str)
{
  char *ret = malloc(strlen(str) + 1);
  strcpy(ret, str);
  return ret;
}


/***************
      L C I
****************/

# define MAXSYMBOL 256

/***   R E A D E R   ***/
char *read_symbol ()
{
  int c, i = 0;
  char symbol[MAXSYMBOL];
  while (i < MAXSYMBOL && (c = getchar()) != EOF && strchr("()[]; \t\r\n", c) == NULL)
    symbol[i++] = c;
  if (i == MAXSYMBOL)
    i--;
  else
    ungetchar(c);
  return strdup(symbol);
}

void eat_wspace_comm ()
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

struct TNode *read_list ()
{
  struct TNode *list = NULL;
  int c;
  
  eat_wspace_comm();
  while ((c = getchar()) != ')' && c != ']') {
    if (c == EOF) {
      printf("read_list error: missing closing paren");
      return NULL;
    }
    if (c == '(' || c == '[')
      list = makeNode(NODETYPE_NODE, read_list(), list);
    else {
      ungetchar(c);
      list = makeNode(NODETYPE_SYMBOL, read_symbol(), list);
    }
    eat_wspace_comm();
  }
  return list_reverse(list);
}

struct TNode *read_expr ()
{
  int c;
  eat_wspace_comm();
  c = getchar();
  if (c == ')' || c == ']') {
    printf("read_expr error: too many parens");
    return NULL;
  }
  if (c == EOF)
    return NULL;
  ungetchar(c);
  if (c == '(' || c == '[') {
    return makeNode(NODETYPE_NODE, read_list(), NULL);
  }
  else {
    return makeNode(NODETYPE_SYMBOL, read_symbol(), NULL);
  }
}

/***   E V A L U A T O R   ***/
struct TNode *eval_expr (struct TNode expr, $local_env=NULL)
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

/***   P R I N T E R   ***/
void print_expr (struct TNode *expr)
{
    /* print(is_callable($data) ? '#<lambda>' : */
    /*       (is_array($data) ? format_list($data) : */
    /*        $data)); */
  pPrintNode(expr);
}

/***   M A I N   L O O P  ***/
int main ()
{
  struct TNode *expr;
  while ((expr = read_expr()) != NULL) {
    print_expr(eval_expr(expr,
                         function (env) { return env; }));
  }
}
