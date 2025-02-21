#include "reccursive.h"

Node*
new_node(NodeKind kind, Node* lhs, Node* rhs, int val)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->val = val;
  return node;
}

Node*
new_node_num(int val)
{
  return new_node(ND_NUM, NULL, NULL, val);
}

Node*
expr(Token** self);
Node*
equality(Token** self);
Node*
relational(Token** self);
Node*
add(Token** self);
Node*
mul(Token** self);
Node*
unary(Token** self);
Node*
primary(Token** self);

Node*
expr(Token** self)
{
  return equality(self);
}

Node*
equality(Token** self)
{
  Node* node = relational(self);

  for (;;) {
    if (consume(self, "=="))
      node = new_node(ND_EQ, node, relational(self), 0);
    else if (consume(self, "!="))
      node = new_node(ND_NE, node, relational(self), 0);
    else
      return node;
  }
}

Node*
relational(Token** self)
{
  Node* node = add(self);

  for (;;) {
    if (consume(self, "<"))
      node = new_node(ND_LT, node, add(self), 0);
    else if (consume(self, "<="))
      node = new_node(ND_LE, node, add(self), 0);
    else if (consume(self, ">"))
      node = new_node(ND_LT, add(self), node, 0);
    else if (consume(self, ">="))
      node = new_node(ND_LE, add(self), node, 0);
    else
      return node;
  }
}

Node*
add(Token** self)
{
  Node* node = mul(self);

  for (;;) {
    if (consume(self, "+"))
      node = new_node(ND_ADD, node, mul(self), 0);
    else if (consume(self, "-"))
      node = new_node(ND_SUB, node, mul(self), 0);
    else
      return node;
  }
}

Node*
mul(Token** self)
{
  Node* node = unary(self);

  for (;;) {
    if (consume(self, "*"))
      node = new_node(ND_MUL, node, unary(self), 0);
    else if (consume(self, "/"))
      node = new_node(ND_DIV, node, unary(self), 0);
    else
      return node;
  }
}

Node*
unary(Token** self)
{
  if (consume(self, "+"))
    return primary(self);
  if (consume(self, "-"))
    return new_node(ND_SUB, new_node_num(0), primary(self), 0);
  return primary(self);
}

Node*
primary(Token** self)
{
  if (consume(self, "(")) {
    Node* node = expr(self);
    expect(self, ")");
    return node;
  }

  return new_node_num(expect_number(self));
}

void
view_node(Node* node, int depth)
{
  if (node == NULL)
    return;

  for (int i = 0; i < depth - 1; i++)
    fprintf(stderr, "   ");
  if (depth > 0)
    fprintf(stderr, " |-");
  switch (node->kind) {
    case ND_ADD:
      fprintf(stderr, "[+]\n");
      break;
    case ND_SUB:
      fprintf(stderr, "[-]\n");
      break;
    case ND_MUL:
      fprintf(stderr, "[*]\n");
      break;
    case ND_DIV:
      fprintf(stderr, "[/]\n");
      break;
    case ND_EQ:
      fprintf(stderr, "[==]\n");
      break;
    case ND_NE:
      fprintf(stderr, "[!=]\n");
      break;
    case ND_LT:
      fprintf(stderr, "[<]\n");
      break;
    case ND_LE:
      fprintf(stderr, "[<=]\n");
      break;
    case ND_NUM:
      fprintf(stderr, "%d\n", node->val);
      break;
    default:
      break;
  }
  view_node(node->lhs, depth + 1);
  view_node(node->rhs, depth + 1);
}