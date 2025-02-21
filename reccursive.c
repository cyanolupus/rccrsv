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
  int len;
};

bool
consume(Token** self, char* op)
{
  if ((*self)->kind != TK_RESERVED || strlen(op) != (*self)->len ||
      memcmp((*self)->str, op, (*self)->len))
    return false;
  *self = (*self)->next;
  return true;
}

void
expect(Token** self, char* op)
{
  if ((*self)->kind != TK_RESERVED)
    error_at((*self)->str, "Token is not RESERVED");
  else if (strlen(op) != (*self)->len)
    error_at((*self)->str, "Token length is not %d", strlen(op));
  else if (memcmp((*self)->str, op, (*self)->len)) {
    char* token_str = calloc(1, (*self)->len + 1);
    strncpy(token_str, (*self)->str, (*self)->len);
    token_str[(*self)->len] = '\0';
    error_at((*self)->str, "Token is not '%s', but '%s'", op, token_str);
  }
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
    char* token_str = calloc(1, self->len + 1);
    strncpy(token_str, self->str, self->len);
    token_str[self->len] = '\0';
    fprintf(
      stderr, "kind: %d, val: %d, str: %s\n", self->kind, self->val, token_str);
    self = self->next;
  }
}

Token*
new_token(TokenKind kind, Token* cur, char* str, int len)
{
  Token* tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
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

    if (strncmp(p, "<<=", 3) == 0 || strncmp(p, ">>=", 3) == 0) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 ||
        strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0 ||
        strncmp(p, "+=", 2) == 0 || strncmp(p, "-=", 2) == 0 ||
        strncmp(p, "*=", 2) == 0 || strncmp(p, "/=", 2) == 0 ||
        strncmp(p, "%=", 2) == 0 || strncmp(p, "&=", 2) == 0 ||
        strncmp(p, "^=", 2) == 0 || strncmp(p, "|=", 2) == 0 ||
        strncmp(p, "?:", 2) == 0 || strncmp(p, "->", 2) == 0 ||
        strncmp(p, "||", 2) == 0 || strncmp(p, "&&", 2) == 0 ||
        strncmp(p, ">>", 2) == 0 || strncmp(p, "<<", 2) == 0 ||
        strncmp(p, "++", 2) == 0 || strncmp(p, "--", 2) == 0) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/%()<>=,&^|!~.[]", *p)) {
      cur = new_token(TK_RESERVED, cur, p, 1);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char* q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

typedef enum
{
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
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
gen(Node* node)
{
  if (node->kind == ND_NUM) {
    printf("  mov x9, %d\n", node->val);
    printf("  str x9, [sp, #-16]!\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  ldr x1, [sp], #16\n");
  printf("  ldr x0, [sp], #16\n");

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
    case ND_EQ:
      printf("  cmp x0, x1\n");
      printf("  cset x0, EQ\n");
      break;
    case ND_NE:
      printf("  cmp x0, x1\n");
      printf("  cset x0, NE\n");
      break;
    case ND_LE:
      printf("  cmp x0, x1\n");
      printf("  cset x0, LE\n");
      break;
    case ND_LT:
      printf("  cmp x0, x1\n");
      printf("  cset x0, LT\n");
      break;
    default:
      error("NodeKind is not supported %d", node->kind);
  }

  printf("  str x0, [sp, #-16]!\n");
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