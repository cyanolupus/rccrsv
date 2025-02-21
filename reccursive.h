#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

int jmpcnt;

// tokenize.c
typedef enum
{
  TK_RESERVED,
  TK_IDENT,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token
{
  TokenKind kind;
  Token* next;
  int val;
  char* str;
  int len;
};

Token*
tokenize(char* p);

bool
consume(Token** self, char* op);

void
expect(Token** self, char* op);

Token*
consume_ident(Token** self);

int
expect_number(Token** self);

bool
at_eof(Token* self);

// parse.c
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
  ND_ASSIGN,
  ND_LVAR,
  ND_RETURN,
  ND_NUM,
  ND_IF,
  ND_ELSE,
  ND_WHILE,
  ND_FOR,
} NodeKind;

typedef struct Node Node;

struct Node
{
  NodeKind kind;
  Node* lhs;
  Node* rhs;
  int val;
  int offset;
  Node* rrhs;
  Node* rrrhs;
};

Node*
expr(Token** self);

typedef struct LVar LVar;

struct LVar {
    LVar* next;
    char* name;
    int len;
    int offset;
};

struct Program
{
  Node* code[100];
  LVar* locals;
  int len;
};

typedef struct Program Program;

Program*
new_program();

void
add_node(Program** self, Token** token);

LVar*
new_lvar(char* name, int len, int offset);

void
add_lvar(LVar* lvar);

LVar*
find_lvar(Token* tok);

Program* program;

// codegen.c
void
gen(Node* node);

// error.c
void
error_at(char* loc, char* fmt, ...);

void
error(char* fmt, ...);

char *user_input;