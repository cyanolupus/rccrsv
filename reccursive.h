#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

int jmpcnt;
char* input_path;
char* output_path;
FILE* output_fp;

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
  ND_DECLARATION,
  ND_CAST,
} NodeKind;

typedef enum
{
  TY_PTR,
  TY_VOID,
  TY_FUNC,
  TY_ARRAY,
  TY_I8,
  TY_I16,
  TY_ISIZE,
  TY_I32,
  TY_I64,
  TY_U8,
  TY_U16,
  TY_USIZE,
  TY_U32,
  TY_U64,
  TY_DOUBLE,
  TY_FLOAT,
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

bool
isnondigit(char c);

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

bool
token_peek(Tokens* self, char* op);

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
    size_t offset;
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
  char *original_str;
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

void
node_view_tree(Node* node, size_t depth);

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
type_new_ptr(Type* ptr_to);

Type*
type_new_void();

Type*
type_new_i8();

Type*
type_new_i16();

Type*
type_new_isize();

Type*
type_new_i32();

Type*
type_new_i64();

Type*
type_new_u8();

Type*
type_new_u16();

Type*
type_new_usize();

Type*
type_new_u32();

Type*
type_new_u64();

Type*
type_new_float();

Type*
type_new_double();

Type*
type_new_func(Type* ret_type);

Type*
type_new_array(Type* ptr_to, size_t size);

size_t
type_sizeof(Type* type);

size_t
type_sizeof_aligned(Type* type);

bool
type_equals(Type* lhs, Type* rhs);

bool
type_func_equals(Type* lhs, Type* rhs);

String*
type_to_string(Type* type);

// register.c
const char*
rn(size_t n, size_t size);

// error.c
void
eprintf(char *fmt, ...);

void
error(char* fmt, ...);

void
eprintf_at(char* loc, char* fmt, ...);

void
error_at(char* loc, char* fmt, ...);

void
eprintf_at_until(char* loc, size_t len, char* fmt, ...);

void
error_at_until(char* loc, size_t len, char* fmt, ...);

char *user_input;