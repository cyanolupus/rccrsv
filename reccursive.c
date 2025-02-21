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

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "cant tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
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

  printf(".global _main\n");
  printf("_main:\n");
  printf("  mov x0, %d\n", expect_number(&token));

  while (!at_eof(token)) {
    if (consume(&token, '+')) {
      printf("  add x0, x0, %d\n", expect_number(&token));
      continue;
    }

    expect(&token, '-');
    printf("  sub x0, x0, %d\n", expect_number(&token));
  }

  printf("  ret\n");
  return 0;
}