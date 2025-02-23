#include "reccursive.h"
#include <string.h>

bool
token_consume(Token** self, char* op)
{
  if ((*self)->kind != TK_RESERVED || strlen(op) != (*self)->len ||
      memcmp((*self)->str, op, (*self)->len))
    return false;
  *self = (*self)->next;
  return true;
}

void
token_expect(Token** self, char* op)
{
  if ((*self)->kind != TK_RESERVED)
    error_at((*self)->str, "Token is not RESERVED");
  else if (strlen(op) != (*self)->len)
    error_at((*self)->str, "Token length is not %d", strlen(op));
  else if (memcmp((*self)->str, op, (*self)->len)) {
    char* token_str = calloc((*self)->len + 1, sizeof(char));
    strncpy(token_str, (*self)->str, (*self)->len);
    token_str[(*self)->len] = '\0';
    error_at((*self)->str, "Token is not '%s', but '%s'", op, token_str);
  }
  *self = (*self)->next;
}

Token*
token_consume_ident(Token** self)
{
  if ((*self)->kind != TK_IDENT)
    return NULL;
  Token* tok = *self;
  *self = (*self)->next;
  return tok;
}

int
token_expect_number(Token** self)
{
  if ((*self)->kind != TK_NUM)
    error_at((*self)->str, "Token is not number");
  int val = (*self)->val;
  *self = (*self)->next;
  return val;
}

bool
token_at_eof(Token* self)
{
  return self->kind == TK_EOF;
}

void
token_view(Token* self)
{
  while (self->kind != TK_EOF) {
    char* token_str = calloc(self->len + 1, sizeof(char));
    strncpy(token_str, self->str, self->len);
    token_str[self->len] = '\0';
    fprintf(
      stderr, "kind: %d, val: %d, str: %s\n", self->kind, self->val, token_str);
    self = self->next;
  }
}

Token*
token_new(TokenKind kind, Token* cur, char* str, int len)
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

    if (isdigit(*p)) {
      cur = token_new(TK_NUM, cur, p, 0);
      char* q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    if (strncmp(p, "<<=", 3) == 0 || strncmp(p, ">>=", 3) == 0) {
      cur = token_new(TK_RESERVED, cur, p, 2);
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
      cur = token_new(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/%()<>=,&^|!~.[];{}", *p)) {
      cur = token_new(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !isalnum(p[2])) {
      cur = token_new(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "for", 3) == 0 && !isalnum(p[3])) {
      cur = token_new(TK_RESERVED, cur, p, 3);
      p += 3;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !isalnum(p[4])) {
      cur = token_new(TK_RESERVED, cur, p, 4);
      p += 4;
      continue;
    }

    if (strncmp(p, "while", 5) == 0 && !isalnum(p[5])) {
      cur = token_new(TK_RESERVED, cur, p, 5);
      p += 5;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !isalnum(p[6])) {
      cur = token_new(TK_RESERVED, cur, p, 6);
      p += 6;
      continue;
    }

    if (isalpha(*p)) {
      char* q = p;
      while (isalnum(*p))
        p++;
      cur = token_new(TK_IDENT, cur, q, p - q);
      continue;
    }

    error_at(p, "invalid token");
  }

  token_new(TK_EOF, cur, p, 0);
  return head.next;
}