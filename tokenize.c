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

    if (isalpha(*p)) {
      if ((strncmp(p, "if", 2) == 0 || strncmp(p, "do", 2) == 0) &&
          !isalnum(p[2])) {
        cur = token_new(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }

      if ((strncmp(p, "for", 3) == 0 || strncmp(p, "int", 3) == 0) &&
          !isalnum(p[3])) {
        cur = token_new(TK_RESERVED, cur, p, 3);
        p += 3;
        continue;
      }

      if ((strncmp(p, "else", 4) == 0 || strncmp(p, "auto", 4) == 0 ||
           strncmp(p, "case", 4) == 0 || strncmp(p, "char", 4) == 0 ||
           strncmp(p, "enum", 4) == 0 || strncmp(p, "goto", 4) == 0 ||
           strncmp(p, "long", 4) == 0 || strncmp(p, "void", 4) == 0) &&
          !isalnum(p[4])) {
        cur = token_new(TK_RESERVED, cur, p, 4);
        p += 4;
        continue;
      }

      if ((strncmp(p, "break", 5) == 0 || strncmp(p, "const", 5) == 0 ||
           strncmp(p, "float", 5) == 0 || strncmp(p, "short", 5) == 0 ||
           strncmp(p, "union", 5) == 0 || strncmp(p, "while", 5) == 0) &&
          !isalnum(p[5])) {
        cur = token_new(TK_RESERVED, cur, p, 5);
        p += 5;
        continue;
      }

      if ((strncmp(p, "double", 6) == 0 || strncmp(p, "extern", 6) == 0 ||
           strncmp(p, "return", 6) == 0 || strncmp(p, "signed", 6) == 0 ||
           strncmp(p, "sizeof", 6) == 0 || strncmp(p, "static", 6) == 0 ||
           strncmp(p, "struct", 6) == 0 || strncmp(p, "switch", 6) == 0) &&
          !isalnum(p[6])) {
        cur = token_new(TK_RESERVED, cur, p, 6);
        p += 6;
        continue;
      }

      if ((strncmp(p, "default", 7) == 0 || strncmp(p, "typedef", 7) == 0) &&
          !isalnum(p[7])) {
        cur = token_new(TK_RESERVED, cur, p, 7);
        p += 7;
        continue;
      }

      if ((strncmp(p, "continue", 8) == 0 || strncmp(p, "register", 8) == 0 ||
           strncmp(p, "unsigned", 8) == 0 || strncmp(p, "volatile", 8) == 0) &&
          !isalnum(p[8])) {
        cur = token_new(TK_RESERVED, cur, p, 8);
        p += 8;
        continue;
      }

      char* q = p;
      while (isalnum(*p))
        p++;
      cur = token_new(TK_IDENT, cur, q, p - q);
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

    error_at(p, "invalid token");
  }

  token_new(TK_EOF, cur, p, 0);
  return head.next;
}