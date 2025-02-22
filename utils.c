#include "reccursive.h"

void
vector_new(Vector* vec)
{
  vec->size = 0;
  vec->capacity = 4;
  vec->data = malloc(vec->capacity * sizeof(void*));
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

void
hashmap_new(HashMap* map)
{
  memset(map->buckets, 0, sizeof(map->buckets));
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
