#include "body.h"
#include "collision.h"
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

size_t CIRCLE_POINTS = 50;

// player
size_t PLAYER_POINTS = 100;
double PLAYER_MAX_RADIUS = 45.0;
double PLAYER_MASS = 200.0;
double PLAYER_VELOCITY = 250.0;
double TIME_BETWEEN_SHOTS = 0.3;
rgb_color_t PLAYER_COLOR = {0.0, 1.0, 0.0};
int PLAYER_TYPE = 1;

// bullet
double BULLET_HEIGHT = 8.0;
double BULLET_WIDTH = 3.0;
double BULLET_MASS = 5.0;
double BULLET_VELOCITY = 250.0;
rgb_color_t INVADER_BULLET_COLOR = {0.5, 0.5, 0.5};
rgb_color_t PLAYER_BULLET_COLOR = {0.0, 1.0, 0.0};
double BULLET_SPAWN_TIME = 0.5;
int BULLET_TYPE = 2;

// invader
double INVADER_RADIUS = 35.0;
double INVADER_VERT_SPACING = 50.0;
double INVADER_MASS = 15.0;
double INVADER_VELOCITY = 80.0;
size_t NUM_INVADERS = 24;
rgb_color_t INVADER_COLOR = {0.5, 0.5, 0.5};
int INVADER_TYPE = 3;

double GRAVITY_CONSTANT = 500;

typedef struct state {
  scene_t *scene;
  double time;
  size_t remaining_invaders;
  bool reload_ready;
  double time_since_last_shot;
} state_t;

list_t *make_player(vector_t center, double length) {
  list_t *shape = list_init(PLAYER_POINTS, (free_func_t)free);

  for (size_t i = 0; i < PLAYER_POINTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);

    point->x = center.x + length * cos(2 * M_PI * i / PLAYER_POINTS);
    point->y = center.y + length / 3 * sin(2 * M_PI * i / PLAYER_POINTS);

    list_add(shape, point);
  }
  return shape;
}

list_t *make_bullet(vector_t corner) {
  list_t *shape = list_init(4, (free_func_t)free);
  vector_t *point1 = malloc(sizeof(vector_t));
  assert(point1 != NULL);
  point1->x = corner.x;
  point1->y = corner.y;
  list_add(shape, point1);
  vector_t *point2 = malloc(sizeof(vector_t));
  assert(point2 != NULL);
  point2->x = corner.x;
  point2->y = corner.y - BULLET_HEIGHT;
  list_add(shape, point2);
  vector_t *point3 = malloc(sizeof(vector_t));
  assert(point3 != NULL);
  point3->x = corner.x + BULLET_WIDTH;
  point3->y = corner.y - BULLET_HEIGHT;
  list_add(shape, point3);
  vector_t *point4 = malloc(sizeof(vector_t));
  assert(point4 != NULL);
  point4->x = corner.x + BULLET_WIDTH;
  point4->y = corner.y;
  list_add(shape, point4);
  return shape;
}

list_t *make_invader(vector_t center, double length) {
  list_t *shape = list_init(CIRCLE_POINTS + 1, (free_func_t)free);
  for (size_t i = 0; i < CIRCLE_POINTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);

    point->x = center.x;
    point->y = center.y + length;

    list_add(shape, point);
    polygon_rotate(shape, -1 * M_PI * 5 / (6 * CIRCLE_POINTS), center);
  }
  polygon_rotate(shape, -1 * M_PI / 12 - 3 * M_PI / 2, center);

  vector_t *center_point = malloc(sizeof(vector_t));
  assert(center_point != NULL);
  center_point->x = center.x;
  center_point->y = center.y;
  list_add(shape, center_point);

  return shape;
}

void check_wrap_around(scene_t *scene) {
  for (size_t i = 1; i < scene_bodies(scene); i++) {
    body_t *body = scene_get_body(scene, i);

    if (*(int *)body_get_info(body) == INVADER_TYPE) {
      vector_t centroid = body_get_centroid(body);
      vector_t old_velocity = body_get_velocity(body);

      if (centroid.x + cos(M_PI / 12) * INVADER_RADIUS > MAX_WIDTH) {
        body_set_centroid(
            body, (vector_t){MAX_WIDTH - cos(M_PI / 12) * INVADER_RADIUS,
                             centroid.y - INVADER_VERT_SPACING * 3});
        body_set_velocity(body, vec_negate(old_velocity));
      } else if (centroid.x + cos(11 * M_PI / 12) * INVADER_RADIUS <= 0) {
        body_set_centroid(body,
                          (vector_t){cos(M_PI / 12) * INVADER_RADIUS,
                                     centroid.y - INVADER_VERT_SPACING * 3});
        body_set_velocity(body, vec_negate(old_velocity));
      }
    }
  }
}

void handler(char key, key_event_type_t type, double held_time,
             state_t *state) {
  body_t *player = scene_get_body(state->scene, (size_t)0);
  if (type == KEY_PRESSED) {
    switch (key) {
    case RIGHT_ARROW: {
      body_set_velocity(player, (vector_t){PLAYER_VELOCITY, 0});
      break;
    }
    case LEFT_ARROW: {
      body_set_velocity(player, (vector_t){-1 * PLAYER_VELOCITY, 0});
      break;
    }
    case SPACE: {
      if (state->reload_ready) {
        state->reload_ready = false;
        state->time_since_last_shot = 0.0;
        vector_t spawn_point = body_get_centroid(player);
        spawn_point.y += 20.0;
        list_t *bullet_points = make_bullet(spawn_point);
        int *type = malloc(sizeof(int));
        *type = BULLET_TYPE;
        body_t *bullet =
            body_init_with_info(bullet_points, BULLET_MASS, PLAYER_BULLET_COLOR,
                                type, (free_func_t)free);
        for (size_t i = 1; i < state->remaining_invaders + 1; i++) {
          create_destructive_collision(state->scene,
                                       scene_get_body(state->scene, i), bullet);
        }
        scene_add_body(state->scene, bullet);
        body_set_velocity(bullet, (vector_t){0, BULLET_VELOCITY});
      }
      break;
    }
    }
  }
  if (type == KEY_RELEASED) {
    switch (key) {
    case RIGHT_ARROW: {
      if (body_get_velocity(player).x > 0) {
        body_set_velocity(player, (vector_t){0, 0});
      }
      break;
    }
    case LEFT_ARROW: {
      if (body_get_velocity(player).x < 0) {
        body_set_velocity(player, (vector_t){0, 0});
      }
      break;
    }
    }
  }
}

void check_player_boundary(scene_t *scene) {
  body_t *player = scene_get_body(scene, (size_t)0);
  vector_t center = body_get_centroid(player);
  vector_t vel = body_get_velocity(player);
  if (center.x + PLAYER_MAX_RADIUS >= MAX_WIDTH) {
    if (vel.x > 0) {
      body_set_velocity(player, (vector_t){0.0, 0.0});
    }
  } else if (center.x - PLAYER_MAX_RADIUS <= 0) {
    if (vel.x < 0) {
      body_set_velocity(player, (vector_t){0.0, 0.0});
    }
  }
}

void check_bullet_boundary(state_t *state) {
  size_t num_bodies = scene_bodies(state->scene);
  for (size_t i = state->remaining_invaders + 1; i < num_bodies; i++) {
    body_t *bullet = scene_get_body(state->scene, i);
    vector_t center = body_get_centroid(bullet);
    if (center.y - BULLET_HEIGHT > MAX_HEIGHT || center.y + BULLET_HEIGHT < 0) {
      body_remove(bullet);
    }
  }
}

void count_invaders(state_t *state) {
  scene_t *scene = state->scene;
  size_t num_invaders = 0;
  size_t num_bodies = scene_bodies(scene);
  for (size_t i = 0; i < num_bodies; i++) {
    body_t *body = scene_get_body(scene, i);
    if (*(int *)body_get_info(body) == INVADER_TYPE) {
      num_invaders++;
    }
  }
  state->remaining_invaders = num_invaders;
}

bool invaders_out_of_bounds(state_t *state) {
  body_t *lowest_invader =
      scene_get_body(state->scene, (size_t)state->remaining_invaders);
  vector_t centroid = body_get_centroid(lowest_invader);
  if (centroid.y - INVADER_RADIUS < 0) {
    return true;
  }
  return false;
}

bool player_is_shot(state_t *state) {
  body_t *first_body = scene_get_body(state->scene, 0);
  if (*(int *)body_get_info(first_body) == PLAYER_TYPE) {
    return false;
  }
  return true;
}

/*
conditions are 1. player is shot
2. no invaders left
3. invaders reach bottom
*/
void check_game_over(state_t *state) {
  if (player_is_shot(state) || state->remaining_invaders == (size_t)0 ||
      invaders_out_of_bounds(state)) {
    exit(0);
  }
}

state_t *emscripten_init() {
  vector_t min = VEC_ZERO;
  vector_t max = {MAX_WIDTH, MAX_HEIGHT};
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);

  state->scene = scene_init();
  state->remaining_invaders = NUM_INVADERS;

  // creates the player
  int *type = malloc(sizeof(int));
  *type = PLAYER_TYPE;
  list_t *player = make_player((vector_t){MAX_WIDTH / 2, PLAYER_MAX_RADIUS / 2},
                               PLAYER_MAX_RADIUS);
  body_t *body_player = body_init_with_info(player, PLAYER_MASS, PLAYER_COLOR,
                                            type, (free_func_t)free);
  scene_add_body(state->scene, body_player);

  // creates the invaders
  for (size_t i = 1; i < 4; i++) {
    for (size_t j = 1; j <= NUM_INVADERS / 3; j++) {
      double x_coord = j * MAX_WIDTH / (NUM_INVADERS / 2);
      double y_coord = MAX_HEIGHT - i * (INVADER_VERT_SPACING);
      vector_t center = {x_coord, y_coord};
      list_t *space_invader = make_invader(center, INVADER_RADIUS);
      int *type = malloc(sizeof(int));
      *type = INVADER_TYPE;
      body_t *invader = body_init_with_info(
          space_invader, INVADER_MASS, INVADER_COLOR, type, (free_func_t)free);
      body_set_velocity(invader, (vector_t){INVADER_VELOCITY, 0});
      scene_add_body(state->scene, invader);
    }
  }
  return state;
}

void spawn_invader_bullet(state_t *state) {
  body_t *body =
      scene_get_body(state->scene, rand_num(1, state->remaining_invaders + 1));
  if (*(int *)body_get_info(body) == INVADER_TYPE) {
    vector_t bullet_corner = body_get_centroid(body);
    bullet_corner.y -= INVADER_RADIUS / 4;
    list_t *bullet = make_bullet(bullet_corner);
    int *type = malloc(sizeof(int));
    *type = BULLET_TYPE;
    body_t *invader_bullet = body_init_with_info(
        bullet, BULLET_MASS, INVADER_BULLET_COLOR, type, (free_func_t)free);
    body_set_velocity(invader_bullet, (vector_t){0, -1 * BULLET_VELOCITY});
    scene_add_body(state->scene, invader_bullet);
    create_destructive_collision(state->scene, scene_get_body(state->scene, 0),
                                 invader_bullet);
  }
}

void emscripten_main(state_t *state) {
  count_invaders(state);
  check_game_over(state);
  double dt = time_since_last_tick();
  sdl_on_key((key_handler_t)handler);
  state->time += dt;
  if (state->time >= BULLET_SPAWN_TIME) {
    spawn_invader_bullet(state);
    state->time = 0.0;
  }
  if (state->time_since_last_shot > TIME_BETWEEN_SHOTS) {
    state->reload_ready = true;
  } else if (!state->reload_ready) {
    state->time_since_last_shot += dt;
  }
  check_wrap_around(state->scene);
  check_player_boundary(state->scene);
  check_bullet_boundary(state);
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}