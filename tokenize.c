#include "reccursive.h"

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

Token*
consume_ident(Token** self)
{
  if ((*self)->kind != TK_IDENT)
    return NULL;
  Token* tok = *self;
  *self = (*self)->next;
  return tok;
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

    if (strchr("+-*/%()<>=,&^|!~.[];", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char* q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      cur = new_token(TK_IDENT, cur, p++, 1);
      continue;
    }

    error_at(p, "invalid token");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}