#include "rccrsv.h"

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
      snprintf(
        buf, 255, "local %s", vector_get_lvar(node->argv, 0)->name->data);
      return string_new(buf);
    }
    case ND_GVAR: {
      char buf[255];
      snprintf(
        buf, 255, "global %s", vector_get_lvar(node->argv, 0)->name->data);
      return string_new(buf);
    }
    case ND_RETURN:
      return string_new("return");
    case ND_NUM: {
      char buf[255];
      switch (node->type->kind) {
        case TY_I8:
          snprintf(buf, 255, "%d", *(int*)&node->val);
          break;
        case TY_I16:
          snprintf(buf, 255, "%d", *(int*)&node->val);
          break;
        case TY_ISIZE:
          snprintf(buf, 255, "%d", *(int*)&node->val);
          break;
        case TY_I32:
          snprintf(buf, 255, "%ld", *(long*)&node->val);
          break;
        case TY_I64:
          snprintf(buf, 255, "%lld", *(long long*)&node->val);
          break;
        case TY_U8:
          snprintf(buf, 255, "%u", *(unsigned int*)&node->val);
          break;
        case TY_U16:
          snprintf(buf, 255, "%u", *(unsigned int*)&node->val);
          break;
        case TY_USIZE:
          snprintf(buf, 255, "%u", *(unsigned int*)&node->val);
          break;
        case TY_U32:
          snprintf(buf, 255, "%lu", *(unsigned long*)&node->val);
          break;
        case TY_U64:
          snprintf(buf, 255, "%llu", *(unsigned long long*)&node->val);
          break;
        default:
          fprintf(stderr,
                  "Invalid number type: %s\n",
                  type_to_string(node->type)->data);
          exit(1);
      }
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
    case ND_LDECLARE:
      return string_new("local declare");
    case ND_GDECLARE:
      return string_new("global declare");
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
    case ND_AUTOCAST:
      return string_new("autocast");
    case ND_STRING:
      return string_new("string");
    default:
      return string_new("unknown");
  }
}

Node*
node_new(NodeKind kind, Type* type)
{
  Node* node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->type = type;
  return node;
}

Node*
node_new_1op(NodeKind kind, Node* lhs, Type* type)
{
  Node* node = node_new(kind, type);
  node->children = vector_new(1);
  vector_push(node->children, lhs);
  return node;
}

Node*
node_new_2op(NodeKind kind, Node* lhs, Node* rhs, Type* type)
{
  Node* node = node_new(kind, type);
  node->children = vector_new(2);
  vector_push(node->children, lhs);
  vector_push(node->children, rhs);
  return node;
}

Node*
node_new_block()
{
  Node* node = node_new(ND_BLOCK, NULL);
  node->children = vector_new(8);
  return node;
}

Node*
node_new_func()
{
  Node* node = node_new(ND_FUNC, NULL);
  node->val = 0;
  node->argv = vector_new(8);
  node->children = vector_new(8);
  return node;
}

Node*
node_new_if(Node* cond, Node* then, Node* els, Type* type)
{
  Node* node = node_new(ND_IF, type);
  node->children = vector_new(3);
  vector_push(node->children, cond);
  vector_push(node->children, then);
  vector_push(node->children, els);
  return node;
}

Node*
node_new_for(Node* init, Node* cond, Node* inc, Node* body)
{
  Node* node = node_new(ND_FOR, NULL);
  node->children = vector_new(4);
  vector_push(node->children, init);
  vector_push(node->children, cond);
  vector_push(node->children, inc);
  vector_push(node->children, body);
  return node;
}

Node*
node_new_number(long long val, Type* type)
{
  Node* node = node_new(ND_NUM, type);
  switch (type->kind) {
    case TY_I8:
      node->val = val;
      return node;
    case TY_I16:
      node->val = val;
      return node;
    case TY_ISIZE:
      node->val = val;
      return node;
    case TY_I32:
      node->val = val;
      return node;
    case TY_I64:
      node->val = val;
      return node;
    case TY_U8:
      node->val = val;
      return node;
    case TY_U16:
      node->val = val;
      return node;
    case TY_USIZE:
      node->val = val;
      return node;
    case TY_U32:
      node->val = val;
      return node;
    case TY_U64:
      node->val = val;
      return node;
    default:
      fprintf(stderr, "Invalid number type: %s\n", type_to_string(type)->data);
      exit(1);
  }
}

Node*
node_new_const(long long val, Type* type)
{
  switch (type->kind) {
    case TY_I8:
    case TY_I16:
    case TY_ISIZE:
    case TY_I32:
    case TY_I64:
    case TY_U8:
    case TY_U16:
    case TY_USIZE:
    case TY_U32:
    case TY_U64:
      return node_new_number(val, type);
    case TY_PTR:
      return node_new(ND_NUM, type);
    case TY_ARRAY:
      return node_new(ND_NUM, type);
    default:
      fprintf(stderr, "Invalid const type: %s\n", type_to_string(type)->data);
      exit(1);
  }
}

Node*
node_new_lvar(LVar* lvar)
{
  Node* node = node_new(ND_LVAR, lvar->type);
  node->argv = vector_new(1);
  vector_push(node->argv, lvar);
  return node;
}

Node*
node_new_gvar(LVar* lvar)
{
  Node* node = node_new(ND_GVAR, lvar->type);
  node->argv = vector_new(1);
  vector_push(node->argv, lvar);
  return node;
}

Node*
node_new_var(LVar* lvar)
{
  if (lvar->offset == 0)
    return node_new_gvar(lvar);
  return node_new_lvar(lvar);
}

Node*
node_new_call(LVar* lvar)
{
  Node* node = node_new(ND_CALL, lvar->type->ptr_to);
  node->children = vector_new(8);
  node->argv = vector_new(8);
  vector_push(node->argv, lvar);
  return node;
}

Node*
node_new_autocast(Node* lhs, Type* dst)
{
  if (type_equals(lhs->type, dst))
    return lhs;
  Node* node = node_new_1op(ND_AUTOCAST, lhs, dst);
  node->type = dst;
  return node;
}

Node*
node_new_typed(NodeKind kind, Node* lhs, Node* rhs)
{
  if (!lhs || !rhs || !lhs->type) {
    fprintf(stderr, "node cannot be typed\n");
    exit(1);
  }
  if (!rhs->type)
    return node_new_2op(kind, lhs, rhs, lhs->type);
  if (!type_equals(lhs->type, rhs->type)) {
    fprintf(stderr,
            "Type mismatch: %s %s %s\n",
            type_to_string(lhs->type)->data,
            node_kind_to_string(lhs)->data,
            node_kind_to_string(rhs)->data);
    exit(1);
  }
  return node_new_2op(kind, lhs, rhs, lhs->type);
}

Node*
node_new_typed_cast(NodeKind kind, Node* lhs, Node* rhs)
{
  return node_new_typed(kind, lhs, node_new_autocast(rhs, lhs->type));
}

Node*
node_new_assign(Node* lhs, Node* rhs)
{
  return node_new_typed_cast(ND_ASSIGN, lhs, rhs);
}

Node*
node_new_post_assign(Node* lhs, Node* rhs)
{
  return node_new_typed_cast(ND_POST_ASSIGN, lhs, rhs);
}

Node*
node_new_ldeclare(LVar* lvar)
{
  Node* node = node_new(ND_LDECLARE, lvar->type);
  node->val = 0;
  node->argv = vector_new(1);
  vector_push(node->argv, lvar);
  return node;
}

Node*
node_new_ldeclare_with_init(LVar* lvar, Node* init)
{
  Node* node = node_new(ND_LDECLARE, lvar->type);
  node->val = 0;
  node->argv = vector_new(1);
  vector_push(node->argv, lvar);
  node->children = vector_new(1);
  vector_push(node->children, node_new_assign(node_new_lvar(lvar), init));
  return node;
}

Node*
node_new_gdeclare(LVar* lvar)
{
  Node* node = node_new(ND_GDECLARE, lvar->type);
  node->val = 0;
  node->argv = vector_new(1);
  vector_push(node->argv, lvar);
  return node;
}

Node*
node_new_gdeclare_with_init(LVar* lvar, Node* init)
{
  Node* node = node_new(ND_GDECLARE, lvar->type);
  node->val = 0;
  node->argv = vector_new(1);
  vector_push(node->argv, lvar);
  node->children = vector_new(1);
  vector_push(node->children, node_new_assign(node_new_gvar(lvar), init));
  return node;
}

Node*
node_new_string(size_t index)
{
  Node* node = node_new(ND_STRING, type_new_ptr(type_new_u8()));
  node->argv = vector_new(1);
  node->val = index;
  return node;
}

void
node_view_tree(Node* node, size_t depth)
{
  if (!node)
    return;
  for (int i = 0; i < depth; i++)
    printf("  ");
  printf("%s %s\n",
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
  String* str = token_consume_str(tokens);
  if (str) {
    vector_push(program->strs, str);
    return node_new_string(program->strs->size - 1);
  }

  String* name = token_consume_ident(tokens);
  if (name) {
    LVar* lvar = expect_var(name);
    if (token_consume(tokens, string_new("("))) {
      Node* node = node_new_call(lvar);
      while (!token_consume(tokens, string_new(")"))) {
        if (node->children->size > 0)
          token_expect(tokens, string_new(","));
        vector_push(node->children, expr(tokens));
      }
      return node;
    }

    if (lvar->type->kind == TY_ARRAY &&
        token_consume(tokens, string_new("["))) {
      Node* node = node_new_1op(
        ND_REF, node_new_var(lvar), type_new_ptr(lvar->type->ptr_to));
      Node* offset = expr(tokens);
      token_expect(tokens, string_new("]"));
      if (node->type->ptr_to == NULL) {
        fprintf(stderr,
                "Dereference of non-pointer type: %s\n",
                type_to_string(node->type)->data);
        exit(1);
      }
      return node_new_1op(ND_DEREF,
                          node_new_typed_cast(ND_ADD, node, offset),
                          node->type->ptr_to);
    }

    if (token_consume(tokens, string_new("++"))) {
      Node* add = node_new_typed_cast(
        ND_ADD, node_new_var(lvar), node_new_number(1, type_new_u8()));
      Node* assign = node_new_post_assign(node_new_var(lvar), add);
      return assign;
    }

    if (token_consume(tokens, string_new("--"))) {
      Node* sub = node_new_typed_cast(
        ND_SUB, node_new_var(lvar), node_new_number(1, type_new_u8()));
      Node* assign = node_new_post_assign(node_new_var(lvar), sub);
      return assign;
    }

    Node* node = node_new_var(lvar);
    if (lvar->type->kind == TY_ARRAY) {
      return node_new_1op(ND_REF, node, type_new_ptr(lvar->type->ptr_to));
    }
    return node;
  }

  if (token_consume(tokens, string_new("("))) {
    Node* node = expr(tokens);
    token_expect(tokens, string_new(")"));
    return node;
  }

  int num = token_expect_number(tokens);
  if (token_consume(tokens, string_new("["))) {
    Node* node = node_new_number(num, type_new_isize());
    Node* offset = expr(tokens);
    token_expect(tokens, string_new("]"));
    return node_new_1op(ND_REF,
                        node_new_typed_cast(ND_ADD, node, offset),
                        type_new_ptr(offset->type));
  }

  return node_new_number(num, type_new_isize());
}

Node*
unary(Tokens* tokens)
{
  if (token_consume(tokens, string_new("++"))) {
    String* name = token_expect_ident(tokens);
    LVar* lvar = expect_var(name);
    Node* add = node_new_typed_cast(
      ND_ADD, node_new_var(lvar), node_new_number(1, type_new_u8()));
    Node* assign = node_new_assign(node_new_var(lvar), add);
    return assign;
  }
  if (token_consume(tokens, string_new("--"))) {
    String* name = token_expect_ident(tokens);
    LVar* lvar = expect_var(name);
    Node* sub = node_new_typed_cast(
      ND_SUB, node_new_var(lvar), node_new_number(1, type_new_u8()));
    Node* assign = node_new_assign(node_new_var(lvar), sub);
    return assign;
  }
  if (token_consume(tokens, string_new("+")))
    return primary(tokens);
  if (token_consume(tokens, string_new("-"))) {
    Node* node = primary(tokens);
    return node_new_typed_cast(ND_SUB, node_new_number(0, node->type), node);
  }
  if (token_consume(tokens, string_new("!")))
    return node_new_1op(ND_LNOT, primary(tokens), type_new_isize());
  if (token_consume(tokens, string_new("~"))) {
    Node* node = primary(tokens);
    return node_new_1op(ND_NOT, node, node->type);
  }
  if (token_consume(tokens, string_new("*"))) {
    Node* node = unary(tokens);
    if (node->type->ptr_to == NULL) {
      fprintf(stderr,
              "Dereference of non-pointer type: %s\n",
              type_to_string(node->type)->data);
      exit(1);
    }
    return node_new_1op(ND_DEREF, node, node->type->ptr_to);
  }
  if (token_consume(tokens, string_new("&"))) {
    Node* node = unary(tokens);
    return node_new_1op(ND_REF, node, type_new_ptr(node->type));
  }
  if (token_consume(tokens, string_new("sizeof")))
    return node_new_1op(ND_SIZEOF, unary(tokens), type_new_usize());
  return primary(tokens);
}

Node*
mul(Tokens* tokens)
{
  Node* node = unary(tokens);

  for (;;) {
    if (token_consume(tokens, string_new("*")))
      node = node_new_typed_cast(ND_MUL, node, unary(tokens));
    else if (token_consume(tokens, string_new("/")))
      node = node_new_typed_cast(ND_DIV, node, unary(tokens));
    else if (token_consume(tokens, string_new("%")))
      node = node_new_typed_cast(ND_MOD, node, unary(tokens));
    else
      return node;
  }
}

Node*
add(Tokens* tokens)
{
  Node* node = mul(tokens);
  for (;;) {
    if (token_consume(tokens, string_new("+"))) {
      Node* rhs = mul(tokens);
      if (rhs->type->kind == TY_PTR)
        return node_new_typed_cast(ND_ADD, rhs, node);
      else
        return node_new_typed_cast(ND_ADD, node, rhs);
    } else if (token_consume(tokens, string_new("-"))) {
      Node* rhs = mul(tokens);
      if (rhs->type->kind == TY_PTR)
        return node_new_typed_cast(ND_SUB, rhs, node);
      else
        return node_new_typed_cast(ND_SUB, node, rhs);
    } else {
      return node;
    }
  }
}

Node*
shift(Tokens* tokens)
{
  Node* node = add(tokens);

  for (;;) {
    if (token_consume(tokens, string_new("<<")))
      node = node_new_typed_cast(ND_SHIFT_L, node, add(tokens));
    else if (token_consume(tokens, string_new(">>")))
      node = node_new_typed_cast(ND_SHIFT_R, node, add(tokens));
    else
      return node;
  }
}

Node*
relational(Tokens* tokens)
{
  Node* node = shift(tokens);

  for (;;) {
    if (token_consume(tokens, string_new("<")))
      return node_new_2op(ND_LT,
                          node,
                          node_new_autocast(shift(tokens), node->type),
                          type_new_isize());
    else if (token_consume(tokens, string_new("<=")))
      return node_new_2op(ND_LE,
                          node,
                          node_new_autocast(shift(tokens), node->type),
                          type_new_isize());
    else if (token_consume(tokens, string_new(">")))
      return node_new_2op(ND_LT,
                          node_new_autocast(shift(tokens), node->type),
                          node,
                          type_new_isize());
    else if (token_consume(tokens, string_new(">=")))
      return node_new_2op(ND_LE,
                          node_new_autocast(shift(tokens), node->type),
                          node,
                          type_new_isize());
    else
      return node;
  }
}

Node*
equality(Tokens* tokens)
{
  Node* node = relational(tokens);

  for (;;) {
    if (token_consume(tokens, string_new("==")))
      return node_new_2op(ND_EQ,
                          node,
                          node_new_autocast(relational(tokens), node->type),
                          type_new_isize());
    else if (token_consume(tokens, string_new("!=")))
      return node_new_2op(ND_NE,
                          node,
                          node_new_autocast(relational(tokens), node->type),
                          type_new_isize());
    else
      return node;
  }
}

Node*
bitwise_and(Tokens* tokens)
{
  Node* node = equality(tokens);

  for (;;) {
    if (token_consume(tokens, string_new("&")))
      node = node_new_typed_cast(ND_AND, node, equality(tokens));
    else
      return node;
  }
}

Node*
bitwise_xor(Tokens* tokens)
{
  Node* node = bitwise_and(tokens);

  for (;;) {
    if (token_consume(tokens, string_new("^")))
      node = node_new_typed_cast(ND_XOR, node, bitwise_and(tokens));
    else
      return node;
  }
}

Node*
bitwise_or(Tokens* tokens)
{
  Node* node = bitwise_xor(tokens);

  for (;;) {
    if (token_consume(tokens, string_new("|")))
      node = node_new_typed_cast(ND_OR, node, bitwise_xor(tokens));
    else
      return node;
  }
}

Node*
logical_and(Tokens* tokens)
{
  Node* node = bitwise_or(tokens);

  for (;;) {
    if (token_consume(tokens, string_new("&&")))
      node = node_new_typed_cast(ND_AND, node, bitwise_or(tokens));
    else
      return node;
  }
}

Node*
logical_or(Tokens* tokens)
{
  Node* node = logical_and(tokens);

  for (;;) {
    if (token_consume(tokens, string_new("||")))
      node = node_new_typed_cast(ND_OR, node, logical_and(tokens));
    else
      return node;
  }
}

Node*
conditional(Tokens* tokens)
{
  Node* node = logical_or(tokens);

  if (token_consume(tokens, string_new("?"))) {
    Node* then = conditional(tokens);
    token_expect(tokens, string_new(":"));
    Node* els = conditional(tokens);
    return node_new_if(node, then, els, then->type);
  }
  return node;
}

Node*
assign(Tokens* tokens)
{
  Node* node = conditional(tokens);

  if (token_consume(tokens, string_new("=")))
    node = node_new_assign(node, assign(tokens));
  else if (token_consume(tokens, string_new("+=")))
    node =
      node_new_assign(node, node_new_typed_cast(ND_ADD, node, assign(tokens)));
  else if (token_consume(tokens, string_new("-=")))
    node =
      node_new_assign(node, node_new_typed_cast(ND_SUB, node, assign(tokens)));
  else if (token_consume(tokens, string_new("*=")))
    node =
      node_new_assign(node, node_new_typed_cast(ND_MUL, node, assign(tokens)));
  else if (token_consume(tokens, string_new("/=")))
    node =
      node_new_assign(node, node_new_typed_cast(ND_DIV, node, assign(tokens)));
  else if (token_consume(tokens, string_new("%=")))
    node =
      node_new_assign(node, node_new_typed_cast(ND_MOD, node, assign(tokens)));
  else if (token_consume(tokens, string_new("&=")))
    node =
      node_new_assign(node, node_new_typed_cast(ND_AND, node, assign(tokens)));
  else if (token_consume(tokens, string_new("^=")))
    node =
      node_new_assign(node, node_new_typed_cast(ND_XOR, node, assign(tokens)));
  else if (token_consume(tokens, string_new("|=")))
    node =
      node_new_assign(node, node_new_typed_cast(ND_OR, node, assign(tokens)));
  else if (token_consume(tokens, string_new("<<=")))
    node = node_new_assign(
      node, node_new_typed_cast(ND_SHIFT_L, node, assign(tokens)));
  else if (token_consume(tokens, string_new(">>=")))
    node = node_new_assign(
      node, node_new_typed_cast(ND_SHIFT_R, node, assign(tokens)));
  return node;
}

Node*
declaration(Tokens* tokens, bool global)
{
  Type* type = token_consume_type(tokens);
  if (type) {
    String* name = token_consume_ident(tokens);
    Node* node;
    LVar* lvar;

    if (token_consume(tokens, string_new("("))) {
      type = type_new_func(type);
      node = node_new_func();
      while (!token_consume(tokens, string_new(")"))) {
        if (type->args->size > 0)
          token_expect(tokens, string_new(","));
        if (token_consume(tokens, string_new("void"))) {
          break;
        }
        Type* arg_type = token_expect_type(tokens);
        vector_push(type->args, arg_type);
        String* arg_name = token_expect_ident(tokens);
        LVar* arg_lvar = add_lvar(arg_name, arg_type);
        vector_push(node->argv, arg_lvar);
      }

      lvar = add_lvar(name, type);
      node->type = type;
      vector_push(node->argv, lvar);

      if (token_peek(tokens, string_new(";"))) {
        if (global)
          return node_new_gdeclare(lvar);
        else
          return node_new_ldeclare(lvar);
      }

      return node;
    } else if (token_consume(tokens, string_new("["))) {
      int size = token_expect_number(tokens);
      token_expect(tokens, string_new("]"));
      type = type_new_array(type, size);
      lvar = global ? add_gvar(name, type) : add_lvar(name, type);
      if (token_peek(tokens, string_new(";"))) {
        if (global)
          return node_new_gdeclare(lvar);
        else
          return node_new_ldeclare(lvar);
      }
    } else {
      lvar = global ? add_gvar(name, type) : add_lvar(name, type);
      if (token_peek(tokens, string_new(";"))) {
        if (global)
          return node_new_gdeclare(lvar);
        else
          return node_new_ldeclare(lvar);
      }
    }

    if (token_consume(tokens, string_new("="))) {
      if (global) {
        return node_new_gdeclare_with_init(
          lvar, node_new_const(token_expect_number(tokens), lvar->type));
      } else
        return node_new_ldeclare_with_init(lvar, assign(tokens));
    }
  }

  return NULL;
}

Node*
expr(Tokens* tokens)
{
  Node* node = declaration(tokens, false);
  if (node) {
    return node;
  }

  if (token_consume(tokens, string_new("return"))) {
    Node* node = expr(tokens);
    return node_new_1op(ND_RETURN, node, node->type);
  }
  return assign(tokens);
}

Node*
stmt(Tokens* tokens)
{
  if (token_consume(tokens, string_new("if"))) {
    token_expect(tokens, string_new("("));
    Node* cond = expr(tokens);
    token_expect(tokens, string_new(")"));
    Node* then = stmt(tokens);
    Node* els = NULL;
    if (token_consume(tokens, string_new("else")))
      els = stmt(tokens);
    return node_new_if(cond, then, els, NULL);
  }
  if (token_consume(tokens, string_new("while"))) {
    token_expect(tokens, string_new("("));
    Node* cond = NULL;
    if (!token_consume(tokens, string_new(")"))) {
      cond = expr(tokens);
      token_expect(tokens, string_new(")"));
    }
    Node* body = stmt(tokens);
    return node_new_for(NULL, cond, NULL, body);
  }
  if (token_consume(tokens, string_new("for"))) {
    Node *init = NULL, *cond = NULL, *inc = NULL;
    token_expect(tokens, string_new("("));
    if (!token_consume(tokens, string_new(";"))) {
      init = expr(tokens);
      token_expect(tokens, string_new(";"));
    }
    if (!token_consume(tokens, string_new(";"))) {
      cond = expr(tokens);
      token_expect(tokens, string_new(";"));
    }
    if (!token_consume(tokens, string_new(")"))) {
      inc = expr(tokens);
      token_expect(tokens, string_new(")"));
    }
    Node* body = stmt(tokens);
    return node_new_for(init, cond, inc, body);
  }
  if (token_consume(tokens, string_new("{"))) {
    Node* node = node_new_block();
    while (!token_consume(tokens, string_new("}"))) {
      vector_push(node->children, stmt(tokens));
    }
    return node;
  }
  Node* node = expr(tokens);
  token_expect(tokens, string_new(";"));
  return node;
}

Node*
global(Tokens* tokens)
{
  Node* node = declaration(tokens, true);
  if (node->kind == ND_FUNC) {
    if (token_consume(tokens, string_new("{"))) {
      while (!token_consume(tokens, string_new("}"))) {
        vector_push(node->children, stmt(tokens));
      }
    }
    node->val = program->code->size;
    return node;
  }
  if (node->kind == ND_GDECLARE) {
    token_expect(tokens, string_new(";"));
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
      if (node->kind == ND_FUNC) {
        vector_push(program->code, node);
      } else if (node->kind == ND_GDECLARE) {
        vector_push(program->vars, node);
      }
    }
  }
}

Program*
program_new()
{
  Program* program = calloc(1, sizeof(Program));
  program->code = vector_new(32);
  program->vars = vector_new(32);
  program->strs = vector_new(32);
  program->locals = hashmap_new();
  program->globals = hashmap_new();
  program->latest_offset = 16;
  return program;
}
