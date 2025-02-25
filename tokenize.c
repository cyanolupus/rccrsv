#include "reccursive.h"

Token*
token_new(TokenKind kind, char* str, int len)
{
  Token* tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  return tok;
}

Tokens*
tokens_new()
{
  Tokens* tokens = calloc(1, sizeof(Tokens));
  tokens->tokens = vector_new();
  tokens->pos = 0;
  return tokens;
}

Token*
tokens_peek(Tokens* tokens)
{
  return vector_get_token(tokens->tokens, tokens->pos);
}

Token*
tokens_pop_front(Tokens* tokens)
{
  return vector_get_token(tokens->tokens, tokens->pos++);
}

void
tokens_pop_front_undo(Tokens* tokens)
{
  tokens->pos--;
}

bool
token_consume(Tokens* tokens, char* op)
{
  Token* tok = tokens_pop_front(tokens);
  if (tok->kind != TK_RESERVED || strlen(op) != tok->len ||
      memcmp(tok->str, op, tok->len)) {
    tokens_pop_front_undo(tokens);
    return false;
  }
  return true;
}

void
token_expect(Tokens* tokens, char* op)
{
  Token* tok = tokens_pop_front(tokens);
  if (tok->kind != TK_RESERVED)
    error_at(tok->str, "Token is not RESERVED");
  else if (strlen(op) != tok->len)
    error_at(tok->str, "Token length is not %d, but %d", strlen(op), tok->len);
  else if (memcmp(tok->str, op, tok->len)) {
    char* token_str = calloc(tok->len + 1, sizeof(char));
    strncpy(token_str, tok->str, tok->len);
    token_str[tok->len] = '\0';
    error_at(tok->str, "Token is not '%s', but '%s'", op, token_str);
  }
}

Token*
token_consume_ident(Tokens* tokens)
{
  Token* tok = tokens_pop_front(tokens);
  if (tok->kind != TK_IDENT) {
    tokens_pop_front_undo(tokens);
    return NULL;
  }
  return tok;
}

Token*
token_expect_ident(Tokens* tokens)
{
  Token* tok = tokens_pop_front(tokens);
  if (tok->kind != TK_IDENT) {
    error_at(tok->str, "Token is not identifier");
  }
  return tok;
}

int
token_expect_number(Tokens* tokens)
{
  Token* tok = tokens_pop_front(tokens);
  if (tok->kind != TK_NUM)
    error_at(tok->str, "Token is not number");

  return tok->val;
}

bool
token_peek(Tokens* tokens, char* op)
{
  Token* tok = tokens_peek(tokens);
  if (tok->kind != TK_RESERVED || strlen(op) != tok->len ||
      memcmp(tok->str, op, tok->len)) {
    return false;
  }
  return true;
}

Type*
token_consume_type(Tokens* tokens)
{
  Type* type;
  Token* tok = tokens_pop_front(tokens);
  bool is_signed = true;
  if (tok->kind != TK_RESERVED && tok->kind != TK_IDENT) {
    tokens_pop_front_undo(tokens);
    return NULL;
  }

  if (tok->len == 8 && strncmp(tok->str, "unsigned", 8) == 0) {
    is_signed = true;
    tok = tokens_pop_front(tokens);
  } else if (tok->len == 6 && strncmp(tok->str, "signed", 6) == 0) {
    is_signed = false;
    tok = tokens_pop_front(tokens);
  }

  if (tok->len == 3 && strncmp(tok->str, "int", 3) == 0)
    type = type_new_int();
  else if (tok->len == 4 && strncmp(tok->str, "void", 4) == 0)
    type = type_new_void();
  else if (tok->len == 4 && strncmp(tok->str, "long", 4) == 0) {
    if (token_consume(tokens, "long"))
      type = type_new_long_long();
    else
      type = type_new_long();
  } else if (tok->len == 4 && strncmp(tok->str, "char", 4) == 0)
    type = type_new_char();
  else if (tok->len == 5 && strncmp(tok->str, "float", 5) == 0)
    type = type_new_float();
  else if (tok->len == 5 && strncmp(tok->str, "short", 5) == 0)
    type = type_new_short();
  else if (tok->len == 6 && strncmp(tok->str, "double", 6) == 0)
    type = type_new_double();
  else {
    tokens_pop_front_undo(tokens);
    return NULL;
  }

  while (token_consume(tokens, "*")) {
    type = type_new_ptr(type);
  }

  type->is_signed = is_signed;

  return type;
}

Type*
token_expect_type(Tokens* tokens)
{
  Type* type;
  Token* tok = tokens_pop_front(tokens);
  if (tok->kind != TK_RESERVED && tok->kind != TK_IDENT)
    error_at_until(tok->str, tok->len, "Token is not RESERVED or IDENT");

  if (tok->len == 3 && strncmp(tok->str, "int", 3) == 0)
    type = type_new_int();
  else if (tok->len == 4 && strncmp(tok->str, "void", 4) == 0)
    type = type_new_void();
  else if (tok->len == 4 && strncmp(tok->str, "long", 4) == 0)
    type = type_new_long();
  else if (tok->len == 4 && strncmp(tok->str, "char", 4) == 0)
    type = type_new_char();
  else if (tok->len == 5 && strncmp(tok->str, "float", 5) == 0)
    type = type_new_float();
  else if (tok->len == 5 && strncmp(tok->str, "short", 5) == 0)
    type = type_new_short();
  else if (tok->len == 6 && strncmp(tok->str, "double", 6) == 0)
    type = type_new_double();
  else
    error_at_until(tok->str, tok->len, "Token is not data type");

  while (token_consume(tokens, "*")) {
    type = type_new_ptr(type);
  }

  return type;
}

bool
token_at_eof(Tokens* tokens)
{
  return tokens_peek(tokens)->kind == TK_EOF;
}

void
token_view(Tokens* tokens)
{
  for (int i = 0; i < tokens->tokens->size; i++) {
    Token* tok = vector_get_token(tokens->tokens, i);
    char* token_str = calloc(tok->len + 1, sizeof(char));
    strncpy(token_str, tok->str, tok->len);
    token_str[tok->len] = '\0';
    fprintf(
      stderr, "kind: %d, val: %d, str: %s\n", tok->kind, tok->val, token_str);
  }
}

Tokens*
tokenize(char* p)
{
  Tokens* tokens = tokens_new();

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (isdigit(*p)) {
      Token* tok = token_new(TK_NUM, p, 0);
      char* q = p;
      tok->val = strtol(p, &p, 10);
      tok->len = p - q;
      vector_push(tokens->tokens, tok);
      continue;
    }

    if (isnondigit(*p)) {
      // data type
      if (strncmp(p, "int", 3) == 0 && !isalnum(p[3])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 3));
        p += 3;
        continue;
      }

      if ((strncmp(p, "void", 4) == 0 || strncmp(p, "long", 4) == 0 ||
           strncmp(p, "char", 4) == 0) &&
          !isalnum(p[4])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 4));
        p += 4;
        continue;
      }

      if ((strncmp(p, "float", 5) == 0 || strncmp(p, "short", 5) == 0) &&
          !isalnum(p[5])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 5));
        p += 5;
        continue;
      }

      if (strncmp(p, "double", 6) == 0 && !isalnum(p[6])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 6));
        p += 6;
        continue;
      }

      // storage class / modifier
      if (strncmp(p, "auto", 4) == 0 && !isalnum(p[4])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 4));
        p += 4;
        continue;
      }

      if (strncmp(p, "const", 5) == 0 && !isalnum(p[5])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 5));
        p += 5;
        continue;
      }

      if ((strncmp(p, "extern", 6) == 0 || strncmp(p, "static", 6) == 0 ||
           strncmp(p, "signed", 6) == 0) &&
          !isalnum(p[6])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 6));
        p += 6;
        continue;
      }

      if ((strncmp(p, "register", 8) == 0 || strncmp(p, "volatile", 8) == 0 ||
           strncmp(p, "unsigned", 8) == 0) &&
          !isalnum(p[8])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 8));
        p += 8;
        continue;
      }

      // control syntax
      if ((strncmp(p, "if", 2) == 0 || strncmp(p, "do", 2) == 0) &&
          !isalnum(p[2])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 2));
        p += 2;
        continue;
      }

      if (strncmp(p, "for", 3) == 0 && !isalnum(p[3])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 3));
        p += 3;
        continue;
      }

      if ((strncmp(p, "goto", 4) == 0 || strncmp(p, "else", 4) == 0 ||
           strncmp(p, "case", 4) == 0) &&
          !isalnum(p[4])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 4));
        p += 4;
        continue;
      }

      if ((strncmp(p, "break", 5) == 0 || strncmp(p, "while", 5) == 0) &&
          !isalnum(p[5])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 5));
        p += 5;
        continue;
      }

      if ((strncmp(p, "return", 6) == 0 || strncmp(p, "switch", 6) == 0) &&
          !isalnum(p[6])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 6));
        p += 6;
        continue;
      }

      if (strncmp(p, "default", 7) == 0 && !isalnum(p[7])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 7));
        p += 7;
        continue;
      }

      if (strncmp(p, "continue", 8) == 0 && !isalnum(p[8])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 8));
        p += 8;
        continue;
      }

      // type definition / type operation
      if (strncmp(p, "enum", 4) == 0 && !isalnum(p[4])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 4));
        p += 4;
        continue;
      }

      if (strncmp(p, "union", 5) == 0 && !isalnum(p[5])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 5));
        p += 5;
        continue;
      }

      if ((strncmp(p, "struct", 6) == 0 || strncmp(p, "sizeof", 6) == 0) &&
          !isalnum(p[6])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 6));
        p += 6;
        continue;
      }

      if (strncmp(p, "typedef", 7) == 0 && !isalnum(p[7])) {
        vector_push(tokens->tokens, token_new(TK_RESERVED, p, 7));
        p += 7;
        continue;
      }

      char* q = p;
      while (isnondigit(*p) || isdigit(*p))
        p++;
      vector_push(tokens->tokens, token_new(TK_IDENT, q, p - q));
      continue;
    }

    if (strncmp(p, "<<=", 3) == 0 || strncmp(p, ">>=", 3) == 0) {
      vector_push(tokens->tokens, token_new(TK_RESERVED, p, 3));
      p += 3;
      continue;
    }

    if (strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 ||
        strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0 ||
        strncmp(p, "+=", 2) == 0 || strncmp(p, "-=", 2) == 0 ||
        strncmp(p, "*=", 2) == 0 || strncmp(p, "/=", 2) == 0 ||
        strncmp(p, "%=", 2) == 0 || strncmp(p, "&=", 2) == 0 ||
        strncmp(p, "^=", 2) == 0 || strncmp(p, "|=", 2) == 0 ||
        strncmp(p, "->", 2) == 0 || strncmp(p, "||", 2) == 0 ||
        strncmp(p, "&&", 2) == 0 || strncmp(p, ">>", 2) == 0 ||
        strncmp(p, "<<", 2) == 0 || strncmp(p, "++", 2) == 0 ||
        strncmp(p, "--", 2) == 0) {
      vector_push(tokens->tokens, token_new(TK_RESERVED, p, 2));
      p += 2;
      continue;
    }

    if (strchr("+-*/%()<>=,&^|!~.[];{}:?", *p)) {
      vector_push(tokens->tokens, token_new(TK_RESERVED, p++, 1));
      continue;
    }

    error_at(p, "invalid token");
  }

  vector_push(tokens->tokens, token_new(TK_EOF, p, 0));
  return tokens;
}