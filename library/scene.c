#include "scene.h"
#include "body.h"
#include "forces.h"
#include "list.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

size_t LIST_SIZE = 10000;

typedef struct scene {
  list_t *bodies;
  list_t *force_infos;
} scene_t;

typedef struct force_info {
  force_creator_t forcer;
  list_t *bodies;
  void *aux;
} force_info_t;

void force_free(force_info_t *force_storage) {
  store_force_free(force_storage->aux);
  free(force_storage);
}

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  assert(scene != NULL);
  scene->bodies = list_init(LIST_SIZE, (free_func_t)body_free);
  scene->force_infos = list_init(LIST_SIZE, (free_func_t)force_free);

  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->force_infos);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  assert(index < list_size(scene->bodies));
  assert(index >= 0);
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_t *body = list_get(scene->bodies, index);
  body_remove(body);
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  // list_add(scene->forces, forcer);
  // list_add(scene->auxes, aux);
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  force_info_t *force_storage = malloc(sizeof(force_info_t));
  force_storage->forcer = forcer;
  force_storage->aux = aux;
  force_storage->bodies = bodies;

  list_add(scene->force_infos, force_storage);
}

void scene_tick(scene_t *scene, double dt) {
  for (size_t i = 0; i < list_size(scene->force_infos); i++) {
    force_info_t *force_storage = list_get(scene->force_infos, i);
    force_creator_t forcer = force_storage->forcer;
    store_force_t *storage = (store_force_t *)force_storage->aux;

    forcer(storage);
  }

  for (size_t i = 0; i < list_size(scene->force_infos); i++) {
    force_info_t *force_storage = list_get(scene->force_infos, i);

    list_t *bodies = force_storage->bodies;

    // loop through bodies to see if any are marked as removed
    for (size_t j = 0; j < list_size(bodies); j++) {
      body_t *body = list_get(bodies, j);
      // if body is removed, remove the corresponding force
      if (body_is_removed(body)) {
        force_info_t *f = list_remove(scene->force_infos, i);
        force_free(f);
        i--;
        break;
      }
    }
  }

  size_t size = list_size(scene->bodies);
  for (size_t i = 0; i < size; i++) {
    if (body_is_removed(list_get(scene->bodies, i))) {
      body_t *body = list_remove(scene->bodies, i);
      body_free(body);
      size--;
      i--;
    } else {
      body_tick(list_get(scene->bodies, i), dt);
    }
  }
}