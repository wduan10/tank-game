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
double ZERO_HEIGHT = 0.0;
double MAX_HEIGHT = 500.0;
size_t NUM_STARS = 50;
size_t PELLET_POINTS = 18;
double SCALER = M_PI / 55;
rgb_color_t WHITE = {1.0, 1.0, 1.0};

double RADIUS = 10.0;
double SMALL_RADIUS = 0.1;
double MASS = 15.0;
double HUGE_MASS = 100.0;

double GRAVITY_CONSTANT = 500;
double SPRING_CONSTANT = 100;
// double SHAPE = 50;
double GAMMA = 2.5;

typedef struct state {
  scene_t *scene;
} state_t;

list_t *make_pellet(vector_t center, double length) {
  list_t *shape = list_init(PELLET_POINTS, (free_func_t)free);

  for (size_t i = 0; i < PELLET_POINTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);

    point->x = center.x;
    point->y = center.y + length;

    list_add(shape, point);
    polygon_rotate(shape, -M_PI / (PELLET_POINTS / 2), center);
  }

  return shape;
}

state_t *emscripten_init() {
  vector_t min = VEC_ZERO;
  vector_t max = {MAX_WIDTH, MAX_HEIGHT};
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);

  state->scene = scene_init();

  double r = 0.5;
  double g = 0.0;
  double b = 1.0;
  for (size_t i = 0; i < NUM_STARS; i++) {
    // vector_t center = {i * MAX_WIDTH / NUM_STARS , MAX_HEIGHT / 2 * exp(-1 *
    // SHAPE / ((2 * MASS) * i * SCALER)) * cos(i * SCALER) + MAX_HEIGHT / 2};
    vector_t center = {i * MAX_WIDTH / NUM_STARS,
                       MAX_HEIGHT / 2 +
                           MAX_HEIGHT / 2 * cos(i * 2 * M_PI / 20)};
    list_t *shape = make_pellet(center, RADIUS);
    rgb_color_t colour = {r + i / 100.0, g, b - i / 100.0};
    body_t *circle = body_init(shape, MASS, colour);
    scene_add_body(state->scene, circle);

    vector_t center2 = {i * MAX_WIDTH / NUM_STARS, MAX_HEIGHT / 2};
    list_t *body2 = make_pellet(center2, SMALL_RADIUS);
    body_t *mini_body = body_init(body2, HUGE_MASS, WHITE);
    create_spring(state->scene, SPRING_CONSTANT,
                  scene_get_body(state->scene, i), mini_body);
    create_drag(state->scene, GAMMA, scene_get_body(state->scene, i));
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