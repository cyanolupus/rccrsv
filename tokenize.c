#include "rccrsv.h"

Token*
token_new(TokenKind kind, char* str, int len)
{
  Token* tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = string_new_with_len(str, len);
  return tok;
}

Tokens*
tokens_new()
{
  Tokens* tokens = calloc(1, sizeof(Tokens));
  tokens->tokens = vector_new(512);
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

bool
token_peek(Tokens* tokens, String* op)
{
  Token* tok = tokens_peek(tokens);
  if (tok->kind != TK_RESERVED || !string_equals(tok->str, op)) {
    return false;
  }
  return true;
}

bool
token_consume(Tokens* tokens, String* op)
{
  Token* tok = tokens_peek(tokens);
  if (tok->kind != TK_RESERVED || !string_equals(tok->str, op)) {
    return false;
  }
  tokens->pos++;
  return true;
}

void
token_expect(Tokens* tokens, String* op)
{
  Token* tok = tokens_pop_front(tokens);
  if (token_consume(tokens, op)) {
    fprintf(stderr, "Token is not '%s'\n", op->data);
    exit(1);
  }
}

String*
token_consume_ident(Tokens* tokens)
{
  Token* tok = tokens_peek(tokens);
  if (tok->kind != TK_IDENT) {
    return NULL;
  }
  tokens->pos++;
  return string_clone(tok->str);
}

String*
token_expect_ident(Tokens* tokens)
{
  Token* tok = tokens_pop_front(tokens);
  if (tok->kind != TK_IDENT) {
    fprintf(stderr, "Token is not identifier\n");
    exit(1);
  }
  return string_clone(tok->str);
}

int
token_expect_number(Tokens* tokens)
{
  Token* tok = tokens_pop_front(tokens);
  if (tok->kind != TK_NUM) {
    fprintf(stderr, "Token is not number\n");
    exit(1);
  }

  return tok->val;
}

Type*
token_consume_type(Tokens* tokens)
{
  Type* type;

  if (token_consume(tokens, string_new("unsigned"))) {
    if (token_consume(tokens, string_new("long"))) {
      if (token_consume(tokens, string_new("long"))) {
        type = type_new_u64();
      }
      token_consume(tokens, string_new("int"));
      type = type_new_u32();
    } else if (token_consume(tokens, string_new("short"))) {
      token_consume(tokens, string_new("int"));
      type = type_new_u16();
    } else if (token_consume(tokens, string_new("int"))) {
      type = type_new_usize();
    } else if (token_consume(tokens, string_new("char"))) {
      type = type_new_u8();
    } else {
      return NULL;
    }
  } else if (token_consume(tokens, string_new("float"))) {
    type = type_new_float();
  } else if (token_consume(tokens, string_new("double"))) {
    type = type_new_double();
  } else {
    token_consume(tokens, string_new("signed"));
    if (token_consume(tokens, string_new("long"))) {
      if (token_consume(tokens, string_new("long"))) {
        token_consume(tokens, string_new("int"));
        type = type_new_i64();
      } else {
        token_consume(tokens, string_new("int"));
        type = type_new_i32();
      }
    } else if (token_consume(tokens, string_new("short"))) {
      token_consume(tokens, string_new("int"));
      type = type_new_i16();
    } else if (token_consume(tokens, string_new("int"))) {
      type = type_new_isize();
    } else if (token_consume(tokens, string_new("char"))) {
      type = type_new_i8();
    } else if (token_consume(tokens, string_new("void"))) {
      type = type_new_void();
    } else {
      return NULL;
    }
  }

  while (token_consume(tokens, string_new("*"))) {
    type = type_new_ptr(type);
  }

  return type;
}

Type*
token_expect_type(Tokens* tokens)
{
  Token* tok = tokens_peek(tokens);
  Type* type = token_consume_type(tokens);
  if (!type) {
    fprintf(stderr, "Token is not data type\n");
    exit(1);
  }
  return type;
}

String*
token_consume_str(Tokens* tokens)
{
  Token* tok = tokens_peek(tokens);
  if (tok->kind != TK_STR) {
    return NULL;
  }
  return string_clone(tok->str);
}

bool
token_at_eof(Tokens* tokens)
{
  return tokens_peek(tokens)->kind == TK_EOF;
}

void
token_view(Tokens* tokens)
{
  const char* st = "> ";
  for (int i = 0; i < tokens->tokens->size; i++) {
    if (i == tokens->pos)
      fprintf(stderr, "%s", st);
    else
      fprintf(stderr, "  ");
    Token* tok = vector_get_token(tokens->tokens, i);
    fprintf(stderr,
            "kind: %d, val: %lld, str: %s\n",
            tok->kind,
            tok->val,
            tok->str->data);
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
      char* q = p;
      Token* tok = token_new(TK_NUM, q, p - q);
      tok->val = strtoll(p, &p, 10);
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

    if (*p == '"') {
      char* q = ++p;
      while (*p != '"')
        p++;
      vector_push(tokens->tokens, token_new(TK_STR, q, p - q));
      p++;
      continue;
    }

    fprintf(stderr, "invalid token\n");
    exit(1);
  }

  vector_push(tokens->tokens, token_new(TK_EOF, p, 0));
  return tokens;
}