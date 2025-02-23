#include "reccursive.h"

Node*
node_new(NodeKind kind, Node* lhs, Node* rhs)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->children = vector_new();
  vector_push(node->children, lhs);
  vector_push(node->children, rhs);
  node->val = 0;
  return node;
}

Node*
node_new_num(int val)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node*
node_new_block()
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->children = vector_new();
  return node;
}

Node*
node_new_lvar(LVar* lvar)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  node->val = 0;
  node->argv = vector_new();
  vector_push(node->argv, lvar);
  return node;
}

Node*
node_new_call(LVar* lvar)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_CALL;
  node->val = 0;
  node->children = vector_new();
  node->argv = vector_new();
  vector_push(node->argv, lvar);
  return node;
}

Node*
node_new_func(LVar* lvar)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
  node->val = 0;
  node->argv = vector_new();
  vector_push(node->argv, lvar);
  node->children = vector_new();
  return node;
}

Node*
node_new_triplet(NodeKind kind, Node* lhs, Node* rhs, Node* rrhs)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->children = vector_new();
  vector_push(node->children, lhs);
  vector_push(node->children, rhs);
  vector_push(node->children, rrhs);
  return node;
}

Node*
node_new_quadruplet(NodeKind kind,
                    Node* lhs,
                    Node* rhs,
                    Node* rrhs,
                    Node* rrrhs)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->children = vector_new();
  vector_push(node->children, lhs);
  vector_push(node->children, rhs);
  vector_push(node->children, rrhs);
  vector_push(node->children, rrrhs);
  return node;
}

Node*
primary(Tokens* tokens)
{
  Token* tok = token_consume_ident(tokens);
  if (tok) {
    LVar* lvar = expect_lvar(tok);
    if (token_consume(tokens, "(")) {
      Node* node = node_new_call(lvar);
      while (!token_consume(tokens, ")")) {
        if (node->children->size > 0)
          token_expect(tokens, ",");
        vector_push(node->children, expr(tokens));
      }
      return node;
    }

    if (token_consume(tokens, "++")) {
      Node* add = node_new(ND_ADD, node_new_lvar(lvar), node_new_num(1));
      Node* assign = node_new(ND_POST_ASSIGN, node_new_lvar(lvar), add);
      return assign;
    }

    if (token_consume(tokens, "--")) {
      Node* sub = node_new(ND_SUB, node_new_lvar(lvar), node_new_num(1));
      Node* assign = node_new(ND_POST_ASSIGN, node_new_lvar(lvar), sub);
      return assign;
    }

    return node_new_lvar(lvar);
  }

  if (token_consume(tokens, "(")) {
    Node* node = expr(tokens);
    token_expect(tokens, ")");
    return node;
  }

  return node_new_num(token_expect_number(tokens));
}

Node*
unary(Tokens* tokens)
{
  if (token_consume(tokens, "++")) {
    Token* tok = token_expect_ident(tokens);
    LVar* lvar = expect_lvar(tok);
    Node* add = node_new(ND_ADD, node_new_lvar(lvar), node_new_num(1));
    Node* assign = node_new(ND_ASSIGN, node_new_lvar(lvar), add);
    return assign;
  }
  if (token_consume(tokens, "--")) {
    Token* tok = token_expect_ident(tokens);
    LVar* lvar = expect_lvar(tok);
    Node* sub = node_new(ND_SUB, node_new_lvar(lvar), node_new_num(1));
    Node* assign = node_new(ND_ASSIGN, node_new_lvar(lvar), sub);
    return assign;
  }
  if (token_consume(tokens, "+"))
    return primary(tokens);
  if (token_consume(tokens, "-"))
    return node_new(ND_SUB, node_new_num(0), primary(tokens));
  if (token_consume(tokens, "!"))
    return node_new(ND_LNOT, primary(tokens), NULL);
  if (token_consume(tokens, "~"))
    return node_new(ND_NOT, primary(tokens), NULL);
  if (token_consume(tokens, "*"))
    return node_new(ND_DEREF, unary(tokens), NULL);
  if (token_consume(tokens, "&"))
    return node_new(ND_REF, unary(tokens), NULL);
  if (token_consume(tokens, "sizeof"))
    return node_new(ND_SIZEOF, primary(tokens), NULL);
  return primary(tokens);
}

Node*
mul(Tokens* tokens)
{
  Node* node = unary(tokens);

  for (;;) {
    if (token_consume(tokens, "*"))
      node = node_new(ND_MUL, node, unary(tokens));
    else if (token_consume(tokens, "/"))
      node = node_new(ND_DIV, node, unary(tokens));
    else if (token_consume(tokens, "%"))
      node = node_new(ND_MOD, node, unary(tokens));
    else
      return node;
  }
}

Node*
add(Tokens* tokens)
{
  Node* node = mul(tokens);

  for (;;) {
    if (token_consume(tokens, "+"))
      node = node_new(ND_ADD, node, mul(tokens));
    else if (token_consume(tokens, "-"))
      node = node_new(ND_SUB, node, mul(tokens));
    else
      return node;
  }
}

Node*
shift(Tokens* tokens)
{
  Node* node = add(tokens);

  for (;;) {
    if (token_consume(tokens, "<<"))
      node = node_new(ND_SHIFT_L, node, add(tokens));
    else if (token_consume(tokens, ">>"))
      node = node_new(ND_SHIFT_R, node, add(tokens));
    else
      return node;
  }
}

Node*
relational(Tokens* tokens)
{
  Node* node = shift(tokens);

  for (;;) {
    if (token_consume(tokens, "<"))
      node = node_new(ND_LT, node, shift(tokens));
    else if (token_consume(tokens, "<="))
      node = node_new(ND_LE, node, shift(tokens));
    else if (token_consume(tokens, ">"))
      node = node_new(ND_LT, shift(tokens), node);
    else if (token_consume(tokens, ">="))
      node = node_new(ND_LE, shift(tokens), node);
    else
      return node;
  }
}

Node*
equality(Tokens* tokens)
{
  Node* node = relational(tokens);

  for (;;) {
    if (token_consume(tokens, "=="))
      node = node_new(ND_EQ, node, relational(tokens));
    else if (token_consume(tokens, "!="))
      node = node_new(ND_NE, node, relational(tokens));
    else
      return node;
  }
}

Node*
bitwise_and(Tokens* tokens)
{
  Node* node = equality(tokens);

  for (;;) {
    if (token_consume(tokens, "&"))
      node = node_new(ND_AND, node, equality(tokens));
    else
      return node;
  }
}

Node*
bitwise_xor(Tokens* tokens)
{
  Node* node = bitwise_and(tokens);

  for (;;) {
    if (token_consume(tokens, "^"))
      node = node_new(ND_XOR, node, bitwise_and(tokens));
    else
      return node;
  }
}

Node*
bitwise_or(Tokens* tokens)
{
  Node* node = bitwise_xor(tokens);

  for (;;) {
    if (token_consume(tokens, "|"))
      node = node_new(ND_OR, node, bitwise_xor(tokens));
    else
      return node;
  }
}

Node*
logical_and(Tokens* tokens)
{
  Node* node = bitwise_or(tokens);

  for (;;) {
    if (token_consume(tokens, "&&"))
      node = node_new(ND_AND, node, bitwise_or(tokens));
    else
      return node;
  }
}

Node*
logical_or(Tokens* tokens)
{
  Node* node = logical_and(tokens);

  for (;;) {
    if (token_consume(tokens, "||"))
      node = node_new(ND_OR, node, logical_and(tokens));
    else
      return node;
  }
}

Node*
conditional(Tokens* tokens)
{
  Node* node = logical_or(tokens);

  if (token_consume(tokens, "?")) {
    Node* then = conditional(tokens);
    token_expect(tokens, ":");
    Node* els = conditional(tokens);
    return node_new_triplet(ND_IF, node, then, els);
  }
  return node;
}

Node*
assign(Tokens* tokens)
{
  Node* node = conditional(tokens);

  if (token_consume(tokens, "="))
    node = node_new(ND_ASSIGN, node, assign(tokens));
  else if (token_consume(tokens, "+="))
    node = node_new(ND_ASSIGN, node, node_new(ND_ADD, node, assign(tokens)));
  else if (token_consume(tokens, "-="))
    node = node_new(ND_ASSIGN, node, node_new(ND_SUB, node, assign(tokens)));
  else if (token_consume(tokens, "*="))
    node = node_new(ND_ASSIGN, node, node_new(ND_MUL, node, assign(tokens)));
  else if (token_consume(tokens, "/="))
    node = node_new(ND_ASSIGN, node, node_new(ND_DIV, node, assign(tokens)));
  else if (token_consume(tokens, "%="))
    node = node_new(ND_ASSIGN, node, node_new(ND_MOD, node, assign(tokens)));
  else if (token_consume(tokens, "&="))
    node = node_new(ND_ASSIGN, node, node_new(ND_AND, node, assign(tokens)));
  else if (token_consume(tokens, "^="))
    node = node_new(ND_ASSIGN, node, node_new(ND_XOR, node, assign(tokens)));
  else if (token_consume(tokens, "|="))
    node = node_new(ND_ASSIGN, node, node_new(ND_OR, node, assign(tokens)));
  else if (token_consume(tokens, "<<="))
    node =
      node_new(ND_ASSIGN, node, node_new(ND_SHIFT_L, node, assign(tokens)));
  else if (token_consume(tokens, ">>="))
    node =
      node_new(ND_ASSIGN, node, node_new(ND_SHIFT_R, node, assign(tokens)));
  return node;
}

Node*
expr(Tokens* tokens)
{
  Type* type = token_consume_type(tokens);
  if (type) {
    Token* tok = token_expect_ident(tokens);
    LVar* lvar = add_lvar(tok, type, false);
    Node* node = node_new_lvar(lvar);
    if (token_consume(tokens, "=")) {
      node = node_new(ND_ASSIGN, node, expr(tokens));
    }
    return node;
  }
  return assign(tokens);
}

Node*
stmt(Tokens* tokens)
{
  if (token_consume(tokens, "return")) {
    Node* node = node_new(ND_RETURN, expr(tokens), NULL);
    token_expect(tokens, ";");
    return node;
  }
  if (token_consume(tokens, "if")) {
    token_expect(tokens, "(");
    Node* cond = expr(tokens);
    token_expect(tokens, ")");
    Node* then = stmt(tokens);
    Node* els = NULL;
    if (token_consume(tokens, "else"))
      els = stmt(tokens);
    return node_new_triplet(ND_IF, cond, then, els);
  }
  if (token_consume(tokens, "while")) {
    token_expect(tokens, "(");
    Node* cond = NULL;
    if (!token_consume(tokens, ")")) {
      cond = expr(tokens);
      token_expect(tokens, ")");
    }
    Node* body = stmt(tokens);
    return node_new_quadruplet(ND_FOR, NULL, cond, NULL, body);
  }
  if (token_consume(tokens, "for")) {
    Node *init = NULL, *cond = NULL, *inc = NULL;
    token_expect(tokens, "(");
    if (!token_consume(tokens, ";")) {
      init = expr(tokens);
      token_expect(tokens, ";");
    }
    if (!token_consume(tokens, ";")) {
      cond = expr(tokens);
      token_expect(tokens, ";");
    }
    if (!token_consume(tokens, ")")) {
      inc = expr(tokens);
      token_expect(tokens, ")");
    }
    Node* body = stmt(tokens);
    return node_new_quadruplet(ND_FOR, init, cond, inc, body);
  }
  if (token_consume(tokens, "{")) {
    Node* node = node_new_block();
    while (!token_consume(tokens, "}")) {
      vector_push(node->children, stmt(tokens));
    }
    return node;
  }
  Node* node = expr(tokens);
  token_expect(tokens, ";");
  return node;
}

void
add_node(Program* program, Tokens* tokens)
{
  while (!token_at_eof(tokens)) {
    Type* type = token_expect_type(tokens);

    Token* funcname_ident = token_consume_ident(tokens);
    if (funcname_ident) {
      LVar* lvar = add_lvar(funcname_ident, type, true);
      Node* node = node_new_func(lvar);

      if (token_consume(tokens, "(")) {
        while (!token_consume(tokens, ")")) {
          if (node->argv->size > 1)
            token_expect(tokens, ",");
          Type* arg_type = token_expect_type(tokens);
          Token* arg_ident = token_consume_ident(tokens);
          if (arg_ident) {
            LVar* lvar = add_lvar(arg_ident, arg_type, false);
            vector_push(node->argv, lvar);
          }
        }

        if (token_consume(tokens, ";")) {
          continue;
        }
        if (token_consume(tokens, "{")) {
          while (!token_consume(tokens, "}")) {
            vector_push(node->children, stmt(tokens));
          }
        }
        node->val = program->code->size;
        vector_push(program->code, node);
      }
    }
  }
}

LVar*
lvar_new(char* name, int len, int offset, Type* type, bool is_func)
{
  LVar* lvar = calloc(1, sizeof(LVar));
  lvar->name = to_string(name, len);
  lvar->offset = offset;
  lvar->type = type;
  lvar->is_func = is_func;
  return lvar;
}

Program*
program_new()
{
  Program* program = calloc(1, sizeof(Program));
  program->code = vector_new();
  program->locals = hashmap_new();
  return program;
}

LVar*
find_lvar(Token* tok)
{
  String* str = to_string(tok->str, tok->len);
  return hashmap_get(program->locals, string_as_cstring(str));
}

LVar*
expect_lvar(Token* tok)
{
  LVar* lvar = find_lvar(tok);
  if (!lvar) {
    error_at_until(
      tok->str, tok->len, "undefined variable: %.*s", tok->len, tok->str);
  }
  return lvar;
}

LVar*
add_lvar(Token* tok, Type* type, bool is_func)
{
  LVar* lvar = find_lvar(tok);
  if (lvar == NULL) {
    lvar =
      lvar_new(tok->str, tok->len, program->latest_offset + 8, type, is_func);
    program->latest_offset += 8;
    hashmap_put(program->locals, string_as_cstring(lvar->name), lvar);
  }
  if (lvar->type->kind != type->kind) {
    error_at_until(tok->str,
                   tok->len,
                   "conflicting types for '%.*s' %s vs %s",
                   tok->len,
                   tok->str,
                   type_to_string(lvar->type),
                   type_to_string(type));
  }
  if (lvar->is_func && !is_func) {
    error_at_until(tok->str,
                   tok->len,
                   "redefinition function of '%.*s'",
                   tok->len,
                   tok->str);
  }
  return lvar;
}

void
view_node(Node* node, int depth)
{
  if (node == NULL)
    return;

  for (int i = 0; i < depth - 1; i++)
    fprintf(stderr, "   ");
  if (depth > 0)
    fprintf(stderr, " |-");
  switch (node->kind) {
    case ND_ADD:
      fprintf(stderr, "[+]\n");
      break;
    case ND_SUB:
      fprintf(stderr, "[-]\n");
      break;
    case ND_MUL:
      fprintf(stderr, "[*]\n");
      break;
    case ND_DIV:
      fprintf(stderr, "[/]\n");
      break;
    case ND_EQ:
      fprintf(stderr, "[==]\n");
      break;
    case ND_NE:
      fprintf(stderr, "[!=]\n");
      break;
    case ND_LT:
      fprintf(stderr, "[<]\n");
      break;
    case ND_LE:
      fprintf(stderr, "[<=]\n");
      break;
    case ND_NUM:
      fprintf(stderr, "%d\n", node->val);
      break;
    default:
      break;
  }
  for (int i = 0; i < node->children->size; i++) {
    view_node(vector_get(node->children, i), depth + 1);
  }
}
