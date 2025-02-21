#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

char* user_input;

void
error(char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void
error_at(char* loc, char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

typedef struct Token Token;

struct Token
{
  TokenKind kind;
  Token* next;
  int val;
  char* str;
};

bool
consume(Token** self, char op)
{
  if ((*self)->kind != TK_RESERVED || (*self)->str[0] != op)
    return false;
  *self = (*self)->next;
  return true;
}

void
expect(Token** self, char op)
{
  if ((*self)->kind != TK_RESERVED || (*self)->str[0] != op)
    error_at((*self)->str, "Token is not '%c', but '%c'", op, (*self)->str[0]);
  *self = (*self)->next;
}

int
expect_number(Token** self)
{
  if ((*self)->kind != TK_NUM)
    error_at((*self)->str, "Token is not number");
  int val = (*self)->val;
  *self = (*self)->next;
  return val;
}

bool
at_eof(Token* self)
{
  return self->kind == TK_EOF;
}

void
view_token(Token* self)
{
  while (self->kind != TK_EOF) {
    printf("kind: %d, val: %d, str: %s, char: %c\n",
           self->kind,
           self->val,
           self->str,
           self->str[0]);
    self = self->next;
  }
}

Token*
new_token(TokenKind kind, Token* cur, char* str)
{
  Token* tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Token*
tokenize(char* p)
{
  Token head;
  head.next = NULL;
  Token* cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

typedef enum
{
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node
{
  NodeKind kind;
  Node* lhs;
  Node* rhs;
  int val;
};

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
mul(Token** self);
Node*
primary(Token** self);

Node*
expr(Token** self)
{
  Node* node = mul(self);

  for (;;) {
    if (consume(self, '+'))
      node = new_node(ND_ADD, node, mul(self), 0);
    else if (consume(self, '-'))
      node = new_node(ND_SUB, node, mul(self), 0);
    else
      return node;
  }
}

Node*
mul(Token** self)
{
  Node* node = primary(self);

  for (;;) {
    if (consume(self, '*'))
      node = new_node(ND_MUL, node, primary(self), 0);
    else if (consume(self, '/'))
      node = new_node(ND_DIV, node, primary(self), 0);
    else
      return node;
  }
}

Node*
primary(Token** self)
{
  if (consume(self, '(')) {
    Node* node = expr(self);
    expect(self, ')');
    return node;
  }

  return new_node_num(expect_number(self));
}

void
gen(Node* node)
{
  if (node->kind == ND_NUM) {
    printf("  mov x9, %d\n", node->val);
    printf("  str x9, [sp, #-16]!\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  ldr x0, [sp], #16\n");
  printf("  ldr x1, [sp], #16\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add x0, x0, x1\n");
      break;
    case ND_SUB:
      printf("  sub x0, x0, x1\n");
      break;
    case ND_MUL:
      printf("  mul x0, x0, x1\n");
      break;
    case ND_DIV:
      printf("  sdiv x0, x0, x1\n");
      break;
  }

  printf("  str x0, [sp, #-16]!\n");
}

int
main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Usage: %s <expr>\n", argv[0]);
    return 1;
  }

  Token* token;
  token = tokenize(argv[1]);
  user_input = argv[1];

  Node* node = expr(&token);

  printf(".global _main\n");
  printf("_main:\n");

  gen(node);

  printf("  ldr x0, [sp], #16\n");
  printf("  ret\n");
  return 0;
}