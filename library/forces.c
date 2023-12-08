#include "forces.h"
#include "body.h"
#include "collision.h"
#include "map.h"
#include "scene.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double MINIMUM_DISTANCE = 5.0;

typedef struct store_force {
  list_t *bodies;
  double constant;
  collision_handler_t handler;
  void *aux;
  bool just_collided;
} store_force_t;

void store_force_free(store_force_t *storage) {
  list_free(storage->bodies);
  free(storage);
}

vector_t calculate_unit_vector(vector_t body1, vector_t body2) {
  double x = body2.x - body1.x;
  double y = body2.y - body1.y;
  double magnitude = sqrt(x * x + y * y);
  double u_x = x / magnitude;
  double u_y = y / magnitude;
  vector_t unit_vec = {u_x, u_y};
  return unit_vec;
}

void gravity_forcer(store_force_t *storage) {
  list_t *bodies = storage->bodies;
  body_t *body1 = list_get(bodies, 0);
  body_t *body2 = list_get(bodies, 1);
  vector_t body1_centroid = body_get_centroid(body1);
  vector_t body2_centroid = body_get_centroid(body2);
  double distance = body_get_distance(body1_centroid, body2_centroid);
  if (distance < MINIMUM_DISTANCE) {
    return;
  }
  // unit vector calculation
  vector_t unit_vec = calculate_unit_vector(body1_centroid, body2_centroid);

  // force
  double force_magnitude = (double)(body_get_mass(body1) *
                                    body_get_mass(body2) * storage->constant) /
                           (distance * distance);
  vector_t force;
  force.x = unit_vec.x * force_magnitude;
  force.y = unit_vec.y * force_magnitude;

  body_add_force(body1, force);
  body_add_force(body2, vec_negate(force));
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  store_force_t *storage = malloc(sizeof(store_force_t));
  assert(storage != NULL);
  storage->bodies = list_init(2, NULL);
  list_add(storage->bodies, body1);
  list_add(storage->bodies, body2);
  storage->constant = G;

  force_creator_t forcer = (force_creator_t)gravity_forcer;

  scene_add_bodies_force_creator(scene, forcer, storage, storage->bodies,
                                 (free_func_t)free);
}

void spring_forcer(store_force_t *storage) {
  list_t *bodies = storage->bodies;
  body_t *body1 = list_get(bodies, 0);
  body_t *body2 = list_get(bodies, 1);
  vector_t body1_centroid = body_get_centroid(body1);
  vector_t body2_centroid = body_get_centroid(body2);
  double distance = body_get_distance(body1_centroid, body2_centroid);

  double force_magnitude = storage->constant * distance;

  vector_t unit_vec = calculate_unit_vector(body1_centroid, body2_centroid);

  // force
  vector_t force;
  force.x = unit_vec.x * force_magnitude;
  force.y = unit_vec.y * force_magnitude;

  body_add_force(body1, force);
  body_add_force(body2, vec_negate(force));
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  store_force_t *storage = malloc(sizeof(store_force_t));
  storage->bodies = list_init(2, NULL);
  list_add(storage->bodies, body1);
  list_add(storage->bodies, body2);
  storage->constant = k;

  force_creator_t forcer = (force_creator_t)spring_forcer;

  scene_add_bodies_force_creator(scene, forcer, storage, storage->bodies,
                                 (free_func_t)free);
}

void drag_forcer(store_force_t *storage) {
  list_t *bodies = storage->bodies;
  body_t *body = list_get(bodies, 0);
  vector_t velocity = body_get_velocity(body);
  vector_t drag_force = vec_negate(vec_multiply(storage->constant, velocity));
  body_add_force(body, drag_force);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  store_force_t *storage = malloc(sizeof(store_force_t));
  storage->bodies = list_init(1, NULL);
  list_add(storage->bodies, body);
  storage->constant = gamma;

  force_creator_t forcer = (force_creator_t)drag_forcer;

  scene_add_bodies_force_creator(scene, forcer, storage, storage->bodies,
                                 (free_func_t)free);
}

void destructive_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                                   void *aux) {
  body_remove(body1);
  body_remove(body2);
}

void partial_destructive_collision_handler(body_t *body1, body_t *body2,
                                           vector_t axis, void *aux) {
  // need to be careful here to consider all cases
  if (*(size_t *)body_get_info(body2) == BULLET_TYPE) {
    body_set_health(body1, body_get_health(body1) - BULLET_DAMAGE);
    body_remove(body2);
  } else if (*(size_t *)body_get_info(body2) == SNIPER_BULLET_TYPE) {
    body_set_health(body1, body_get_health(body1) - SNIPER_BULLET_DAMAGE);
    body_remove(body2);
  } else if (*(size_t *)body_get_info(body2) == GATLING_BULLET_TYPE) {
    body_set_health(body1, body_get_health(body1) - GATLING_BULLET_DAMAGE);
    body_remove(body2);
  } else if (*(size_t *)body_get_info(body2) == GRAVITY_BULLET_TYPE) {
    body_set_health(body1, body_get_health(body1) - GRAVITY_BULLET_DAMAGE);
    body_remove(body2);
  }
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  create_collision(scene, body1, body2, destructive_collision_handler, NULL,
                   (free_func_t)free);
}

void create_partial_destructive_collision(scene_t *scene, body_t *body1,
                                          body_t *body2) {
  create_collision(scene, body1, body2, partial_destructive_collision_handler,
                   NULL, (free_func_t)free);
}

void impulse_handler(body_t *body1, body_t *body2, vector_t axis, void *aux) {
  double constant = *(double *)aux;
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);

  double u_a = vec_dot(body_get_velocity(body1), axis);
  double u_b = vec_dot(body_get_velocity(body2), axis);

  double reduced_mass;
  if (mass1 == INFINITY && mass2 != INFINITY) {
    reduced_mass = mass2;
  } else if (mass2 == INFINITY && mass1 != INFINITY) {
    reduced_mass = mass1;
  } else {
    reduced_mass = ((mass1 * mass2) / (mass1 + mass2));
  }

  double impulse_magnitude = reduced_mass * (1 + constant) * (u_b - u_a);
  vector_t impulse = vec_multiply(impulse_magnitude, axis);

  body_add_impulse(body1, impulse);
  body_add_impulse(body2, vec_negate(impulse));

  if (*(size_t *)body_get_info(body1) == TRIANGLE_OBSTACLE_TYPE) {
    body_set_health(body2, body_get_health(body2) - TRIANGLE_DAMAGE);
  }
  if (*(size_t *)body_get_info(body2) == TRIANGLE_OBSTACLE_TYPE) {
    body_set_health(body1, body_get_health(body1) - TRIANGLE_DAMAGE);
  }
}

void custom_forcer(store_force_t *storage) {
  list_t *bodies = storage->bodies;
  body_t *body1 = list_get(bodies, 0);
  body_t *body2 = list_get(bodies, 1);

  // if (storage->just_collided) {
  //   storage->just_collided = false;
  //   return;
  // }

  collision_info_t collision_info =
      find_collision(body_get_shape(body1), body_get_shape(body2));

  if (collision_info.collided == false) {
    storage->just_collided = false;
    return;
  }
  body_set_just_collided(body1, true);
  body_set_just_collided(body2, true);
  if (storage->just_collided) {
    return;
  }
  storage->just_collided = true;

  collision_handler_t handler = storage->handler;
  void *aux = storage->aux;

  handler(body1, body2, collision_info.axis, aux);
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  double *constant = malloc(sizeof(double));
  *constant = elasticity;
  create_collision(scene, body1, body2, impulse_handler, constant,
                   (free_func_t)free);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  store_force_t *storage = malloc(sizeof(store_force_t));
  storage->bodies = list_init(2, NULL);
  list_add(storage->bodies, body1);
  list_add(storage->bodies, body2);
  storage->aux = aux;
  storage->handler = handler;

  force_creator_t forcer = (force_creator_t)custom_forcer;

  scene_add_bodies_force_creator(scene, forcer, storage, storage->bodies,
                                 (free_func_t)free);
}