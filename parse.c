#include "reccursive.h"
#include <stdlib.h>

Node*
new_node(NodeKind kind, Node* lhs, Node* rhs)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->val = 0;
  node->offset = 0;
  return node;
}

Node*
new_node_num(int val)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->lhs = NULL;
  node->rhs = NULL;
  node->val = val;
  node->offset = 0;
  return node;
}

Node*
new_node_lvar(int offset)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->lhs = NULL;
  node->rhs = NULL;
  node->val = 0;
  node->offset = offset;
  return node;
}

Node*
primary(Token** self)
{
  Token* tok = consume_ident(self);
  if (tok) {
    LVar* lvar = find_lvar(tok);
    if (lvar == NULL) {
      lvar = new_lvar(tok->str, tok->len, program->locals->offset + 8);
      add_lvar(lvar);
    }
    return new_node_lvar(lvar->offset);
  }

  if (consume(self, "(")) {
    Node* node = expr(self);
    expect(self, ")");
    return node;
  }

  return new_node_num(expect_number(self));
}

Node*
unary(Token** self)
{
  if (consume(self, "+"))
    return primary(self);
  if (consume(self, "-"))
    return new_node(ND_SUB, new_node_num(0), primary(self));
  return primary(self);
}

Node*
mul(Token** self)
{
  Node* node = unary(self);

  for (;;) {
    if (consume(self, "*"))
      node = new_node(ND_MUL, node, unary(self));
    else if (consume(self, "/"))
      node = new_node(ND_DIV, node, unary(self));
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
      node = new_node(ND_ADD, node, mul(self));
    else if (consume(self, "-"))
      node = new_node(ND_SUB, node, mul(self));
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
      node = new_node(ND_LT, node, add(self));
    else if (consume(self, "<="))
      node = new_node(ND_LE, node, add(self));
    else if (consume(self, ">"))
      node = new_node(ND_LT, add(self), node);
    else if (consume(self, ">="))
      node = new_node(ND_LE, add(self), node);
    else
      return node;
  }
}

Node*
equality(Token** self)
{
  Node* node = relational(self);

  for (;;) {
    if (consume(self, "=="))
      node = new_node(ND_EQ, node, relational(self));
    else if (consume(self, "!="))
      node = new_node(ND_NE, node, relational(self));
    else
      return node;
  }
}

Node*
assign(Token** self)
{
  Node* node = equality(self);

  if (consume(self, "="))
    node = new_node(ND_ASSIGN, node, assign(self));
  return node;
}

Node*
expr(Token** self)
{
  return assign(self);
}

Node*
stmt(Token** self)
{
  if (consume(self, "return")) {
    Node* node = new_node(ND_RETURN, expr(self), NULL);
    expect(self, ";");
    return node;
  } else {
    Node* node = expr(self);
    expect(self, ";");
    return node;
  }
}

LVar*
new_lvar(char* name, int len, int offset)
{
  LVar* lvar = calloc(1, sizeof(LVar));
  lvar->name = name;
  lvar->len = len;
  lvar->offset = offset;
  return lvar;
}

Program*
new_program()
{
  Program* program = calloc(1, sizeof(Program));
  program->len = 0;
  program->code[0] = NULL;
  program->locals = new_lvar(NULL, 0, 0);
  return program;
}

void
add_node(Program** self, Token** tok)
{
  while (!at_eof(*tok)) {
    (*self)->code[(*self)->len++] = stmt(tok);
  }
  (*self)->code[(*self)->len] = NULL;
}

void
add_lvar(LVar* lvar)
{
  lvar->next = program->locals;
  program->locals = lvar;
}

LVar*
find_lvar(Token* tok)
{
  for (LVar* lvar = program->locals; lvar; lvar = lvar->next) {
    if (lvar->len == tok->len && !memcmp(lvar->name, tok->str, tok->len)) {
      return lvar;
    }
  }
  return NULL;
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
