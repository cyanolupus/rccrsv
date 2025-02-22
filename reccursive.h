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
token_consume(Token** self, char* op);

void
token_expect(Token** self, char* op);

Token*
token_consume_ident(Token** self);

int
token_expect_number(Token** self);

bool
token_at_eof(Token* self);

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
  ND_FOR,
  ND_BLOCK,
  ND_CALL,
  ND_FUNC,
} NodeKind;

typedef struct LVar LVar;

struct LVar {
    LVar* next;
    char* name;
    int len;
    int offset;
};

typedef struct Node Node;

struct Node
{
  NodeKind kind;
  Node* lhs;
  Node* rhs;
  int val;
  Node* rrhs;
  Node* rrrhs;
  LVar* lvar;
  Node *argv;
  int argc;
};

Node*
node_new_lvar(LVar* lvar);

Node*
expr(Token** self);


struct Program
{
  Node* code[100];
    int len;
  LVar* locals;
};

typedef struct Program Program;

Program*
new_program();

void
add_node(Program* program, Token** self);

Node*
global(Token** self);

LVar*
new_lvar(char* name, int len, int offset);

void
add_lvar(LVar* lvar);

LVar*
find_lvar(Token* tok);

LVar*
find_or_new_lvar(Token* tok);

Program* program;

// codegen.c
void
gen(Node* node);

void
gen_stmt(Node* node);

// utils.c

typedef struct {
    void **data;
    size_t size;
    size_t capacity;
} Vector;

Vector *new_vector();
void vector_push(Vector *vec, void *elem);
void *vector_pop(Vector *vec);
void *vector_get(Vector *vec, size_t index);

typedef struct HashNode {
    char *key;
    void *value;
    struct HashNode *next;
} HashNode;

#define HASHMAP_SIZE 256

typedef struct {
    HashNode *buckets[HASHMAP_SIZE];
} HashMap;

void hashmap_new(HashMap *map);
void hashmap_put(HashMap *map, const char *key, void *value);
void *hashmap_get(HashMap *map, const char *key);

// error.c
void
error_at(char* loc, char* fmt, ...);

void
error(char* fmt, ...);

char *user_input;