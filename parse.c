#include "reccursive.h"

String*
node_kind_to_string(Node* node)
{
  switch (node->kind) {
    case ND_ADD:
      return string_new("+");
    case ND_SUB:
      return string_new("-");
    case ND_MUL:
      return string_new("*");
    case ND_DIV:
      return string_new("/");
    case ND_MOD:
      return string_new("%");
    case ND_EQ:
      return string_new("==");
    case ND_NE:
      return string_new("!=");
    case ND_LT:
      return string_new("<");
    case ND_LE:
      return string_new("<=");
    case ND_ASSIGN:
      return string_new("=");
    case ND_POST_ASSIGN:
      return string_new("=");
    case ND_LVAR: {
      char buf[255];
      snprintf(buf, 255, "%s", vector_get_lvar(node->argv, 0)->name->data);
      return string_new(buf);
    }
    case ND_RETURN:
      return string_new("return");
    case ND_NUM: {
      char buf[255];
      snprintf(buf, 255, "%d", node->val);
      return string_new(buf);
    }
    case ND_IF:
      return string_new("if");
    case ND_ELSE:
      return string_new("else");
    case ND_FOR:
      return string_new("for");
    case ND_BLOCK:
      return string_new("block");
    case ND_CALL:
      return string_new("call");
    case ND_FUNC:
      return string_new("func");
    case ND_DECLARATION:
      return string_new("declaration");
    case ND_DEREF:
      return string_new("deref");
    case ND_REF:
      return string_new("ref");
    case ND_SIZEOF:
      return string_new("sizeof");
    case ND_LNOT:
      return string_new("!");
    case ND_NOT:
      return string_new("~");
    case ND_SHIFT_L:
      return string_new("<<");
    case ND_SHIFT_R:
      return string_new(">>");
    case ND_AND:
      return string_new("&");
    case ND_XOR:
      return string_new("^");
    case ND_OR:
      return string_new("|");
    case ND_LAND:
      return string_new("&&");
    case ND_LOR:
      return string_new("||");
    case ND_CAST:
      return string_new("cast");
    default:
      return string_new("unknown");
  }
}

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
  node->type = lvar->type->ptr_to;
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
node_new_declaraion(LVar* lvar)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = ND_DECLARATION;
  node->val = 0;
  node->argv = vector_new();
  vector_push(node->argv, lvar);
  node->type = lvar->type;
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

Node*
node_new_typed(NodeKind kind, Node* lhs, Node* rhs)
{
  if (!rhs)
    return node_new(kind, lhs, NULL, lhs->type);
  if (!lhs->type)
    return node_new(kind, lhs, rhs, NULL);
  if (!rhs->type)
    return node_new(kind, lhs, rhs, lhs->type);
  if (!type_equals(lhs->type, rhs->type)) {
    switch (lhs->type->kind) {
      case TY_INT:
        if (rhs->type->kind == TY_CHAR || rhs->type->kind == TY_SHORT ||
            rhs->type->kind == TY_LONG || rhs->type->kind == TY_PTR ||
            rhs->type->kind == TY_LONG_LONG || rhs->type->kind == TY_FLOAT ||
            rhs->type->kind == TY_DOUBLE || rhs->type->kind == TY_ARRAY) {
          return node_new(
            kind, lhs, node_new(ND_CAST, rhs, NULL, lhs->type), lhs->type);
        }
        break;
      case TY_PTR:
        if (rhs->type->kind == TY_INT || rhs->type->kind == TY_CHAR ||
            rhs->type->kind == TY_SHORT || rhs->type->kind == TY_LONG ||
            rhs->type->kind == TY_LONG_LONG || rhs->type->kind == TY_FLOAT ||
            rhs->type->kind == TY_DOUBLE || rhs->type->kind == TY_ARRAY) {
          return node_new(
            kind, lhs, node_new(ND_CAST, rhs, NULL, lhs->type), lhs->type);
        }
        break;
      case TY_ARRAY:
        if (rhs->type->kind == TY_INT || rhs->type->kind == TY_CHAR ||
            rhs->type->kind == TY_SHORT || rhs->type->kind == TY_LONG ||
            rhs->type->kind == TY_LONG_LONG || rhs->type->kind == TY_FLOAT ||
            rhs->type->kind == TY_DOUBLE || rhs->type->kind == TY_PTR) {
          return node_new(
            kind, lhs, node_new(ND_CAST, rhs, NULL, lhs->type), lhs->type);
        }
        break;
      default:
        break;
    }
  }
  return node_new(kind, lhs, rhs, lhs->type);
}

Node*
node_new_assign(Node* lhs, Node* rhs)
{
  return node_new_typed(ND_ASSIGN, lhs, rhs);
}

Node*
node_new_post_assign(Node* lhs, Node* rhs)
{
  return node_new_typed(ND_POST_ASSIGN, lhs, rhs);
}

void
node_view_tree(Node* node, size_t depth)
{
  if (!node)
    return;
  for (int i = 0; i < depth; i++)
    fprintf(stderr, "  ");
  fprintf(stderr,
          "%s %s\n",
          node_kind_to_string(node)->data,
          type_to_string(node->type)->data);
  if (!node->children)
    return;
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
      Node* add = node_new_typed(ND_ADD, node_new_lvar(lvar), node_new_int(1));
      Node* assign = node_new_post_assign(node_new_lvar(lvar), add);
      return assign;
    }

    if (token_consume(tokens, "--")) {
      Node* sub = node_new_typed(ND_SUB, node_new_lvar(lvar), node_new_int(1));
      Node* assign = node_new_assign(node_new_lvar(lvar), sub);
      return assign;
    }

    Node* node = node_new_lvar(lvar);
    if (lvar->type->kind == TY_ARRAY) {
      return node_new(ND_REF, node, NULL, type_new_ptr(lvar->type->ptr_to));
    }
    return node;
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
    Node* add = node_new_typed(ND_ADD, node_new_lvar(lvar), node_new_int(1));
    Node* assign = node_new_assign(node_new_lvar(lvar), add);
    return assign;
  }
  if (token_consume(tokens, "--")) {
    Token* tok = token_expect_ident(tokens);
    LVar* lvar = expect_lvar(tok);
    Node* sub = node_new_typed(ND_SUB, node_new_lvar(lvar), node_new_int(1));
    Node* assign = node_new_assign(node_new_lvar(lvar), sub);
    return assign;
  }
  if (token_consume(tokens, "+"))
    return primary(tokens);
  if (token_consume(tokens, "-"))
    return node_new_typed(ND_SUB, node_new_int(0), primary(tokens));
  if (token_consume(tokens, "!"))
    return node_new_typed(ND_LNOT, primary(tokens), NULL);
  if (token_consume(tokens, "~"))
    return node_new_typed(ND_NOT, primary(tokens), NULL);
  if (token_consume(tokens, "*")) {
    Node* node = unary(tokens);
    if (node->type->ptr_to == NULL)
      error("Dereference of non-pointer type: %s",
            type_to_string(node->type)->data);
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
      node = node_new_typed(ND_MUL, node, unary(tokens));
    else if (token_consume(tokens, "/"))
      node = node_new_typed(ND_DIV, node, unary(tokens));
    else if (token_consume(tokens, "%"))
      node = node_new_typed(ND_MOD, node, unary(tokens));
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
      node = node_new_typed(ND_ADD, node, mul(tokens));
    else if (token_consume(tokens, "-"))
      node = node_new_typed(ND_SUB, node, mul(tokens));
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
      node = node_new_typed(ND_SHIFT_L, node, add(tokens));
    else if (token_consume(tokens, ">>"))
      node = node_new_typed(ND_SHIFT_R, node, add(tokens));
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
      node = node_new_typed(ND_AND, node, equality(tokens));
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
      node = node_new_typed(ND_XOR, node, bitwise_and(tokens));
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
      node = node_new_typed(ND_OR, node, bitwise_xor(tokens));
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
      node = node_new_typed(ND_AND, node, bitwise_or(tokens));
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
      node = node_new_typed(ND_OR, node, logical_and(tokens));
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
    node = node_new_assign(node,
                           node_new(ND_CAST, assign(tokens), NULL, node->type));
  else if (token_consume(tokens, "+="))
    node = node_new_assign(node, node_new_typed(ND_ADD, node, assign(tokens)));
  else if (token_consume(tokens, "-="))
    node = node_new_assign(node, node_new_typed(ND_SUB, node, assign(tokens)));
  else if (token_consume(tokens, "*="))
    node = node_new_assign(node, node_new_typed(ND_MUL, node, assign(tokens)));
  else if (token_consume(tokens, "/="))
    node = node_new_assign(node, node_new_typed(ND_DIV, node, assign(tokens)));
  else if (token_consume(tokens, "%="))
    node = node_new_assign(node, node_new_typed(ND_MOD, node, assign(tokens)));
  else if (token_consume(tokens, "&="))
    node = node_new_assign(node, node_new_typed(ND_AND, node, assign(tokens)));
  else if (token_consume(tokens, "^="))
    node = node_new_assign(node, node_new_typed(ND_XOR, node, assign(tokens)));
  else if (token_consume(tokens, "|="))
    node = node_new_assign(node, node_new_typed(ND_OR, node, assign(tokens)));
  else if (token_consume(tokens, "<<="))
    node =
      node_new_assign(node, node_new_typed(ND_SHIFT_L, node, assign(tokens)));
  else if (token_consume(tokens, ">>="))
    node =
      node_new_assign(node, node_new_typed(ND_SHIFT_R, node, assign(tokens)));
  return node;
}

Node*
declaration(Tokens* tokens)
{
  Type* type = token_consume_type(tokens);
  if (type) {
    Token* ident = token_consume_ident(tokens);
    Node* node;
    LVar* lvar;

    if (token_consume(tokens, "(")) {
      type = type_new_func(type);
      node = node_new_func();
      while (!token_consume(tokens, ")")) {
        if (type->args->size > 0)
          token_expect(tokens, ",");
        if (token_consume(tokens, "void")) {
          break;
        }
        Type* arg_type = token_expect_type(tokens);
        vector_push(type->args, arg_type);
        Token* arg_ident = token_expect_ident(tokens);
        LVar* arg_lvar = add_lvar(arg_ident, arg_type);
        vector_push(node->argv, arg_lvar);
      }

      lvar = add_lvar(ident, type);
      node->type = type;
      vector_push(node->argv, lvar);

      if (token_peek(tokens, ";")) {
        return node_new_declaraion(lvar);
      }

      return node;
    } else if (token_consume(tokens, "[")) {
      int size = token_expect_number(tokens);
      token_expect(tokens, "]");
      type = type_new_array(type, size);
      lvar = add_lvar(ident, type);
      if (token_peek(tokens, ";")) {
        return node_new_declaraion(lvar);
      }
    } else {
      lvar = add_lvar(ident, type);
      if (token_peek(tokens, ";")) {
        return node_new_declaraion(lvar);
      }
    }

    if (token_consume(tokens, "=")) {
      return node_new_assign(node_new_lvar(lvar), assign(tokens));
    }
  }

  return NULL;
}

Node*
expr(Tokens* tokens)
{
  Node* node = declaration(tokens);
  if (node) {
    return node;
  }

  if (token_consume(tokens, "return")) {
    Node* node = node_new_typed(ND_RETURN, expr(tokens), NULL);
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
  Node* node = declaration(tokens);
  if (node->kind == ND_FUNC) {
    if (token_consume(tokens, "{")) {
      while (!token_consume(tokens, "}")) {
        vector_push(node->children, stmt(tokens));
      }
    }
    node->val = program->code->size;
    return node;
  }
  if (node->kind == ND_ASSIGN) {
    token_expect(tokens, ";");
    return node;
  }
  if (node->kind == ND_DECLARATION) {
    token_expect(tokens, ";");
    return node;
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
    node_view_tree(node, 0);
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
find_lvar_str(char* name, int len)
{
  String* str = string_new_with_len(name, len);
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
expect_lvar_str(char* name, int len)
{
  LVar* lvar = find_lvar_str(name, len);
  if (!lvar) {
    error_at_until(name, len, "undefined variable: %.*s", len, name);
  }
  return lvar;
}

LVar*
add_lvar(Token* tok, Type* type)
{
  LVar* lvar = find_lvar(tok);
  if (lvar == NULL) {
    size_t head = program->latest_offset;
    if (type->kind == TY_ARRAY) {
      head += type_sizeof_aligned(type->ptr_to);
      program->latest_offset += type_sizeof_aligned(type);
    } else {
      program->latest_offset += type_sizeof_aligned(type);
      head = program->latest_offset;
    }
    lvar = lvar_new(tok->str, tok->len, head, type);
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
