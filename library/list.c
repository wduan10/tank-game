#include "list.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

size_t GROW_FACTOR = 2;

typedef struct list {
  void **items;
  size_t size;
  int capacity;
  free_func_t freer;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *lst = malloc(sizeof(list_t));
  assert(lst != NULL);

  lst->size = 0;
  lst->capacity = initial_size;
  lst->items = malloc(initial_size * sizeof(void *));
  assert(lst->items != NULL);

  if (freer != NULL) {
    lst->freer = freer;
  } else {
    lst->freer = NULL;
  }

  return lst;
}

void list_free(list_t *list) {
  assert(list != NULL);

  if (list->freer != NULL) {
    for (size_t i = 0; i < list->size; i++) {
      list->freer(list->items[i]);
    }
  }

  free(list->items);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index < (size_t)(list->size));
  void *item = list->items[index];
  return item;
}

void *list_remove(list_t *list, size_t index) {
  assert(index >= 0);
  assert(index < list->size);

  void *elem = list->items[index];
  for (size_t i = index; i < list->size - 1; i++) {
    list->items[i] = list->items[i + 1];
  }
  list->items[list->size - 1] = NULL;
  list->size--;

  return elem;
}

void list_add(list_t *list, void *value) {
  assert(value != NULL);

  if (list->size >= list->capacity) {
    list->capacity = list->capacity * GROW_FACTOR;
    list->items = realloc(list->items, sizeof(void *) * list->capacity);
  }

  list->items[list->size] = value;
  list->size++;
}

void *list_replace(list_t *list, size_t index, void *value) {
  assert(index < (size_t)(list->size));
  void *item = list->items[index];
  list->items[index] = value;
  return item;
}
