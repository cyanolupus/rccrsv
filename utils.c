#include "reccursive.h"
#include <stdlib.h>

Vector*
vector_new()
{
  Vector* vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void*) * 4);
  vec->size = 0;
  vec->capacity = 4;
  return vec;
}

void
vector_push(Vector* vec, void* item)
{
  if (vec->size == vec->capacity) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, vec->capacity * sizeof(void*));
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
string_new()
{
  String* str = malloc(sizeof(String));
  str->data = malloc(sizeof(char) * 16);
  str->size = 0;
  str->capacity = 16;
  return str;
}

String*
to_string(char* s, size_t len)
{
  String* str = malloc(sizeof(String));
  str->data = malloc(sizeof(char) * len);
  str->size = len;
  str->capacity = len;
  memcpy(str->data, s, len);
  return str;
}

void
string_add(String* str, char* s, size_t len)
{
  if (str->size + len >= str->capacity) {
    str->capacity = str->size + len;
    str->data = realloc(str->data, str->capacity);
  }
  memcpy(str->data + str->size, s, len);
  str->size += len;
}

const char*
string_as_cstring(String* str)
{
  char* cstr = malloc(str->size + 1);
  memcpy(cstr, str->data, str->size);
  cstr[str->size] = '\0';
  return cstr;
}
