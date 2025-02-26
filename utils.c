#include "rccrsv.h"

bool
isnondigit(char c)
{
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

Vector*
vector_new()
{
  Vector* vec = calloc(1, sizeof(Vector));
  vec->data = malloc(sizeof(void*) * 4);
  vec->size = 0;
  vec->capacity = 4;
  return vec;
}

void
vector_push(Vector* vec, void* item)
{
  if (vec->size == vec->capacity) {
    size_t new_capacity = vec->capacity * 2;
    void** new_data = malloc(sizeof(void*) * new_capacity);
    memcpy(new_data, vec->data, sizeof(void*) * vec->size);
    free(vec->data);
    vec->data = new_data;
    vec->capacity = new_capacity;
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
string_new(const char* initial)
{
  size_t initial_size = initial ? strlen(initial) : 0;
  size_t capacity = initial_size + 1;

  String* str = (String*)malloc(sizeof(String));
  if (!str)
    return NULL;

  str->data = (char*)malloc(capacity);
  if (!str->data) {
    free(str);
    return NULL;
  }

  if (initial) {
    memcpy(str->data, initial, initial_size);
  }
  str->data[initial_size] = '\0';
  str->size = initial_size;
  str->capacity = capacity;

  return str;
}

String*
string_new_with_len(char* s, size_t len)
{
  String* str = (String*)malloc(sizeof(String));
  if (!str)
    return NULL;

  size_t capacity = (len + 1) * 2;
  str->data = (char*)malloc(capacity);
  if (!str->data) {
    free(str);
    return NULL;
  }

  memcpy(str->data, s, len);
  str->data[len] = '\0';
  str->size = len;
  str->capacity = capacity;
  return str;
}

const char*
string_as_cstring(String* str)
{
  return str ? str->data : "";
}

void
string_append(String* str, const char* s)
{
  if (!s)
    return;
  if (!str)
    return;

  size_t suffix_len = strlen(s);
  size_t new_size = str->size + suffix_len;

  if (new_size + 1 > str->capacity) {
    size_t new_capacity = (new_size + 1) * 2;
    char* new_data = (char*)malloc(new_capacity);
    if (!new_data) {
      return;
    }
    memcpy(new_data, str->data, str->size);
    free(str->data);
    str->data = new_data;
    str->capacity = new_capacity;
  }

  memcpy(str->data + str->size, s, suffix_len);
  str->size = new_size;
  str->data[new_size] = '\0';
}
