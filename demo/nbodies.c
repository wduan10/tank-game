#include "body.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "star.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

double MAX_WIDTH = 1000.0;
double MAX_HEIGHT = 500.0;
size_t NUM_STARS = 50;
int STAR_POINTS = 4;

double MIN_LENGTH = 10.0;
double MAX_LENGTH = 30.0;
double MIN_MASS = 5.0;
double MAX_MASS = 20.0;

double GRAVITY_CONSTANT = 500;

typedef struct state {
  scene_t *scene;
} state_t;

state_t *emscripten_init() {
  vector_t min = VEC_ZERO;
  vector_t max = {MAX_WIDTH, MAX_HEIGHT};
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);

  state->scene = scene_init();

  for (size_t i = 0; i < NUM_STARS; i++) {
    vector_t center = {rand_num(0.0, MAX_WIDTH), rand_num(0.0, MAX_HEIGHT)};
    list_t *star =
        make_star(center, rand_num(MIN_LENGTH, MAX_LENGTH), STAR_POINTS);
    double mass = rand_num(MIN_MASS, MAX_MASS);
    rgb_color_t color = {rand_num(0.0, 1.0), rand_num(0.0, 1.0),
                         rand_num(0.0, 1.0)};
    body_t *body = body_init(star, mass, color);
    scene_add_body(state->scene, body);
  }

  for (size_t i = 0; i < NUM_STARS; i++) {
    for (size_t j = i + 1; j < NUM_STARS; j++) {
      create_newtonian_gravity(state->scene, GRAVITY_CONSTANT,
                               scene_get_body(state->scene, i),
                               scene_get_body(state->scene, j));
    }
  }

  return state;
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();

  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
