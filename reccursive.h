#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

int jmpcnt;

// typedef
typedef struct Vector Vector;
typedef struct HashNode HashNode;
typedef struct HashMap HashMap;
typedef Vector String;

typedef struct Token Token;
typedef struct LVar LVar;
typedef struct Node Node;
typedef struct Program Program;

// utils.c

struct Vector {
    void **data;
    size_t size;
    size_t capacity;
};

Vector *vector_new();
void vector_push(Vector *vec, void *elem);
void *vector_pop(Vector *vec);
void *vector_get(Vector *vec, size_t index);
Node *vector_get_node(Vector *vec, size_t index);
LVar *vector_get_lvar(Vector *vec, size_t index);

struct HashNode {
    char *key;
    void *value;
    struct HashNode *next;
};

#define HASHMAP_SIZE 256

struct HashMap {
    HashNode *buckets[HASHMAP_SIZE];
};

HashMap *hashmap_new();
void hashmap_put(HashMap *map, const char *key, void *value);
void *hashmap_get(HashMap *map, const char *key);

String *string_new();
String *to_string(char *s, size_t len);
void string_add(String *str, char *s, size_t len);
const char *string_as_cstring(String *str);

// tokenize.c
typedef enum
{
  TK_RESERVED,
  TK_IDENT,
  TK_NUM,
  TK_EOF,
} TokenKind;

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
  ND_NOT,
  ND_INV,
  ND_DEREF,
  ND_REF,
  ND_SIZEOF,
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_MOD,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
  ND_ASSIGN,
  ND_POST_ASSIGN,
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

struct LVar {
    String *name;
    int offset;
};

struct Node
{
  NodeKind kind;
  int val;
  Vector *children;
  Vector *locals;
  Vector *argv;
};

Node*
node_new_lvar(LVar* lvar);

Node*
expr(Token** self);


struct Program
{
  Vector *code;
  HashMap *locals;
  int latest_offset;
};

Program*
program_new();

void
add_node(Program* program, Token** self);

Node*
global(Token** self);

LVar*
lvar_new(char* name, int len, int offset);

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

// error.c
void
error_at(char* loc, char* fmt, ...);

void
error(char* fmt, ...);

char *user_input;