#include "list.h"
#include "polygon.h"
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
double LENGTH = 60.0;
double GRAVITATIONAL_CONSTANT = -900.81;
double DAMPING_CONSTANT = 0.85;

double DEFAULT_SPEED = 200.0;
double DEFAULT_ANGLE = M_PI / 4;
double DEFAULT_ROTATION_SPEED = M_PI / 8;

bool is_out_of_bounds(star_t *star);

/**
 * Stores the demo state
 * Use this to store any variable needed every 'tick' of your demo
 */
typedef struct state {
  list_t *stars;
  double time;
  int num_stars;
  int vertices;
} state_t;

bool is_out_of_bounds(star_t *star) {
  list_t *polygon = get_star_polygon(star);
  for (int i = 0; i < list_size(polygon); i++) {
    vector_t *curr = list_get(polygon, i);
    if (curr->x < MAX_WIDTH) {
      return false;
    }
  }
  return true;
}

star_t *move_star(int i, state_t *state, double dt) {
  star_t *star = list_get(state->stars, i);
  vector_t *vel = get_star_velocity(star);
  vel->y = vel->y + GRAVITATIONAL_CONSTANT * dt;
  vector_t vec = {dt * vel->x, dt * vel->y};

  star_translate(star, vec);
  star_rotate(star, dt * get_star_rotation(star),
              polygon_centroid(get_star_polygon(star)));

  if (!get_star_just_moved(star)) {
    if (fabs(vel->y) > dt * GRAVITATIONAL_CONSTANT) {
      for (size_t j = 0; j < list_size(get_star_polygon(star)); j++) {
        vector_t *vector = list_get(get_star_polygon(star), j);
        if (vector->y <= 0.0) {
          vel->y = -1 * vel->y * DAMPING_CONSTANT;
          set_star_just_moved(star, true);
          break;
        }
      }
    }
  } else {
    set_star_just_moved(star, false);
  }
  sdl_draw_polygon(get_star_polygon(star),
                   (rgb_color_t){get_star_red_val(star),
                                 get_star_green_val(star),
                                 get_star_blue_val(star)});
  return star;
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
  state->num_stars = 1;
  state->time = 0.0;
  state->vertices = 3;

  star_t *initial_poly =
      star_init(state->vertices, MAX_HEIGHT, LENGTH, DEFAULT_SPEED,
                DEFAULT_ANGLE, DEFAULT_ROTATION_SPEED);
  state->stars = list_init(10, free);
  assert(state->stars != NULL);
  list_add(state->stars, initial_poly);

  return state;
}

/**
 * Called on each tick of the program
 * Updates the state variables and display as necessary, depending on the time
 * that has passed
 */
void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  state->time += dt;
  if (state->time >= 2.0) {
    state->vertices++;
    state->num_stars++;

    star_t *star = star_init(state->vertices, MAX_HEIGHT, LENGTH, DEFAULT_SPEED,
                             DEFAULT_ANGLE, DEFAULT_ROTATION_SPEED);
    list_add(state->stars, star);

    state->time = 0.0;
  }

  for (size_t i = 0; i < state->num_stars; i++) {
    star_t *star = move_star(i, state, dt);
    bool out_of_bounds = is_out_of_bounds(star);
    if (out_of_bounds == true) {
      for (size_t j = i + 1; j < state->num_stars; j++) {
        star_t *star = list_get(state->stars, j);
        star_t *replaced = list_replace(state->stars, j - 1, star);
        if (j == i + 1) {
          star_free(replaced);
        }
      }
      list_remove(state->stars, list_size(state->stars) - 1);
      state->num_stars--;
      i--;
    }
  }
  sdl_show();
}

/**
 * Frees anything allocated in the demo
 * Should free everything in state as well as state itself.
 */
void emscripten_free(state_t *state) {
  list_free(state->stars);
  free(state);
}
