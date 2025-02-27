#include "rccrsv.h"

bool
isnondigit(char c)
{
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

Vector*
vector_new(size_t capacity)
{
  Vector* vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void*) * capacity);
  vec->size = 0;
  vec->capacity = capacity;
  return vec;
}

void
vector_push(Vector* vec, void* item)
{
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void*) * vec->capacity);
  }
  vec->data[vec->size++] = item;
}

void*
vector_get(Vector* vec, size_t index)
{
  if (index >= vec->size)
    return NULL;
  return vec->data[index];
}

Token*
vector_get_token(Vector* vec, size_t index)
{
  return (Token*)vector_get(vec, index);
}

Node*
vector_get_node(Vector* vec, size_t index)
{
  return (Node*)vector_get(vec, index);
}

LVar*
vector_get_lvar(Vector* vec, size_t index)
{
  return (LVar*)vector_get(vec, index);
}

Type*
vector_get_type(Vector* vec, size_t index)
{
  return (Type*)vector_get(vec, index);
}

String*
vector_get_string(Vector* vec, size_t index)
{
  return (String*)vector_get(vec, index);
}

unsigned int
hash(const char* key)
{
  unsigned int hash = 5381;
  int c;
  while ((c = *key++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash % HASHMAP_SIZE;
}

HashMap*
hashmap_new()
{
  HashMap* map = malloc(sizeof(HashMap));
  memset(map->buckets, 0, sizeof(map->buckets));
  return map;
}

void
hashmap_put(HashMap* map, const char* key, void* value)
{
  unsigned int h = hash(key);
  HashNode* node = map->buckets[h];
  while (node) {
    if (strcmp(node->key, key) == 0) {
      node->value = value;
      return;
    }
    node = node->next;
  }
  HashNode* new_node = malloc(sizeof(HashNode));
  new_node->key = strdup(key);
  new_node->value = value;
  new_node->next = map->buckets[h];
  map->buckets[h] = new_node;
}

void*
hashmap_get(HashMap* map, const char* key)
{
  unsigned int h = hash(key);
  HashNode* node = map->buckets[h];
  while (node) {
    if (strcmp(node->key, key) == 0) {
      return node->value;
    }
    node = node->next;
  }
  return NULL;
}

String*
string_new(const char* s)
{
  size_t len = strlen(s);
  String* str = malloc(sizeof(String));
  str->data = malloc(len + 1);
  strcpy(str->data, s);
  str->size = len;
  str->capacity = len + 1;
  return str;
}

String*
string_new_with_len(const char* s, size_t len)
{
  String* str = malloc(sizeof(String));
  str->data = malloc(len + 1);
  strncpy(str->data, s, len);
  str->data[len] = '\0';
  str->size = len;
  str->capacity = len + 1;
  return str;
}

String*
string_append(String* str, const char* s)
{
  size_t new_len = str->size + strlen(s);
  String* new_str = malloc(sizeof(String));
  new_str->data = malloc(new_len + 1);
  strcpy(new_str->data, str->data);
  strcat(new_str->data, s);
  new_str->size = new_len;
  new_str->capacity = new_len + 1;
  return new_str;
}

String*
string_clone(String* str)
{
  String* new_str = malloc(sizeof(String));
  new_str->data = malloc(str->size + 1);
  strcpy(new_str->data, str->data);
  new_str->size = str->size;
  new_str->capacity = str->size + 1;
  return new_str;
}

bool
string_equals(String* str1, String* str2)
{
  if (str1->size != str2->size)
    return false;
  return strcmp(str1->data, str2->data) == 0;
}

const char*
string_as_cstring(String* str)
{
  return str ? str->data : "";
}
