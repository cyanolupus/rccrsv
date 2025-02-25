#include "reccursive.h"

Node*
node_new(NodeKind kind, Node* lhs, Node* rhs, Type* type)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->children = vector_new();
  vector_push(node->children, lhs);
  vector_push(node->children, rhs);
  node->type = type;
  return node;
}

Node*
node_new_ltyped(NodeKind kind, Node* lhs, Node* rhs)
{
  return node_new(kind, lhs, rhs, lhs->type);
}

Node*
node_new_rtyped(NodeKind kind, Node* lhs, Node* rhs)
{
  return node_new(kind, lhs, rhs, rhs->type);
}

Node*
node_new_int(int val)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = type_new_int();
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
  node->type = lvar->type;
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
  node->type = lvar->type;
  return node;
}

Node*
node_new_func()
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
  node->val = 0;
  node->argv = vector_new();
  node->children = vector_new();
  return node;
}

Node*
node_new_triplet(NodeKind kind, Node* lhs, Node* rhs, Node* rrhs, Type* type)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->children = vector_new();
  vector_push(node->children, lhs);
  vector_push(node->children, rhs);
  vector_push(node->children, rrhs);
  node->type = type;
  return node;
}

Node*
node_new_quadruplet(NodeKind kind,
                    Node* lhs,
                    Node* rhs,
                    Node* rrhs,
                    Node* rrrhs,
                    Type* type)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->children = vector_new();
  vector_push(node->children, lhs);
  vector_push(node->children, rhs);
  vector_push(node->children, rrhs);
  vector_push(node->children, rrrhs);
  node->type = type;
  return node;
}

void
node_view_tree(Node* node, size_t depth)
{
  if (!node)
    return;
  for (int i = 0; i < depth; i++)
    fprintf(stderr, "  ");
  fprintf(stderr, "kind: %03d\n", node->kind);
  for (int i = 0; i < node->children->size; i++) {
    node_view_tree(vector_get_node(node->children, i), depth + 1);
  }
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
      Node* add = node_new_ltyped(ND_ADD, node_new_lvar(lvar), node_new_int(1));
      Node* assign = node_new_ltyped(ND_POST_ASSIGN, node_new_lvar(lvar), add);
      return assign;
    }

    if (token_consume(tokens, "--")) {
      Node* sub = node_new_ltyped(ND_SUB, node_new_lvar(lvar), node_new_int(1));
      Node* assign = node_new_ltyped(ND_POST_ASSIGN, node_new_lvar(lvar), sub);
      return assign;
    }

    return node_new_lvar(lvar);
  }

  if (token_consume(tokens, "(")) {
    Node* node = expr(tokens);
    token_expect(tokens, ")");
    return node;
  }

  return node_new_int(token_expect_number(tokens));
}

Node*
unary(Tokens* tokens)
{
  if (token_consume(tokens, "++")) {
    Token* tok = token_expect_ident(tokens);
    LVar* lvar = expect_lvar(tok);
    Node* add = node_new_ltyped(ND_ADD, node_new_lvar(lvar), node_new_int(1));
    Node* assign = node_new_ltyped(ND_ASSIGN, node_new_lvar(lvar), add);
    return assign;
  }
  if (token_consume(tokens, "--")) {
    Token* tok = token_expect_ident(tokens);
    LVar* lvar = expect_lvar(tok);
    Node* sub = node_new_ltyped(ND_SUB, node_new_lvar(lvar), node_new_int(1));
    Node* assign = node_new_ltyped(ND_ASSIGN, node_new_lvar(lvar), sub);
    return assign;
  }
  if (token_consume(tokens, "+"))
    return primary(tokens);
  if (token_consume(tokens, "-"))
    return node_new_ltyped(ND_SUB, node_new_int(0), primary(tokens));
  if (token_consume(tokens, "!"))
    return node_new_ltyped(ND_LNOT, primary(tokens), NULL);
  if (token_consume(tokens, "~"))
    return node_new_ltyped(ND_NOT, primary(tokens), NULL);
  if (token_consume(tokens, "*")) {
    Node* node = unary(tokens);
    return node_new(ND_DEREF, node, NULL, node->type->ptr_to);
  }
  if (token_consume(tokens, "&")) {
    Node* node = unary(tokens);
    return node_new(ND_REF, node, NULL, type_new_ptr(node->type));
  }
  if (token_consume(tokens, "sizeof"))
    return node_new(ND_SIZEOF, unary(tokens), NULL, type_new_int());
  return primary(tokens);
}

Node*
mul(Tokens* tokens)
{
  Node* node = unary(tokens);

  for (;;) {
    if (token_consume(tokens, "*"))
      node = node_new_ltyped(ND_MUL, node, unary(tokens));
    else if (token_consume(tokens, "/"))
      node = node_new_ltyped(ND_DIV, node, unary(tokens));
    else if (token_consume(tokens, "%"))
      node = node_new_ltyped(ND_MOD, node, unary(tokens));
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
      node = node_new_ltyped(ND_ADD, node, mul(tokens));
    else if (token_consume(tokens, "-"))
      node = node_new_ltyped(ND_SUB, node, mul(tokens));
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
      node = node_new_ltyped(ND_SHIFT_L, node, add(tokens));
    else if (token_consume(tokens, ">>"))
      node = node_new_ltyped(ND_SHIFT_R, node, add(tokens));
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
      node = node_new(ND_LT, node, shift(tokens), type_new_int());
    else if (token_consume(tokens, "<="))
      node = node_new(ND_LE, node, shift(tokens), type_new_int());
    else if (token_consume(tokens, ">"))
      node = node_new(ND_LT, shift(tokens), node, type_new_int());
    else if (token_consume(tokens, ">="))
      node = node_new(ND_LE, shift(tokens), node, type_new_int());
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
      node = node_new(ND_EQ, node, relational(tokens), type_new_int());
    else if (token_consume(tokens, "!="))
      node = node_new(ND_NE, node, relational(tokens), type_new_int());
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
      node = node_new_ltyped(ND_AND, node, equality(tokens));
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
      node = node_new_ltyped(ND_XOR, node, bitwise_and(tokens));
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
      node = node_new_ltyped(ND_OR, node, bitwise_xor(tokens));
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
      node = node_new_ltyped(ND_AND, node, bitwise_or(tokens));
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
      node = node_new_ltyped(ND_OR, node, logical_and(tokens));
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
    return node_new_triplet(ND_IF, node, then, els, then->type);
  }
  return node;
}

Node*
assign(Tokens* tokens)
{
  Node* node = conditional(tokens);

  if (token_consume(tokens, "="))
    node = node_new_ltyped(ND_ASSIGN, node, assign(tokens));
  else if (token_consume(tokens, "+="))
    node = node_new_ltyped(
      ND_ASSIGN, node, node_new_ltyped(ND_ADD, node, assign(tokens)));
  else if (token_consume(tokens, "-="))
    node = node_new_ltyped(
      ND_ASSIGN, node, node_new_ltyped(ND_SUB, node, assign(tokens)));
  else if (token_consume(tokens, "*="))
    node = node_new_ltyped(
      ND_ASSIGN, node, node_new_ltyped(ND_MUL, node, assign(tokens)));
  else if (token_consume(tokens, "/="))
    node = node_new_ltyped(
      ND_ASSIGN, node, node_new_ltyped(ND_DIV, node, assign(tokens)));
  else if (token_consume(tokens, "%="))
    node = node_new_ltyped(
      ND_ASSIGN, node, node_new_ltyped(ND_MOD, node, assign(tokens)));
  else if (token_consume(tokens, "&="))
    node = node_new_ltyped(
      ND_ASSIGN, node, node_new_ltyped(ND_AND, node, assign(tokens)));
  else if (token_consume(tokens, "^="))
    node = node_new_ltyped(
      ND_ASSIGN, node, node_new_ltyped(ND_XOR, node, assign(tokens)));
  else if (token_consume(tokens, "|="))
    node = node_new_ltyped(
      ND_ASSIGN, node, node_new_ltyped(ND_OR, node, assign(tokens)));
  else if (token_consume(tokens, "<<="))
    node = node_new_ltyped(
      ND_ASSIGN, node, node_new_ltyped(ND_SHIFT_L, node, assign(tokens)));
  else if (token_consume(tokens, ">>="))
    node = node_new_ltyped(
      ND_ASSIGN, node, node_new_ltyped(ND_SHIFT_R, node, assign(tokens)));
  return node;
}

Node*
expr(Tokens* tokens)
{
  Type* type = token_consume_type(tokens);
  if (type) {
    Token* tok = token_expect_ident(tokens);
    LVar* lvar = add_lvar(tok, type);
    if (token_consume(tokens, "=")) {
      Node* node =
        node_new_ltyped(ND_ASSIGN, node_new_lvar(lvar), assign(tokens));
      return node;
    } else {
      return NULL;
    }
  }
  if (token_consume(tokens, "return")) {
    Node* node = node_new_ltyped(ND_RETURN, expr(tokens), NULL);
    return node;
  }
  return assign(tokens);
}

Node*
stmt(Tokens* tokens)
{
  if (token_consume(tokens, "if")) {
    token_expect(tokens, "(");
    Node* cond = expr(tokens);
    token_expect(tokens, ")");
    Node* then = stmt(tokens);
    Node* els = NULL;
    if (token_consume(tokens, "else"))
      els = stmt(tokens);
    return node_new_triplet(ND_IF, cond, then, els, NULL);
  }
  if (token_consume(tokens, "while")) {
    token_expect(tokens, "(");
    Node* cond = NULL;
    if (!token_consume(tokens, ")")) {
      cond = expr(tokens);
      token_expect(tokens, ")");
    }
    Node* body = stmt(tokens);
    return node_new_quadruplet(ND_FOR, NULL, cond, NULL, body, NULL);
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
    return node_new_quadruplet(ND_FOR, init, cond, inc, body, NULL);
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

Node*
global(Tokens* tokens)
{
  Type* type = token_consume_type(tokens);
  Token* ident = token_consume_ident(tokens);
  if (ident) {
    Node* node;

    if (token_consume(tokens, "(")) {
      Type* func_type = type_new_func(type);
      node = node_new_func();
      while (!token_consume(tokens, ")")) {
        if (func_type->args->size > 0)
          token_expect(tokens, ",");
        if (token_consume(tokens, "void")) {
          break;
        }
        Type* arg_type = token_expect_type(tokens);
        vector_push(func_type->args, arg_type);
        Token* arg_ident = token_expect_ident(tokens);
        LVar* lvar = add_lvar(arg_ident, arg_type);
        vector_push(node->argv, lvar);
      }

      LVar* lvar = add_lvar(ident, func_type);
      node->type = func_type;
      vector_push(node->argv, lvar);

      if (token_consume(tokens, ";")) {
        return NULL;
      }

      if (token_consume(tokens, "{")) {
        while (!token_consume(tokens, "}")) {
          vector_push(node->children, stmt(tokens));
        }
      }
      node->val = program->code->size;
      return node;
    } else if (token_consume(tokens, "=")) {
      node = node_new_ltyped(
        ND_ASSIGN, node_new_lvar(add_lvar(ident, type)), expr(tokens));
    } else {
      add_lvar(ident, type);
      return NULL;
    }
  }

  return NULL;
}

void
add_node(Program* program, Tokens* tokens)
{
  while (!token_at_eof(tokens)) {
    Node* node = global(tokens);
    if (node) {
      vector_push(program->code, node);
    }
  }
}

LVar*
lvar_new(char* name, int len, int offset, Type* type)
{
  LVar* lvar = calloc(1, sizeof(LVar));
  lvar->name = string_new_with_len(name, len);
  lvar->offset = offset;
  lvar->type = type;
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
  String* str = string_new_with_len(tok->str, tok->len);
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
add_lvar(Token* tok, Type* type)
{
  LVar* lvar = find_lvar(tok);
  if (lvar == NULL) {
    lvar = lvar_new(tok->str, tok->len, program->latest_offset + 8, type);
    program->latest_offset += 8;
    hashmap_put(program->locals, string_as_cstring(lvar->name), lvar);
  } else {
    if (!type_equals(lvar->type, type)) {
      error_at_until(tok->str,
                     tok->len,
                     "conflicting types for '%.*s' %s vs %s",
                     tok->len,
                     tok->str,
                     type_to_string(lvar->type)->data,
                     type_to_string(type)->data);
    }
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
