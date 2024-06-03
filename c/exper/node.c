# include <stdio.h>
# include <stdlib.h>

enum TNodeTypes {
  NODETYPE_NODE,
  NODETYPE_STRING
};

struct TNode {
  enum TNodeTypes type;
  union {
    struct TNode *node;
    char *string;
  } data;
  struct TNode *next;
};

struct TNode *makeNode (enum TNodeTypes type, void *data, struct TNode *next)
{
  struct TNode *node = malloc(sizeof(struct TNode));
  if (node == NULL)
    return NULL;
  node->type = type;
  switch (type) {
  case NODETYPE_NODE:
    node->data.node = data;
    break;
  case NODETYPE_STRING:
    node->data.string = data;
    break;
  }
  node->next = next;
  return node;
}

void delNode (struct TNode *node)
{
  if (node == NULL)
    return;
  if (node->next != NULL)
    delNode(node->next);
  if (node->type == NODETYPE_NODE)
    delNode(node->data.node);
  free(node);
}

void printNode (struct TNode *node)
{
  printf("node %p:\n- data: %s\n- next: %p\n", node, node->data.string, node->next);
}

void pPrintNode (struct TNode *node);

void pprintNode (struct TNode *node)
{
  switch (node->type) {
  case NODETYPE_NODE:
    pPrintNode(node->data.node);
    break;
  case NODETYPE_STRING:
    printf("\"%s\" ", node->data.string);
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

struct TNode *readExpr ()
{
  int c;
  while ((c = getchar()) != EOF) {
    
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  }
}

int main ()
{
  struct TNode *n[3];
  char a[] = "1", b[] = "22";
  n[0] = makeNode(NODETYPE_STRING, &b, NULL);
  n[1] = makeNode(NODETYPE_STRING, &a, NULL);
  n[2] = makeNode(NODETYPE_NODE, n[0], n[1]);
  pPrintNode(n[2]);
  printNode(n[0]);
  printNode(n[1]);
  delNode(n[0]);
  pPrintNode(makeNode(NODETYPE_STRING, "one", makeNode(NODETYPE_STRING, "two", NULL)));
  return 0;
}
