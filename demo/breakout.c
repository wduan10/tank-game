#include "body.h"
#include "collision.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "star.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

double MAX_WIDTH = 820.0;
double MAX_HEIGHT = 1000.0;

// color
rgb_color_t rainbow[] = {{1, 0, 0},          {1, 0.498, 0},   {1, 1, 0},
                         {0.56, 0.93, 0.56}, {0, 1, 0},       {0.69, 0.85, 0.9},
                         {0, 0, 1},          {0.29, 0, 0.90}, {0.58, 0, 0.83},
                         {1, 0, 1}};
rgb_color_t RED = {1, 0, 0};

// brick
double BRICK_HEIGHT = 20.0;
double BRICK_WIDTH = 70.0;
size_t NUM_BRICKS = 30;
double BRICK_HORZ_SPACING = 4.0;
double BRICK_VERT_SPACING = 10.0;
double BRICK_MASS = INFINITY;
int BRICK_TYPE = 1;
rgb_color_t BRICK_COLOR = {0.5, 0.5, 0.5};

// ball
double CIRCLE_POINTS = 300.0;
double BALL_MASS = 2.0;
double BALL_RADIUS = 8.0;
int BALL_TYPE = 2;
double BALL_VELOCITY = 1000.0;
double BALL_ANGLE = M_PI / 3;
double SPAWN_TIME = 5.0;

// player
double PLAYER_WIDTH = 100.0;
double PLAYER_VELOCITY = 700.0;
double PLAYER_MASS = INFINITY;
int PLAYER_TYPE = 3;
double PLAYER_ELASTICITY = 1.0;

// wall
double WALL_MASS = INFINITY;
rgb_color_t WALL_COLOR = {1, 1, 1};
double WALL_WIDTH = 30.0;
double WALL_HEIGHT = 1000.0;
int WALL_TYPE = 4;
double WALL_ELASTICITY = 1.0;

typedef struct state {
  scene_t *scene;
  double time;
} state_t;

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
  if (center.x + PLAYER_WIDTH / 2 + BRICK_HORZ_SPACING >= MAX_WIDTH) {
    if (vel.x > 0) {
      body_set_velocity(player, (vector_t){0.0, 0.0});
    }
  } else if (center.x - PLAYER_WIDTH / 2 + BRICK_HORZ_SPACING <= 0) {
    if (vel.x < 0) {
      body_set_velocity(player, (vector_t){0.0, 0.0});
    }
  }
}

list_t *make_ball(vector_t center, double length) {
  list_t *ball = list_init(CIRCLE_POINTS, (free_func_t)free);

  for (size_t i = 0; i < CIRCLE_POINTS; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);

    point->x = center.x;
    point->y = center.y + length;

    list_add(ball, point);
    polygon_rotate(ball, -M_PI / (CIRCLE_POINTS / 2), center);
  }
  return ball;
}

list_t *make_rectangle(vector_t corner, double width, double height) {
  list_t *wall = list_init(4, (free_func_t)free);
  vector_t *point1 = malloc(sizeof(vector_t));
  assert(point1 != NULL);
  point1->x = corner.x;
  point1->y = corner.y;
  list_add(wall, point1);
  vector_t *point2 = malloc(sizeof(vector_t));
  assert(point2 != NULL);
  point2->x = corner.x;
  point2->y = corner.y - height;
  list_add(wall, point2);
  vector_t *point3 = malloc(sizeof(vector_t));
  assert(point3 != NULL);
  point3->x = corner.x + width;
  point3->y = corner.y - height;
  list_add(wall, point3);
  vector_t *point4 = malloc(sizeof(vector_t));
  assert(point4 != NULL);
  point4->x = corner.x + width;
  point4->y = corner.y;
  list_add(wall, point4);
  return wall;
}

void add_walls(scene_t *scene) {
  int *type = malloc(sizeof(int));
  *type = WALL_TYPE;
  vector_t corner1 = {-WALL_WIDTH, WALL_HEIGHT};
  list_t *wall1_points = make_rectangle(corner1, WALL_WIDTH, WALL_HEIGHT);
  body_t *wall1 = body_init_with_info(wall1_points, WALL_MASS, WALL_COLOR, type,
                                      (free_func_t)free);
  scene_add_body(scene, wall1);

  vector_t corner2 = {0.0, WALL_HEIGHT + WALL_WIDTH};
  list_t *wall2_points = make_rectangle(corner2, MAX_WIDTH, WALL_WIDTH);
  body_t *wall2 = body_init_with_info(wall2_points, WALL_MASS, WALL_COLOR, type,
                                      (free_func_t)free);
  scene_add_body(scene, wall2);

  vector_t corner3 = {MAX_WIDTH, WALL_HEIGHT};
  list_t *wall3_points = make_rectangle(corner3, WALL_WIDTH, WALL_HEIGHT);
  body_t *wall3 = body_init_with_info(wall3_points, WALL_MASS, WALL_COLOR, type,
                                      (free_func_t)free);
  scene_add_body(scene, wall3);

  vector_t corner4 = {0.0, -WALL_WIDTH};
  list_t *wall4_points = make_rectangle(corner4, MAX_WIDTH, WALL_WIDTH);
  body_t *wall4 = body_init_with_info(wall4_points, WALL_MASS, WALL_COLOR, type,
                                      (free_func_t)free);
  scene_add_body(scene, wall4);
}

void make_bricks(state_t *state) {
  for (size_t i = 1; i < 4; i++) {
    for (size_t j = 0; j < NUM_BRICKS / 3; j++) {
      double x_coord = j * MAX_WIDTH / (NUM_BRICKS / 3) + BRICK_HORZ_SPACING;
      double y_coord =
          MAX_HEIGHT - i * BRICK_VERT_SPACING - (i - 1) * BRICK_HEIGHT;
      vector_t corner = {x_coord, y_coord};
      list_t *points = make_rectangle(corner, BRICK_WIDTH, BRICK_HEIGHT);
      int *type1 = malloc(sizeof(int));
      *type1 = BRICK_TYPE;
      body_t *brick = body_init_with_info(points, BRICK_MASS, rainbow[j], type1,
                                          (free_func_t)free);
      scene_add_body(state->scene, brick);
    }
  }
}

void player_init(state_t *state) {
  vector_t corner = {MAX_WIDTH / 2 - BRICK_WIDTH / 2, 1.5 * BRICK_HEIGHT};
  list_t *points = make_rectangle(corner, PLAYER_WIDTH, BRICK_HEIGHT);
  int *type3 = malloc(sizeof(int));
  *type3 = PLAYER_TYPE;
  body_t *player =
      body_init_with_info(points, PLAYER_MASS, RED, type3, (free_func_t)free);
  scene_add_body(state->scene, player);
}

body_t *ball_init(state_t *state) {
  vector_t center = {MAX_WIDTH / 2, MAX_HEIGHT / 10};
  list_t *circle = make_ball(center, BALL_RADIUS);
  int *type2 = malloc(sizeof(int));
  *type2 = BALL_TYPE;
  body_t *ball =
      body_init_with_info(circle, BALL_MASS, RED, type2, (free_func_t)free);
  vector_t initial_velocity = {BALL_VELOCITY * cos(BALL_ANGLE),
                               BALL_VELOCITY * sin(BALL_ANGLE)};
  body_set_velocity(ball, initial_velocity);
  scene_add_body(state->scene, ball);
  return ball;
}

void create_collisions(state_t *state, body_t *ball) {
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    if (i != 1) {
      body_t *body = scene_get_body(state->scene, i);
      if (*(int *)body_get_info(body) == BRICK_TYPE) {
        create_physics_collision(state->scene, 1.0, ball, body);
        create_partial_destructive_collision(state->scene, ball, body);
      } else if (*(int *)body_get_info(body) == WALL_TYPE) {
        create_physics_collision(state->scene, WALL_ELASTICITY, ball, body);
      } else if (*(int *)body_get_info(body) == PLAYER_TYPE) {
        create_physics_collision(state->scene, PLAYER_ELASTICITY, ball, body);
      }
    }
  }
}

state_t *emscripten_init() {
  vector_t min = VEC_ZERO;
  vector_t max = {MAX_WIDTH, MAX_HEIGHT};
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);

  state->scene = scene_init();

  // initializes the player
  player_init(state);

  // initializes the ball
  body_t *ball = ball_init(state);

  // initializes the bricks
  make_bricks(state);

  // initializes walls
  add_walls(state->scene);

  create_collisions(state, ball);

  return state;
}

bool balls_left(state_t *state) {
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    if (*(int *)body_get_info(body) == BALL_TYPE) {
      return true;
    }
  }
  return false;
}

bool ball_hit_ground(state_t *state) {
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    if (*(int *)body_get_info(body) == BALL_TYPE) {
      if (body_get_centroid(body).y < 0) {
        scene_remove_body(state->scene, i);
      }
    }
  }
  if (balls_left(state)) {
    return false;
  }
  return true;
}

void reset(state_t *state, double dt) {
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    scene_remove_body(state->scene, i);
  }

  // initializes the ball
  body_t *ball = ball_init(state);

  // initializes the bricks
  make_bricks(state);

  // initializes walls
  add_walls(state->scene);

  create_collisions(state, ball);

  scene_tick(state->scene, dt);
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  sdl_on_key((key_handler_t)handler);
  check_player_boundary(state->scene);
  state->time += dt;
  if (state->time >= SPAWN_TIME) {
    body_t *ball = ball_init(state);
    create_collisions(state, ball);
    state->time = 0.0;
  }
  if (ball_hit_ground(state)) {
    reset(state, dt);
  }
  scene_tick(state->scene, dt);
  sdl_render_scene(state->scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}