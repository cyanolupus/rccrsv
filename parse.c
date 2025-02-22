#include "reccursive.h"
#include <stdio.h>
#include <stdlib.h>

Node*
node_new(NodeKind kind, Node* lhs, Node* rhs)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->val = 0;
  return node;
}

Node*
node_new_num(int val)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node*
node_new_lvar(LVar* lvar)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->val = 0;
  node->lvar = lvar;
  return node;
}

Node*
node_new_call(LVar* lvar, int argc, Node** argv)
{
  if (argc > 6)
    error("Too many arguments");
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_CALL;
  node->val = 0;
  node->lvar = lvar;
  return node;
}

Node*
node_new_func(LVar* lvar)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
  node->val = 0;
  node->lvar = lvar;
  return node;
}

Node*
node_new_triplet(NodeKind kind, Node* lhs, Node* rhs, Node* rrhs)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->rrhs = rrhs;
  node->val = 0;
  return node;
}

Node*
node_new_quadruplet(NodeKind kind,
                    Node* lhs,
                    Node* rhs,
                    Node* rrhs,
                    Node* rrrhs)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->rrhs = rrhs;
  node->rrrhs = rrrhs;
  node->val = 0;
  return node;
}

Node*
primary(Token** self)
{
  Token* tok = token_consume_ident(self);
  if (tok) {
    LVar* lvar = find_or_new_lvar(tok);
    if (token_consume(self, "(")) {
      Node* node = node_new_call(lvar, 0, NULL);
      while (!token_consume(self, ")")) {
        if (node->argc > 0)
          token_expect(self, ",");
        node->argv = realloc(node->argv, sizeof(Node*) * node->argc);
        // node->argv[node->argc++] = expr(self);
      }
      return node;
    }

    return node_new_lvar(lvar);
  }

  if (token_consume(self, "(")) {
    Node* node = expr(self);
    token_expect(self, ")");
    return node;
  }

  return node_new_num(token_expect_number(self));
}

Node*
unary(Token** self)
{
  if (token_consume(self, "+"))
    return primary(self);
  if (token_consume(self, "-"))
    return node_new(ND_SUB, node_new_num(0), primary(self));
  return primary(self);
}

Node*
mul(Token** self)
{
  Node* node = unary(self);

  for (;;) {
    if (token_consume(self, "*"))
      node = node_new(ND_MUL, node, unary(self));
    else if (token_consume(self, "/"))
      node = node_new(ND_DIV, node, unary(self));
    else
      return node;
  }
}

Node*
add(Token** self)
{
  Node* node = mul(self);

  for (;;) {
    if (token_consume(self, "+"))
      node = node_new(ND_ADD, node, mul(self));
    else if (token_consume(self, "-"))
      node = node_new(ND_SUB, node, mul(self));
    else
      return node;
  }
}

Node*
relational(Token** self)
{
  Node* node = add(self);

  for (;;) {
    if (token_consume(self, "<"))
      node = node_new(ND_LT, node, add(self));
    else if (token_consume(self, "<="))
      node = node_new(ND_LE, node, add(self));
    else if (token_consume(self, ">"))
      node = node_new(ND_LT, add(self), node);
    else if (token_consume(self, ">="))
      node = node_new(ND_LE, add(self), node);
    else
      return node;
  }
}

Node*
equality(Token** self)
{
  Node* node = relational(self);

  for (;;) {
    if (token_consume(self, "=="))
      node = node_new(ND_EQ, node, relational(self));
    else if (token_consume(self, "!="))
      node = node_new(ND_NE, node, relational(self));
    else
      return node;
  }
}

Node*
assign(Token** self)
{
  Node* node = equality(self);

  if (token_consume(self, "="))
    node = node_new(ND_ASSIGN, node, assign(self));
  return node;
}

Node*
expr(Token** self)
{
  return assign(self);
}

int jmpcnt = 0;

Node*
stmt(Token** self)
{
  if (token_consume(self, "return")) {
    Node* node = node_new(ND_RETURN, expr(self), NULL);
    token_expect(self, ";");
    return node;
  }
  if (token_consume(self, "if")) {
    token_expect(self, "(");
    Node* cond = expr(self);
    token_expect(self, ")");
    Node* then = stmt(self);
    Node* els = NULL;
    if (token_consume(self, "else"))
      els = stmt(self);
    return node_new_triplet(ND_IF, cond, then, els);
  }
  if (token_consume(self, "while")) {
    token_expect(self, "(");
    Node* cond = expr(self);
    token_expect(self, ")");
    Node* body = stmt(self);
    return node_new_quadruplet(ND_FOR, NULL, cond, NULL, body);
  }
  if (token_consume(self, "for")) {
    Node *init = NULL, *cond = NULL, *inc = NULL;
    token_expect(self, "(");
    if (!token_consume(self, ";")) {
      init = expr(self);
      token_expect(self, ";");
    }
    if (!token_consume(self, ";")) {
      cond = expr(self);
      token_expect(self, ";");
    }
    if (!token_consume(self, ")")) {
      inc = expr(self);
      token_expect(self, ")");
    }
    Node* body = stmt(self);
    return node_new_quadruplet(ND_FOR, init, cond, inc, body);
  }
  if (token_consume(self, "{")) {
    Node* node = node_new(ND_BLOCK, NULL, NULL);
    Node* cur = node;
    while (!token_consume(self, "}")) {
      cur->lhs = stmt(self);
      cur->rhs = node_new(ND_BLOCK, NULL, NULL);
      cur = cur->rhs;
    }
    return node;
  }
  Node* node = expr(self);
  token_expect(self, ";");
  return node;
}

void
add_node(Program* program, Token** self)
{
  while (!token_at_eof(*self)) {
    Token* funcname_ident = token_consume_ident(self);
    if (funcname_ident) {
      if (token_consume(self, "(")) {
        LVar* lvar = find_or_new_lvar(funcname_ident);
        int argc = 0;
        while (!token_consume(self, ")")) {
          if (argc > 0)
            token_expect(self, ",");
          Token* arg_ident = token_consume_ident(self);
          if (arg_ident) {
            LVar* lvar = find_or_new_lvar(arg_ident);
            argc++;
          }
        }

        Node* node = node_new_func(lvar);

        if (token_consume(self, ";")) {
          node->val = program->len;
          program->code[program->len++] = node;
          continue;
        }
        if (token_consume(self, "{")) {
          Node* cur = node;
          while (!token_consume(self, "}")) {
            cur->lhs = stmt(self);
            cur->rhs = node_new(ND_FUNC, NULL, NULL);
            cur = cur->rhs;
          }
        }
        node->val = program->len;
        program->code[program->len++] = node;
      }
    }
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
  program->code[0] = NULL;
  program->len = 0;
  program->locals = new_lvar(NULL, 0, 0);
  return program;
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

LVar*
find_or_new_lvar(Token* tok)
{
  LVar* lvar = find_lvar(tok);
  if (lvar == NULL) {
    lvar = new_lvar(tok->str, tok->len, program->locals->offset + 16);
    add_lvar(lvar);
  }
  return lvar;
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
