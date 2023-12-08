#include "body.h"
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
double PACMAN_RADIUS = 60.0;

double DEFAULT_SPEED = 200.0;
double DEFAULT_ANGLE = M_PI / 4;
double DEFAULT_ROTATION_SPEED = M_PI / 8;
size_t NUMBER_OF_PELLETS = 15;
double PELLET_RADIUS = 5.0;
double CIRCLE_POINTS = 300.0;
double SPAWN_TIME = 1.0;
size_t PELLET_POINTS = 18;
double INITIAL_MASS = 10.0;
double PELLET_MASS = 0.25;
rgb_color_t YELLOW = {1.0, 1.0, 0.0};

double PAC_ACCELERATION = 100.0;
double INITIAL_VELOCITY = 0.0;

typedef struct state {
  scene_t *scene;
  double time;
} state_t;

void handler(char key, key_event_type_t type, double held_time,
             state_t *state) {
  body_t *pacman = scene_get_body(state->scene, (size_t)0);
  if (type == KEY_PRESSED) {
    switch (key) {
    case UP_ARROW: {
      body_set_rotation(pacman, M_PI / 2);
      vector_t up_vel = {0.0, INITIAL_VELOCITY + held_time * PAC_ACCELERATION};
      body_set_velocity(pacman, up_vel);
      break;
    }
    case DOWN_ARROW: {
      body_set_rotation(pacman, M_PI * 3 / 2);
      vector_t down_vel = {0.0,
                           -INITIAL_VELOCITY - held_time * PAC_ACCELERATION};
      body_set_velocity(pacman, down_vel);
      break;
    }
    case RIGHT_ARROW: {
      body_set_rotation(pacman, 0.0);
      vector_t right_vel = {INITIAL_VELOCITY + held_time * PAC_ACCELERATION,
                            0.0};
      body_set_velocity(pacman, right_vel);
      break;
    }
    case LEFT_ARROW: {
      body_set_rotation(pacman, M_PI);
      vector_t left_vel = {-INITIAL_VELOCITY - held_time * PAC_ACCELERATION,
                           0.0};
      body_set_velocity(pacman, left_vel);
      break;
    }
    }
  }
}

void check_pacman_wrap(scene_t *scene) {
  body_t *pacman = scene_get_body(scene, (size_t)0);
  vector_t centroid = body_get_centroid(pacman);
  if (centroid.x + PACMAN_RADIUS < 0) {
    body_set_centroid(pacman, (vector_t){MAX_WIDTH - centroid.x, centroid.y});
  } else if (centroid.x - PACMAN_RADIUS > MAX_WIDTH) {
    body_set_centroid(
        pacman,
        (vector_t){centroid.x - MAX_WIDTH - PACMAN_RADIUS * 2, centroid.y});
  } else if (centroid.y + PACMAN_RADIUS < 0) {
    body_set_centroid(pacman, (vector_t){centroid.x, MAX_HEIGHT - centroid.y});
  } else if (centroid.y - PACMAN_RADIUS > MAX_HEIGHT) {
    body_set_centroid(pacman, (vector_t){centroid.x, centroid.y - MAX_HEIGHT -
                                                         PACMAN_RADIUS * 2});
  }
}

body_t *make_pacman(vector_t center, double length) {
  list_t *shape = list_init(CIRCLE_POINTS + 1, (free_func_t)free);

  for (size_t i = 0; i < CIRCLE_POINTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);

    point->x = center.x;
    point->y = center.y + length;

    list_add(shape, point);
    polygon_rotate(shape, -1 * M_PI / 180, center);
  }

  polygon_rotate(shape, -1 * M_PI / 6 - M_PI / 2, center);

  vector_t *center_point = malloc(sizeof(vector_t));
  assert(center_point != NULL);
  center_point->x = center.x;
  center_point->y = center.y;
  list_add(shape, center_point);

  body_t *pacman = body_init(shape, INITIAL_MASS, YELLOW);

  return pacman;
}

body_t *make_pellet(vector_t center, double length) {
  list_t *shape = list_init(PELLET_POINTS, (free_func_t)free);

  for (size_t i = 0; i < PELLET_POINTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);

    point->x = center.x;
    point->y = center.y + length;

    list_add(shape, point);
    polygon_rotate(shape, -M_PI / (PELLET_POINTS / 2), center);
  }

  body_t *pellet = body_init(shape, PELLET_MASS, YELLOW);

  return pellet;
}

void spawn_pellet(scene_t *scene) {
  vector_t centroid = {rand_num(0.0, MAX_WIDTH), rand_num(0.0, MAX_HEIGHT)};
  body_t *pellet = make_pellet(centroid, PELLET_RADIUS);
  scene_add_body(scene, pellet);
}

void eat_pellets(scene_t *scene) {
  if (scene_bodies(scene) > 1) {
    body_t *pacman = scene_get_body(scene, (size_t)0);
    vector_t pacman_centroid = body_get_centroid(pacman);
    size_t num_bodies = scene_bodies(scene);
    for (size_t i = 1; i < num_bodies; i++) {
      body_t *pellet = scene_get_body(scene, i);
      vector_t pellet_centroid = body_get_centroid(pellet);
      size_t xdist = fabs(pacman_centroid.x - pellet_centroid.x);
      size_t ydist = fabs(pacman_centroid.y - pellet_centroid.y);
      size_t distance = sqrt((xdist * xdist) + (ydist * ydist));
      if (distance < 4 * PACMAN_RADIUS / 5) {
        body_combine_mass(pacman, pellet);
        scene_remove_body(scene, i);
        i--;
        num_bodies--;
      }
    }
  }
}

/**
 * Initializes sdl as well as the variables needed
 * Creates and stores all necessary variables for the demo in a created state
 * variable Returns the pointer to this state (This is the state emscripten_main
 * and emscripten_free work with)
 */
state_t *emscripten_init() {
  vector_t min = VEC_ZERO;
  vector_t max = {MAX_WIDTH, MAX_HEIGHT};
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);

  vector_t center = {MAX_WIDTH / 2, MAX_HEIGHT / 2};

  body_t *pacman = make_pacman(center, PACMAN_RADIUS);

  state->scene = scene_init();
  scene_add_body(state->scene, pacman);

  for (size_t i = 0; i < NUMBER_OF_PELLETS; i++) {
    spawn_pellet(state->scene);
  }

  return state;
}

/**
 * Called on each tick of the program
 * Updates the state variables and display as necessary, depending on the time
 * that has passed
 */
void emscripten_main(state_t *state) {
  sdl_on_key((key_handler_t)handler);
  double dt = time_since_last_tick();
  state->time += dt;
  if (state->time >= SPAWN_TIME) {
    spawn_pellet(state->scene);
    state->time = 0.0;
  }
  eat_pellets(state->scene);
  check_pacman_wrap(state->scene);
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}

/**
 * Frees anything allocated in the demo
 * Should free everything in state as well as state itself.
 */
void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
