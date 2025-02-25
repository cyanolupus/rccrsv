#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

int jmpcnt;

// typedef
typedef enum
{
  TK_RESERVED,
  TK_IDENT,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef enum
{
  ND_LNOT,
  ND_NOT,
  ND_DEREF,
  ND_REF,
  ND_SIZEOF,
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_MOD,
  ND_SHIFT_L,
  ND_SHIFT_R,
  ND_AND,
  ND_XOR,
  ND_OR,
  ND_LAND,
  ND_LOR,
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

typedef enum
{
  TY_INT,
  TY_PTR,
  TY_VOID,
  TY_LONG,
  TY_CHAR,
  TY_FLOAT,
  TY_SHORT,
  TY_DOUBLE,
  TY_FUNC,
} TypeKind;

typedef struct Vector Vector;
typedef struct HashNode HashNode;
typedef struct HashMap HashMap;
typedef struct String String;

typedef struct Token Token;
typedef struct Tokens Tokens;
typedef struct LVar LVar;
typedef struct Node Node;
typedef struct Program Program;

typedef struct Type Type;

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
Token *vector_get_token(Vector *vec, size_t index);
Node *vector_get_node(Vector *vec, size_t index);
LVar *vector_get_lvar(Vector *vec, size_t index);
Type *vector_get_type(Vector *vec, size_t index);

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

struct String {
    char *data;
    size_t size;
    size_t capacity;
};

String *string_new(const char *initial);
String *string_new_with_len(char *s, size_t len);
void string_append(String *str, const char *s);
const char *string_as_cstring(String *str);

// tokenize.c
struct Token
{
  TokenKind kind;
  int val;
  char* str;
  int len;
};

struct Tokens
{
  Vector* tokens;
  int pos;
};

Tokens*
tokenize(char* p);

bool
token_consume(Tokens* self, char* op);

void
token_expect(Tokens* self, char* op);

Token*
token_consume_ident(Tokens* self);

Token*
token_expect_ident(Tokens* self);

int
token_expect_number(Tokens* self);

Type*
token_consume_type(Tokens* self);

Type*
token_expect_type(Tokens* self);

bool
token_at_eof(Tokens* self);

void
token_view(Tokens* self);

// parse.c

struct LVar {
    String *name;
    int offset;
    Type *type;
};

struct Node
{
  NodeKind kind;
  int val;
  Vector *children;
  Vector *locals;
  Vector *argv;
  Type *type;
};

Node*
node_new_lvar(LVar* lvar);

Node*
expr(Tokens* self);

struct Program
{
  Vector *code;
  HashMap *locals;
  int latest_offset;
};

Program*
program_new();

void
add_node(Program* program, Tokens* tokens);

Node*
global(Tokens* tokens);

LVar*
lvar_new(char* name, int len, int offset, Type* type);

LVar*
expect_lvar(Token* tok);

LVar*
add_lvar(Token* tok, Type* type);

Program* program;

// codegen.c
void
gen(Node* node);

void
gen_stmt(Node* node);

void
gen_expr(Node* node);

// type.c
struct Type {
  TypeKind kind;
  size_t size;
  Vector *args;
  struct Type *ptr_to;
};

Type*
type_new(TypeKind kind, Type* ptr_to);

Type*
type_new_int();

Type*
type_new_ptr(Type* ptr_to);

Type*
type_new_void();

Type*
type_new_long();

Type*
type_new_char();

Type*
type_new_float();

Type*
type_new_short();

Type*
type_new_double();

Type*
type_new_func(Type* ret_type);

size_t
type_sizeof(Type* type);

bool
type_equals(Type* lhs, Type* rhs);

bool
type_func_equals(Type* lhs, Type* rhs);

String*
type_to_string(Type* type);

// error.c
void
error_at(char* loc, char* fmt, ...);

void
error_at_until(char* loc, int len, char* fmt, ...);

void
error(char* fmt, ...);

char *user_input;