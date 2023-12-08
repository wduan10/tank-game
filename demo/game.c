#include "body.h"
#include "collision.h"
#include "forces.h"
#include "list.h"
#include "map.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "star.h"
#include "state.h"
#include "text.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// types of different bodies
const size_t WALL_TYPE = 0;
const size_t BULLET_TYPE = 1;
const size_t SNIPER_BULLET_TYPE = 10;
const size_t GRAVITY_BULLET_TYPE = 13;
const size_t GATLING_BULLET_TYPE = 11;
const size_t DEFAULT_TANK_TYPE = 2;
const size_t GRAVITY_TANK_TYPE = 3;
const size_t SNIPER_TANK_TYPE = 4;
const size_t HEALTH_BAR_TYPE = 6;
const size_t GATLING_TANK_TYPE = 7;

int FONT_SIZE = 50;
int TITLE_SIZE = 100;
int TANK_SELECT_SIZE = 25;
double CIRCLE_POINTS = 300.0;

// DEATH animation time
double DEATH_PAUSE_TIME = 0.2;

const double MAX_WIDTH_GAME = 1600.0;
const double MAX_HEIGHT_GAME = 1300.0;

// elasticity between tank
double TANKS_ELASTICITY = 3.0;

// default tank stats
double DEFAULT_TANK_VELOCITY = 160.0;
double DEFAULT_TANK_SIDE_LENGTH = 80.0;
double DEFAULT_TANK_MASS = 1000.0;
double DEFAULT_TANK_ROTATION_SPEED = M_PI / 2;
double DEFAULT_TANK_MAX_HEALTH = 50.0;

double BULLET_HEIGHT = 25.0;
double BULLET_WIDTH = 10.0;
double BULLET_VELOCITY = 300.0;
double RELOAD_SPEED = 1.0;

// GRAVITY tank stats
size_t GRAVITY_TANK_POINTS = 6;
double GRAVITY_TANK_VELOCITY = 140.0;
double GRAVITY_TANK_SIDE_LENGTH = 60.0;
double GRAVITY_TANK_MASS = 1000.0;
double GRAVITY_TANK_ROTATION_SPEED = M_PI * 3 / 4;
double GRAVITY_TANK_MAX_HEALTH = 50.0;
double GRAVITY_TANK_RELOAD_SPEED = 1.75;

double GRAVITY_BULLET_VELOCITY = 300.0;
double GRAVITY_TANK_STRENGTH = 5000.0;

// SNIPER tank stats
double SNIPER_TANK_VELOCITY = 100.0;
double SNIPER_TANK_SIDE_LENGTH = 60.0;
double SNIPER_TANK_MASS = 1000.0;
double SNIPER_TANK_ROTATION_SPEED = M_PI * 2 / 5;
double SNIPER_TANK_MAX_HEALTH = 40.0; // something wrong w/ health
const double SNIPER_TANK_DAMAGE = 25.0;

double SNIPER_RELOAD_SPEED = 2.5;
double SNIPER_BULLET_HEIGHT = 25.0;
double SNIPER_BULLET_WIDTH = 10.0;
double SNIPER_BULLET_VELOCITY = 500.0;

// GATLING tank stats
double GATLING_TANK_VELOCITY = 100.0;
double GATLING_TANK_SIDE_LENGTH = 60.0;
double GATLING_TANK_MASS = 1000.0;
double GATLING_TANK_ROTATION_SPEED = M_PI * 3 / 5;
double GATLING_TANK_MAX_HEALTH = 80.0;
const double GATLING_TANK_DAMAGE = 25.0;

double GATLING_RELOAD_SPEED = 0.4;
double GATLING_BULLET_HEIGHT = 25.0;
double GATLING_BULLET_WIDTH = 10.0;
double GATLING_BULLET_VELOCITY = 400.0;

// bullet damage
const double BULLET_DAMAGE = 10.0;
const double GRAVITY_BULLET_DAMAGE = 15.0;
const double SNIPER_BULLET_DAMAGE = 25.0;
const double GATLING_BULLET_DAMAGE = 5.0;

// default bullet characteristics
double BULLET_MASS = 5.0;
double BULLET_DISAPPEAR_TIME = 10.0;

double HEALTH_BAR_WIDTH = 500.0;
double HEALTH_BAR_HEIGHT = 50.0;
double HEALTH_BAR_OFFSET_HORIZONTAL = 50.0;
double HEALTH_BAR_OFFSET_VERTICAL = 25.0;

double COLLISION_ELASTICITY = 20.0;

// menu stats
double BUTTON_X_MIN = 404.0;
double BUTTON_X_MAX = 598.0;
double START_BUTTON_Y_MIN = 194.0;
double START_BUTTON_Y_MAX = 262.0;
double OPTIONS_BUTTON_Y_MIN = 289.0;
double OPTIONS_BUTTON_Y_MAX = 359.0;

// options stats

double GAMMA = 1.0;

// COLORS:
rgb_color_t PLAYER1_COLOR = {1.0, 0.0, 0.0};
rgb_color_t PLAYER1_COLOR_SIMILAR = {0.5, 0.0, 0.0};
rgb_color_t PLAYER2_COLOR = {0.0, 1.0, 0.0};
rgb_color_t PLAYER2_COLOR_SIMILAR = {0.0, 0.5, 0.0};
rgb_color_t LIGHT_GREY = {0.86, 0.86, 0.86};
rgb_color_t SELECTED_TANK = {0.3, 0.3, 0.3};
rgb_color_t GREEN = {0.0, 1.0, 0.0};
rgb_color_t BLUE = {0.0, 0.0, 1.0};
rgb_color_t DARKER_BLUE = {0.0, 0.0, 0.6};
rgb_color_t BLACK = {0.0, 0.0, 0.0};
rgb_color_t RED = {1.0, 0.0, 0.0};
rgb_color_t DARKER_RED = {0.6, 0.0, 0.0};
rgb_color_t YELLOW = {0.9, 0.9, 0.1};
rgb_color_t DARKER_YELLOW = {0.6, 0.6, 0.0};
rgb_color_t FOREST_GREEN = {0.2, 0.6, 0.2};
rgb_color_t DARKER_FOREST_GREEN = {0.00, 0.45, 0.0};
rgb_color_t DARKER_FOREST_GREEN_POLY = {0.05, 0.35, 0.05};
rgb_color_t FOREST_GREEN_POLY = {0.13, 0.55, 0.13};
rgb_color_t TEAL = {0.0, 0.8, 0.8};
SDL_Color SDL_WHITE = {255, 255, 255, 255};
rgb_color_t SLATE_GREY = {0.72, 0.79, 0.89};
SDL_Color SDL_BLACK = {0, 0, 0, 255};
SDL_Color SDL_FOREST_GREEN = {74, 103, 65, 255};
SDL_Color SHADE_GREEN = {74, 170, 65, 255};
SDL_Color SDL_RED = {255, 20, 20, 255};
SDL_Color SDL_GREEN = {20, 255, 20, 255};
SDL_Color NUM_PLAYERS_COLOR = {15, 15, 240, 255};

typedef struct state {
  scene_t *scene;
  double time;
  size_t player1_tank_type;
  size_t player2_tank_type;
  int player1_score;
  int player2_score;
  bool singleplayer;
  bool is_menu;
  bool is_options;
  bool is_round_end;
  text_t *text;
  text_t *title;
  text_t *select_tank;
  text_t *scoreboard;
} state_t;

list_t *make_half_circle(vector_t center, double radius) {
  list_t *shape = list_init(18, (free_func_t)free);
  for (size_t i = 0; i < 18; i++) {
    vector_t *point = malloc(sizeof(vector_t));
    assert(point != NULL);
    point->x = center.x + radius;
    point->y = center.y;
    list_add(shape, point);
    polygon_rotate(shape, M_PI / 18, center);
  }
  vector_t *point = malloc(sizeof(vector_t));
  assert(point != NULL);
  point->x = center.x + radius;
  point->y = center.y;
  list_add(shape, point);
  return shape;
}

list_t *make_heart(vector_t center, double length) {
  list_t *shape = list_init(100, (free_func_t)free);

  // create first half circle
  vector_t rotation_area1 = {center.x + length / 2, center.y};
  list_t *half_circle1 = make_half_circle(rotation_area1, length / 2);
  // have to use int here since size_t is unsigned
  for (int i = (int)list_size(half_circle1) - 1; i >= 0; i--) {
    vector_t *point = list_get(half_circle1, i);
    list_add(shape, point);
  }

  // create second half circle
  vector_t rotation_area2 = {center.x - length / 2, center.y};
  list_t *half_circle2 = make_half_circle(rotation_area2, length / 2);
  for (int i = (int)list_size(half_circle2) - 1; i >= 0; i--) {
    vector_t *point = list_get(half_circle2, i);
    list_add(shape, point);
  }

  vector_t *bottom_point = malloc(sizeof(vector_t));
  *bottom_point = (vector_t){center.x, center.y - length};
  list_add(shape, bottom_point);
  return shape;
}

list_t *make_health_bar_p1(double health) {
  list_t *shape = list_init(4, (free_func_t)free);
  if (health < 0) {
    health = 0.0;
  }

  vector_t *point1 = malloc(sizeof(vector_t));
  assert(point1 != NULL);
  point1->x = health / DEFAULT_TANK_MAX_HEALTH * HEALTH_BAR_WIDTH +
              HEALTH_BAR_OFFSET_HORIZONTAL;
  point1->y = MAX_HEIGHT_GAME - HEALTH_BAR_HEIGHT - HEALTH_BAR_OFFSET_VERTICAL;
  list_add(shape, point1);

  vector_t *point2 = malloc(sizeof(vector_t));
  assert(point2 != NULL);
  point2->x = health / DEFAULT_TANK_MAX_HEALTH * HEALTH_BAR_WIDTH +
              HEALTH_BAR_OFFSET_HORIZONTAL;
  point2->y = MAX_HEIGHT_GAME - HEALTH_BAR_OFFSET_VERTICAL;
  list_add(shape, point2);

  vector_t *point3 = malloc(sizeof(vector_t));
  assert(point3 != NULL);
  point3->x = +HEALTH_BAR_OFFSET_HORIZONTAL;
  point3->y = MAX_HEIGHT_GAME - HEALTH_BAR_OFFSET_VERTICAL;
  list_add(shape, point3);
  vector_t *point4 = malloc(sizeof(vector_t));
  assert(point4 != NULL);
  point4->x = +HEALTH_BAR_OFFSET_HORIZONTAL;
  point4->y = MAX_HEIGHT_GAME - HEALTH_BAR_HEIGHT - HEALTH_BAR_OFFSET_VERTICAL;
  list_add(shape, point4);
  return shape;
}

list_t *make_health_bar_p2(double health) {
  list_t *shape = list_init(4, (free_func_t)free);
  if (health < 0) {
    health = 0.0;
  }

  vector_t *point1 = malloc(sizeof(vector_t));
  assert(point1 != NULL);
  point1->x = MAX_WIDTH_GAME -
              health / DEFAULT_TANK_MAX_HEALTH * HEALTH_BAR_WIDTH -
              HEALTH_BAR_OFFSET_HORIZONTAL;
  point1->y = MAX_HEIGHT_GAME - HEALTH_BAR_OFFSET_VERTICAL;
  list_add(shape, point1);

  vector_t *point2 = malloc(sizeof(vector_t));
  assert(point2 != NULL);
  point2->x = MAX_WIDTH_GAME -
              health / DEFAULT_TANK_MAX_HEALTH * HEALTH_BAR_WIDTH -
              HEALTH_BAR_OFFSET_HORIZONTAL;
  point2->y = MAX_HEIGHT_GAME - HEALTH_BAR_HEIGHT - HEALTH_BAR_OFFSET_VERTICAL;
  list_add(shape, point2);

  vector_t *point3 = malloc(sizeof(vector_t));
  assert(point3 != NULL);
  point3->x = MAX_WIDTH_GAME - HEALTH_BAR_OFFSET_HORIZONTAL;
  point3->y = MAX_HEIGHT_GAME - HEALTH_BAR_HEIGHT - HEALTH_BAR_OFFSET_VERTICAL;
  list_add(shape, point3);

  vector_t *point4 = malloc(sizeof(vector_t));
  assert(point4 != NULL);
  point4->x = MAX_WIDTH_GAME - HEALTH_BAR_OFFSET_HORIZONTAL;
  point4->y = MAX_HEIGHT_GAME - HEALTH_BAR_OFFSET_VERTICAL;
  list_add(shape, point4);
  return shape;
}
void init_sounds() {
  SDL_Init(SDL_INIT_AUDIO);
  Mix_Init(0);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  Mix_Music *music = Mix_LoadMUS("assets/upbeat_music.wav");
  Mix_PlayMusic(music, -1);
}

void bullet_shot_sound() {
  Mix_Chunk *bullet_sound = Mix_LoadWAV("assets/default_tank_sound.wav");
  Mix_PlayChannel(1, bullet_sound, 0);
}

void death_sound() {
  Mix_Chunk *death = Mix_LoadWAV("assets/death.wav");
  Mix_PlayChannel(1, death, 0);
}

void free_channel(int channel) { Mix_FreeChunk(Mix_GetChunk(channel)); }

list_t *make_bullet(vector_t edge) {
  list_t *shape = list_init(4, (free_func_t)free);
  vector_t *point1 = malloc(sizeof(vector_t));
  assert(point1 != NULL);
  point1->x = edge.x;
  point1->y = edge.y - BULLET_WIDTH / 2;
  list_add(shape, point1);

  vector_t *point2 = malloc(sizeof(vector_t));
  assert(point2 != NULL);
  point2->x = edge.x + BULLET_HEIGHT;
  point2->y = edge.y - BULLET_WIDTH / 2;
  list_add(shape, point2);

  vector_t *point3 = malloc(sizeof(vector_t));
  assert(point3 != NULL);
  point3->x = edge.x + BULLET_HEIGHT;
  point3->y = edge.y + BULLET_WIDTH / 2;
  list_add(shape, point3);

  vector_t *point4 = malloc(sizeof(vector_t));
  assert(point4 != NULL);
  point4->x = edge.x;
  point4->y = edge.y + BULLET_WIDTH / 2;
  list_add(shape, point4);
  return shape;
}

void handle_bullet(state_t *state, body_t *player, rgb_color_t color) {
  body_set_time(player, 0.0);
  vector_t spawn_point = body_get_centroid(player);
  list_t *bullet_points =
      make_bullet(spawn_point); // can change make bullet later for sniper
                                // bullet and machine gun bullet
  polygon_rotate(bullet_points, body_get_rotation(player),
                 body_get_centroid(player));
  vector_t player_dir = {cos(body_get_rotation(player)),
                         sin(body_get_rotation(player))};
  vector_t move_up =
      vec_multiply(DEFAULT_TANK_SIDE_LENGTH / 2 + 10, player_dir);
  polygon_translate(bullet_points, move_up);
  size_t *type = malloc(sizeof(size_t));
  double vel;
  if (*(size_t *)body_get_info(player) == DEFAULT_TANK_TYPE) {
    *type = BULLET_TYPE;
    vel = BULLET_VELOCITY;
  } else if (*(size_t *)body_get_info(player) == SNIPER_TANK_TYPE) {
    *type = SNIPER_BULLET_TYPE;
    vel = SNIPER_BULLET_VELOCITY;
  } else if (*(size_t *)body_get_info(player) == GATLING_TANK_TYPE) {
    *type = GATLING_BULLET_TYPE;
    vel = GATLING_BULLET_VELOCITY;
  } else if (*(size_t *)body_get_info(player) == GRAVITY_TANK_TYPE) {
    *type = GRAVITY_BULLET_TYPE;
    vel = GRAVITY_BULLET_VELOCITY;
  } else { // default
    *type = BULLET_TYPE;
    vel = BULLET_VELOCITY;
  }
  body_t *bullet = body_init_with_info(bullet_points, BULLET_MASS, color, type,
                                       (free_func_t)free);

  if (*(size_t *)body_get_info(player) == GRAVITY_TANK_TYPE) {
    if (scene_get_body(state->scene, 0) == player) {
      create_newtonian_gravity(state->scene, GRAVITY_TANK_STRENGTH,
                               scene_get_body(state->scene, 1), bullet);
      create_newtonian_gravity(state->scene, (-1 * GRAVITY_TANK_STRENGTH / 2),
                               scene_get_body(state->scene, 0), bullet);
    } else {
      create_newtonian_gravity(state->scene, GRAVITY_TANK_STRENGTH,
                               scene_get_body(state->scene, 0), bullet);
      create_newtonian_gravity(state->scene, (-1 * GRAVITY_TANK_STRENGTH / 2),
                               scene_get_body(state->scene, 1), bullet);
    }
  }
  body_set_rotation_empty(bullet, body_get_rotation(player));
  body_set_velocity(bullet, vec_multiply(vel, player_dir));
  body_set_time(bullet, 0.0);
  scene_add_body(state->scene, bullet);

  // create collision with tanks
  create_partial_destructive_collision(state->scene,
                                       scene_get_body(state->scene, 0), bullet);
  create_partial_destructive_collision(state->scene,
                                       scene_get_body(state->scene, 1), bullet);

  // add drag force
  create_drag(state->scene, GAMMA, bullet);

  // create collision with walls and other bullets
  for (size_t i = 2; i < scene_bodies(state->scene) - 1; i++) {
    body_t *body = scene_get_body(state->scene, i);
    if (*(size_t *)body_get_info(body) == BULLET_TYPE ||
        *(size_t *)body_get_info(body) == SNIPER_BULLET_TYPE ||
        *(size_t *)body_get_info(body) == GATLING_BULLET_TYPE ||
        *(size_t *)body_get_info(body) == GRAVITY_BULLET_TYPE) {
      create_destructive_collision(state->scene, body, bullet);
    } else if (*(size_t *)body_get_info(body) == RECTANGLE_OBSTACLE_TYPE ||
               *(size_t *)body_get_info(body) == TRIANGLE_OBSTACLE_TYPE) {
      create_physics_collision(state->scene, 1.0, bullet, body);
    }
  }
}

void tank_handler(char key, key_event_type_t type, double held_time,
                  state_t *state, body_t *player, rgb_color_t player_color) {
  if (*(size_t *)body_get_info(player) ==
      DEFAULT_TANK_TYPE) { // this is to handle the different tank types
    if (type == KEY_PRESSED) {
      switch (key) {
      case 'w': {
        body_set_magnitude(player, DEFAULT_TANK_VELOCITY);
        break;
      }
      case 's': {
        body_set_magnitude(player, -DEFAULT_TANK_VELOCITY);
        break;
      }
      case 'd': {
        body_set_rotation_speed(player, -DEFAULT_TANK_ROTATION_SPEED);
        break;
      }
      case 'a': {
        body_set_rotation_speed(player, DEFAULT_TANK_ROTATION_SPEED);
        break;
      }
      case 'r': {
        if (body_get_time(player) > RELOAD_SPEED) {
          handle_bullet(state, player, player_color);
          bullet_shot_sound();
        }
      }
      }
    }
  } else if (*(size_t *)body_get_info(player) ==
             GRAVITY_TANK_TYPE) { // handles GRAVITY tank
    if (type == KEY_PRESSED) {
      switch (key) {
      case 'w': {
        body_set_magnitude(player, GRAVITY_TANK_VELOCITY);
        break;
      }
      case 's': {
        body_set_magnitude(player, -GRAVITY_TANK_VELOCITY);
        break;
      }
      case 'd': {
        body_set_rotation_speed(player, -GRAVITY_TANK_ROTATION_SPEED);
        break;
      }
      case 'a': {
        body_set_rotation_speed(player, GRAVITY_TANK_ROTATION_SPEED);
        break;
      }
      case 'r': {
        if (body_get_time(player) > GRAVITY_TANK_RELOAD_SPEED) {
          handle_bullet(state, player, player_color);
          bullet_shot_sound();
        }
      }
      }
    }
  } else if (*(size_t *)body_get_info(player) == SNIPER_TANK_TYPE) {
    if (type == KEY_PRESSED) {
      switch (key) {
      case 'w': {
        body_set_magnitude(player, SNIPER_TANK_VELOCITY);
        break;
      }
      case 's': {
        body_set_magnitude(player, -SNIPER_TANK_VELOCITY);
        break;
      }
      case 'd': {
        body_set_rotation_speed(player, -SNIPER_TANK_ROTATION_SPEED);
        break;
      }
      case 'a': {
        body_set_rotation_speed(player, SNIPER_TANK_ROTATION_SPEED);
        break;
      }
      case 'r': {
        if (body_get_time(player) > SNIPER_RELOAD_SPEED) {
          handle_bullet(state, player, player_color);
          bullet_shot_sound();
        }
      }
      }
    }
  } else if (*(size_t *)body_get_info(player) == GATLING_TANK_TYPE) {
    if (type == KEY_PRESSED) {
      switch (key) {
      case 'w': {
        body_set_magnitude(player, GATLING_TANK_VELOCITY);
        break;
      }
      case 's': {
        body_set_magnitude(player, -GATLING_TANK_VELOCITY);
        break;
      }
      case 'd': {
        body_set_rotation_speed(player, -GATLING_TANK_ROTATION_SPEED);
        break;
      }
      case 'a': {
        body_set_rotation_speed(player, GATLING_TANK_ROTATION_SPEED);
        break;
      }
      case 'r': {
        if (body_get_time(player) > GATLING_RELOAD_SPEED) {
          handle_bullet(state, player, player_color);
          bullet_shot_sound();
        }
      }
      }
    }
  } else {
    // add other tanks after
  }
  if (type == KEY_RELEASED) {
    switch (key) {
    case 'w': {
      body_set_velocity(player, VEC_ZERO);
      body_set_magnitude(player, 0.0);
      break;
    }
    case 's': {
      body_set_velocity(player, VEC_ZERO);
      body_set_magnitude(player, 0.0);
      break;
    }
    case 'd': {
      body_set_rotation_speed(player, 0.0);
      break;
    }
    case 'a': {
      body_set_rotation_speed(player, 0.0);
      break;
    }
    }
  }
}
void tank_handler2(char key, key_event_type_t type, double held_time,
                   state_t *state, body_t *player, rgb_color_t player_color) {
  if (*(size_t *)body_get_info(player) ==
      DEFAULT_TANK_TYPE) { // this is to handle the different tank types
    if (type == KEY_PRESSED) {
      switch (key) {
      case UP_ARROW: {
        body_set_magnitude(player, DEFAULT_TANK_VELOCITY);
        break;
      }
      case DOWN_ARROW: {
        body_set_magnitude(player, -DEFAULT_TANK_VELOCITY);
        break;
      }
      case RIGHT_ARROW: {
        body_set_rotation_speed(player, -DEFAULT_TANK_ROTATION_SPEED);
        break;
      }
      case LEFT_ARROW: {
        body_set_rotation_speed(player, DEFAULT_TANK_ROTATION_SPEED);
        break;
      }
      case SPACE: {
        if (body_get_time(player) > RELOAD_SPEED) {
          handle_bullet(state, player, player_color);
          bullet_shot_sound();
        }
      }
      }
    }
  } else if (*(size_t *)body_get_info(player) ==
             GRAVITY_TANK_TYPE) { // handles GRAVITY tank
    if (type == KEY_PRESSED) {
      switch (key) {
      case UP_ARROW: {
        body_set_magnitude(player, GRAVITY_TANK_VELOCITY);
        break;
      }
      case DOWN_ARROW: {
        body_set_magnitude(player, -GRAVITY_TANK_VELOCITY);
        break;
      }
      case RIGHT_ARROW: {
        body_set_rotation_speed(player, -GRAVITY_TANK_ROTATION_SPEED);
        break;
      }
      case LEFT_ARROW: {
        body_set_rotation_speed(player, GRAVITY_TANK_ROTATION_SPEED);
        break;
      }
      case SPACE: {
        if (body_get_time(player) > GRAVITY_TANK_RELOAD_SPEED) {
          handle_bullet(state, player, player_color);
          bullet_shot_sound();
        }
      }
      }
    }
  } else if (*(size_t *)body_get_info(player) ==
             SNIPER_TANK_TYPE) { // this is to handle the different tank types
    if (type == KEY_PRESSED) {
      switch (key) {
      case UP_ARROW: {
        body_set_magnitude(player, SNIPER_TANK_VELOCITY);
        break;
      }
      case DOWN_ARROW: {
        body_set_magnitude(player, -SNIPER_TANK_VELOCITY);
        break;
      }
      case RIGHT_ARROW: {
        body_set_rotation_speed(player, -SNIPER_TANK_ROTATION_SPEED);
        break;
      }
      case LEFT_ARROW: {
        body_set_rotation_speed(player, SNIPER_TANK_ROTATION_SPEED);
        break;
      }
      case SPACE: {
        if (body_get_time(player) > SNIPER_RELOAD_SPEED) {
          handle_bullet(state, player, player_color);
          bullet_shot_sound();
        }
      }
      }
    }
  } else if (*(size_t *)body_get_info(player) ==
             GATLING_TANK_TYPE) { // this is to handle the different tank types
    if (type == KEY_PRESSED) {
      switch (key) {
      case UP_ARROW: {
        body_set_magnitude(player, GATLING_TANK_VELOCITY);
        break;
      }
      case DOWN_ARROW: {
        body_set_magnitude(player, -GATLING_TANK_VELOCITY);
        break;
      }
      case RIGHT_ARROW: {
        body_set_rotation_speed(player, -GATLING_TANK_ROTATION_SPEED);
        break;
      }
      case LEFT_ARROW: {
        body_set_rotation_speed(player, GATLING_TANK_ROTATION_SPEED);
        break;
      }
      case SPACE: {
        if (body_get_time(player) > GATLING_RELOAD_SPEED) {
          handle_bullet(state, player, player_color);
          bullet_shot_sound();
        }
      }
      }
    }
  } else {
    // add other tanks after
  }
  if (type == KEY_RELEASED) {
    switch (key) {
    case UP_ARROW: {
      body_set_velocity(player, VEC_ZERO);
      body_set_magnitude(player, 0.0);
      break;
    }
    case DOWN_ARROW: {
      body_set_velocity(player, VEC_ZERO);
      body_set_magnitude(player, 0.0);
      break;
    }
    case RIGHT_ARROW: {
      body_set_rotation_speed(player, 0.0);
      break;
    }
    case LEFT_ARROW: {
      body_set_rotation_speed(player, 0.0);
      break;
    }
    }
  }
}

double double_abs(double x) {
  if (x < 0)
    return -x;
  return x;
}

void reset_mode(body_t *ai) {
  body_set_ai_mode(ai, 0);
  body_set_ai_time(ai, 0.0);
}

void ai_aim(body_t *player, body_t *ai) {
  // program ai to aim towards enemy, works for default tank
  if (body_get_distance(body_get_centroid(ai), body_get_centroid(player)) <
      750.0) {
    vector_t distance =
        vec_subtract(body_get_centroid(player), body_get_centroid(ai));
    double angle = atan(distance.y / distance.x);
    if (distance.x < 0) {
      angle += M_PI;
    }
    angle =
        angle -
        2 * M_PI * ((size_t)angle / ((size_t)(2 * M_PI))); // simulate % by 2pi
    double ai_angle = body_get_rotation(ai);
    ai_angle =
        ai_angle -
        2 * M_PI * ((size_t)angle / ((size_t)(2 * M_PI))); // simulate % by 2pi
    if (ai_angle < angle) {
      body_set_rotation_speed(ai, DEFAULT_TANK_ROTATION_SPEED);
    } else {
      body_set_rotation_speed(ai, -DEFAULT_TANK_ROTATION_SPEED);
    }
  } else {
    body_set_rotation_speed(ai, 0.0);
  }
}

void ai_shoot(state_t *state, body_t *player, body_t *ai) {
  if (body_get_distance(body_get_centroid(ai), body_get_centroid(player)) <
      750.0) {
    vector_t distance =
        vec_subtract(body_get_centroid(player), body_get_centroid(ai));
    double angle = atan(distance.y / distance.x);
    if (distance.x < 0) {
      angle += M_PI;
    }
    angle =
        angle -
        2 * M_PI * ((size_t)angle / ((size_t)(2 * M_PI))); // simulate % by 2pi
    double ai_angle = body_get_rotation(ai);
    ai_angle =
        ai_angle -
        2 * M_PI * ((size_t)angle / ((size_t)(2 * M_PI))); // simulate % by 2pi

    // program ai to shoot randomly, but only if pointed somewhat close to enemy
    // tank
    if (double_abs(angle - ai_angle) < M_PI / 8) {
      double time = body_get_time(ai);
      if (time > rand_num(RELOAD_SPEED, RELOAD_SPEED * 3)) {
        handle_bullet(state, ai, PLAYER2_COLOR);
      }
    }
  }
}

void move_ai(state_t *state, double dt) {
  body_t *player = scene_get_body(state->scene, 0);
  body_t *ai = scene_get_body(state->scene, 1);
  size_t ai_mode = body_get_ai_mode(ai);
  double ai_time = body_get_ai_time(ai);
  ai_shoot(state, player, ai);

  if (ai_mode == 0) {
    ai_aim(player, ai);

    body_set_velocity(ai, VEC_ZERO);
    body_set_magnitude(ai, 0.0);
    bool move = (ai_time > rand_num(2.5, 5.0));
    if (move) {
      size_t rand_mode = (size_t)rand_num(0.0, 10.0);
      body_set_ai_mode(ai, rand_mode);
      body_set_ai_time(ai, 0.0);
    }
  } else if (ai_mode == 1) {
    if (body_get_just_collided(ai)) {
      body_set_ai_mode(ai, 2);
      body_set_ai_time(ai, 1.5 - ai_time);
      body_set_just_collided(ai, false);
    } else {
      body_set_magnitude(ai, DEFAULT_TANK_VELOCITY);
      if (ai_time > 1.5) {
        reset_mode(ai);
      }
    }
  } else if (ai_mode == 2) {
    if (body_get_just_collided(ai)) {
      body_set_ai_mode(ai, 1);
      body_set_ai_time(ai, 1.5 - ai_time);
      body_set_just_collided(ai, false);
    } else {
      body_set_magnitude(ai, -DEFAULT_TANK_VELOCITY);
      if (ai_time > 1.5) {
        reset_mode(ai);
      }
    }
  } else if (ai_mode == 3) {
    if (body_get_just_collided(ai)) {
      body_set_ai_mode(ai, 6);
      body_set_ai_time(ai, 1.5 - ai_time);
      body_set_just_collided(ai, false);
    } else {
      body_set_magnitude(ai, DEFAULT_TANK_VELOCITY);
      body_set_rotation_speed(ai, DEFAULT_TANK_ROTATION_SPEED);
      if (ai_time > 1.5) {
        reset_mode(ai);
      }
    }
  } else if (ai_mode == 4) {
    if (body_get_just_collided(ai)) {
      body_set_ai_mode(ai, 5);
      body_set_ai_time(ai, 1.5 - ai_time);
      body_set_just_collided(ai, false);
    } else {
      body_set_magnitude(ai, DEFAULT_TANK_VELOCITY);
      body_set_rotation_speed(ai, -DEFAULT_TANK_ROTATION_SPEED);
      if (ai_time > 1.5) {
        reset_mode(ai);
      }
    }
  } else if (ai_mode == 5) {
    if (body_get_just_collided(ai)) {
      body_set_ai_mode(ai, 4);
      body_set_ai_time(ai, 1.5 - ai_time);
      body_set_just_collided(ai, false);
    } else {
      body_set_magnitude(ai, -DEFAULT_TANK_VELOCITY);
      body_set_rotation_speed(ai, DEFAULT_TANK_ROTATION_SPEED);
      if (ai_time > 1.5) {
        reset_mode(ai);
      }
    }
  } else if (ai_mode == 6) {
    if (body_get_just_collided(ai)) {
      body_set_ai_mode(ai, 3);
      body_set_ai_time(ai, 1.5 - ai_time);
      body_set_just_collided(ai, false);
    } else {
      body_set_magnitude(ai, -DEFAULT_TANK_VELOCITY);
      body_set_rotation_speed(ai, -DEFAULT_TANK_ROTATION_SPEED);
      if (ai_time > 1.5) {
        reset_mode(ai);
      }
    }
  } else if (ai_mode == 7) {
    body_set_rotation_speed(ai, DEFAULT_TANK_ROTATION_SPEED);
    if (ai_time > 1) {
      reset_mode(ai);
    }
  } else if (ai_mode == 8) {
    body_set_rotation_speed(ai, DEFAULT_TANK_ROTATION_SPEED);
    if (ai_time > 0.5) {
      reset_mode(ai);
    }
  } else if (ai_mode == 9) {
    body_set_rotation_speed(ai, -DEFAULT_TANK_ROTATION_SPEED);
    if (ai_time > 0.5) {
      reset_mode(ai);
    }
  }
}
void gameover_pop_up(state_t *state) {
  // background
  vector_t corner1 = {0.0, MAX_HEIGHT_GAME};
  list_t *background = make_rectangle(corner1, MAX_WIDTH_GAME, MAX_HEIGHT_GAME);
  sdl_draw_polygon(background, BLACK);
  char *player1_wins = "Player 1 wins";
  char *player2_wins = "Player 2 wins";
  char *winning_message;
  if (state->player1_score == 3) {
    winning_message = player1_wins;
  } else {
    winning_message = player2_wins;
  }

  vector_t winner_loc = {500.0, 650.0};
  SDL_Texture *winner =
      sdl_load_text(state, winning_message, state->text, SDL_RED, winner_loc);
  vector_t gameover_loc = {320.0, 950.0};
  SDL_Texture *gamemode =
      sdl_load_text(state, "GAMEOVER", state->title, SDL_RED, gameover_loc);
  SDL_DestroyTexture(winner);
  SDL_DestroyTexture(gamemode);
}

void check_end_game(state_t *state) {
  if (state->player1_score == 3 || state->player2_score == 3) {
    gameover_pop_up(state);
    exit(0);
  }
}

void show_scoreboard(state_t *state, int player1_score, int player2_score) {
  vector_t corner = {600.0, MAX_HEIGHT_GAME - 25.0};
  list_t *points = make_rectangle(corner, 400.0, 150.0);
  rgb_color_t black = {0.0, 0.0, 0.0};
  sdl_draw_polygon(points, black);

  SDL_Color white = {255, 255, 255, 255};
  // loc
  vector_t score_loc = {675.0, MAX_HEIGHT_GAME - 13.0};

  char player1_str[20];
  sprintf(player1_str, "%d", player1_score);
  char player2_str[20];
  sprintf(player2_str, "%d", player2_score);
  char *final_str = (char *)malloc(
      sizeof(char) * (strlen(player1_str) + strlen(player2_str) + 20));
  strcpy(final_str, player1_str);
  strcat(final_str, "   -   ");
  strcat(final_str, player2_str);

  TTF_Font *font1 = TTF_OpenFont("assets/font.ttf", FONT_SIZE);
  text_t *text = text_init(font1, (free_func_t)free);
  state->scoreboard = text;
  SDL_Texture *scoreboard =
      sdl_load_text(state, final_str, state->text, white, score_loc);

  sdl_show();
  SDL_DestroyTexture(scoreboard);
}

body_t *handle_selected_tank(size_t tank_type, vector_t start_pos,
                             rgb_color_t color) {
  // add rest of the tanks
  if (tank_type == DEFAULT_TANK_TYPE) {
    return init_default_tank(start_pos, DEFAULT_TANK_SIDE_LENGTH, VEC_ZERO,
                             DEFAULT_TANK_MASS, color, DEFAULT_TANK_MAX_HEALTH,
                             DEFAULT_TANK_TYPE);
  } else if (tank_type == GRAVITY_TANK_TYPE) {
    return init_gravity_tank(start_pos, GRAVITY_TANK_SIDE_LENGTH, VEC_ZERO,
                             GRAVITY_TANK_MASS, color, GRAVITY_TANK_MAX_HEALTH,
                             GRAVITY_TANK_TYPE);
  } else if (tank_type == SNIPER_TANK_TYPE) {
    return init_sniper_tank(start_pos, SNIPER_TANK_SIDE_LENGTH, VEC_ZERO,
                            SNIPER_TANK_MASS, color, SNIPER_TANK_MAX_HEALTH,
                            SNIPER_TANK_TYPE);
  } else if (tank_type == GATLING_TANK_TYPE) {
    return init_gatling_tank(start_pos, GATLING_TANK_SIDE_LENGTH, VEC_ZERO,
                             GATLING_TANK_MASS, color, GATLING_TANK_MAX_HEALTH,
                             GATLING_TANK_TYPE);
  } else {
    return init_default_tank(start_pos, DEFAULT_TANK_SIDE_LENGTH, VEC_ZERO,
                             DEFAULT_TANK_MASS, color, DEFAULT_TANK_MAX_HEALTH,
                             DEFAULT_TANK_TYPE);
  }
}

void make_players(state_t *state) {
  vector_t player1_start =
      (vector_t){MAX_WIDTH_GAME / 6, MAX_HEIGHT_GAME - 400.0};
  vector_t player2_start =
      (vector_t){MAX_WIDTH_GAME * 5 / 6, MAX_HEIGHT_GAME / 2 - 50.0};
  // can channge it to choose the type of tank later
  body_t *player1 = handle_selected_tank(state->player1_tank_type,
                                         player1_start, PLAYER1_COLOR);
  body_t *player2 = handle_selected_tank(state->player2_tank_type,
                                         player2_start, PLAYER2_COLOR);
  body_set_rotation(player2, M_PI);
  body_set_health(player1, DEFAULT_TANK_MAX_HEALTH);
  body_set_health(player2, DEFAULT_TANK_MAX_HEALTH);
  scene_add_body(state->scene, player1);
  scene_add_body(state->scene, player2);
  create_physics_collision(state->scene, TANKS_ELASTICITY,
                           scene_get_body(state->scene, 0),
                           scene_get_body(state->scene, 1));
}

void make_health_bars(state_t *state) {
  // initialize health bars
  list_t *p1_health_bar_shape = make_health_bar_p1(DEFAULT_TANK_MAX_HEALTH);
  size_t *type = malloc(sizeof(size_t));
  *type = HEALTH_BAR_TYPE;
  body_t *p1_health_bar = body_init_with_info(
      p1_health_bar_shape, 10.0, PLAYER1_COLOR, type, (free_func_t)free);
  scene_add_body(state->scene, p1_health_bar);

  list_t *p2_health_bar_shape = make_health_bar_p2(DEFAULT_TANK_MAX_HEALTH);
  size_t *type2 = malloc(sizeof(size_t));
  *type2 = HEALTH_BAR_TYPE;
  body_t *p2_health_bar = body_init_with_info(
      p2_health_bar_shape, 10.0, PLAYER2_COLOR, type2, (free_func_t)free);
  scene_add_body(state->scene, p2_health_bar);

  vector_t P1_HEART_CENTER = {50.0, MAX_HEIGHT_GAME - 40.0};
  vector_t P2_HEART_CENTER = {MAX_WIDTH_GAME - 50.0, MAX_HEIGHT_GAME - 40.0};

  list_t *p1_heart = make_heart(P1_HEART_CENTER, 50.0);
  size_t *type3 = malloc(sizeof(size_t));
  *type3 = HEALTH_BAR_TYPE;
  body_t *p1_heart_body = body_init_with_info(
      p1_heart, 10.0, PLAYER1_COLOR_SIMILAR, type3, (free_func_t)free);
  scene_add_body(state->scene, p1_heart_body);

  list_t *p2_heart = make_heart(P2_HEART_CENTER, 50.0);
  size_t *type4 = malloc(sizeof(size_t));
  *type4 = HEALTH_BAR_TYPE;
  body_t *p2_heart_body = body_init_with_info(
      p2_heart, 10.0, PLAYER2_COLOR_SIMILAR, type4, (free_func_t)free);
  scene_add_body(state->scene, p2_heart_body);
}

void reset_game(state_t *state) {
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    body_remove(body);
  }
  scene_tick(state->scene, 0.0);
  Mix_ChannelFinished(free_channel);

  make_players(state);
  make_health_bars(state);
  map_init(state->scene);
  for (size_t i = 2; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    if (*(size_t *)body_get_info(body) == RECTANGLE_OBSTACLE_TYPE ||
        *(size_t *)body_get_info(body) == TRIANGLE_OBSTACLE_TYPE) {
      create_physics_collision(state->scene, COLLISION_ELASTICITY,
                               scene_get_body(state->scene, 0),
                               scene_get_body(state->scene, i));
      create_physics_collision(state->scene, COLLISION_ELASTICITY,
                               scene_get_body(state->scene, 1),
                               scene_get_body(state->scene, i));
    }
  }
}

bool check_round_end(state_t *state) {
  body_t *player1 = scene_get_body(state->scene, 0);
  body_t *player2 = scene_get_body(state->scene, 1);
  if (body_get_health(player1) <= 0) {
    state->player2_score++;
    body_set_image_path(player1, "assets/destroyed_tank.png");
    return true;
  } else if (body_get_health(player2) <= 0) {
    state->player1_score++;
    body_set_image_path(player2, "assets/destroyed_tank.png");
    return true;
  }
  return false;
}

bool start_button_pressed(vector_t mouse) {
  if (mouse.x >= BUTTON_X_MIN && mouse.x <= BUTTON_X_MAX &&
      mouse.y >= START_BUTTON_Y_MIN && mouse.y <= START_BUTTON_Y_MAX) {
    return true;
  }
  return false;
}

bool options_button_pressed(vector_t mouse) {
  if (mouse.x >= BUTTON_X_MIN && mouse.x <= BUTTON_X_MAX &&
      mouse.y >= OPTIONS_BUTTON_Y_MIN && mouse.y <= OPTIONS_BUTTON_Y_MAX) {
    return true;
  }
  return false;
}

bool single_player_pressed(vector_t mouse) {
  if (mouse.x >= 275.0 && mouse.x <= 460.0 && mouse.y >= 60.0 &&
      mouse.y <= 140.0) {
    return true;
  }
  return false;
}
bool multiplayer_pressed(vector_t mouse) {
  if (mouse.x >= 540.0 && mouse.x <= 730.0 && mouse.y >= 60.0 &&
      mouse.y <= 140.0) {
    return true;
  }
  return false;
}
bool player1_default_pressed(vector_t mouse) {
  if (mouse.x >= 240.0 && mouse.x <= 320.0 && mouse.y >= 265.0 &&
      mouse.y <= 310.0) {
    return true;
  }
  return false;
}
bool player1_gravity_pressed(vector_t mouse) {
  if (mouse.x >= 365.0 && mouse.x <= 445.0 && mouse.y >= 265.0 &&
      mouse.y <= 310.0) {
    return true;
  }
  return false;
}
bool player1_sniper_pressed(vector_t mouse) {
  if (mouse.x >= 240.0 && mouse.x <= 320.0 && mouse.y >= 344.0 &&
      mouse.y <= 389.0) {
    return true;
  }
  return false;
}
bool player1_gatling_pressed(vector_t mouse) {
  if (mouse.x >= 365.0 && mouse.x <= 445.0 && mouse.y >= 344.0 &&
      mouse.y <= 389.0) {
    return true;
  }
  return false;
}
bool player2_default_pressed(vector_t mouse) {
  if (mouse.x >= 525.0 && mouse.x <= 605.0 && mouse.y >= 265.0 &&
      mouse.y <= 310.0) {
    return true;
  }
  return false;
}
bool player2_gravity_pressed(vector_t mouse) {
  if (mouse.x >= 655.0 && mouse.x <= 735.0 && mouse.y >= 265.0 &&
      mouse.y <= 310.0) {
    return true;
  }
  return false;
}
bool player2_sniper_pressed(vector_t mouse) {
  if (mouse.x >= 525.0 && mouse.x <= 605.0 && mouse.y >= 344.0 &&
      mouse.y <= 389.0) {
    return true;
  }
  return false;
}
bool player2_gatling_pressed(vector_t mouse) {
  if (mouse.x >= 655.0 && mouse.x <= 735.0 && mouse.y >= 344.0 &&
      mouse.y <= 389.0) {
    return true;
  }
  return false;
}
bool go_back_pressed(vector_t mouse) {
  if (mouse.x >= BUTTON_X_MIN && mouse.x <= BUTTON_X_MAX && mouse.y >= 410 &&
      mouse.y <= 490) {
    return true;
  }
  return false;
}

void menu_init(state_t *state) {
  state->is_menu = true;

  TTF_Font *font1 = TTF_OpenFont("assets/font.ttf", FONT_SIZE);
  text_t *text = text_init(font1, (free_func_t)free);
  state->text = text;

  TTF_Font *font2 = TTF_OpenFont("assets/font.ttf", TITLE_SIZE);
  text_t *title = text_init(font2, (free_func_t)free);
  state->title = title;

  TTF_Font *font3 = TTF_OpenFont("assets/font.ttf", TANK_SELECT_SIZE);
  text_t *select_tank = text_init(font3, (free_func_t)free);
  state->select_tank = select_tank;
}

void menu_pop_up(state_t *state) {
  vector_t corner1 = {0.0, MAX_HEIGHT_GAME};
  list_t *background = make_rectangle(corner1, MAX_WIDTH_GAME, MAX_HEIGHT_GAME);
  sdl_draw_polygon(background, LIGHT_GREY);

  // start button
  vector_t corner2 = {550.0, 750.0};
  list_t *start_button = make_rectangle(corner2, 500.0, 180.0);
  sdl_draw_polygon(start_button, GREEN);

  vector_t start_loc = {680.0, 750.0};
  SDL_Texture *start =
      sdl_load_text(state, "Start!", state->text, SDL_WHITE, start_loc);

  // options button
  vector_t corner3 = {550.0, 500.0};
  list_t *options_button = make_rectangle(corner3, 500.0, 180.0);
  sdl_draw_polygon(options_button, SLATE_GREY);

  // options text
  vector_t options_loc = {640.0, 500.0};
  SDL_Texture *options =
      sdl_load_text(state, "Options", state->text, SDL_BLACK, options_loc);

  // title
  vector_t title_loc = {540.0, 1120.0};
  SDL_Texture *title =
      sdl_load_text(state, "Tanks", state->title, SDL_FOREST_GREEN, title_loc);

  sdl_show();
  SDL_DestroyTexture(start);
  SDL_DestroyTexture(options);
  SDL_DestroyTexture(title);
}

void options_pop_up(state_t *state) {
  // background
  vector_t corner1 = {0.0, MAX_HEIGHT_GAME};
  list_t *background = make_rectangle(corner1, MAX_WIDTH_GAME, MAX_HEIGHT_GAME);
  sdl_draw_polygon(background, LIGHT_GREY);

  rgb_color_t singleplayer_color = FOREST_GREEN_POLY;
  rgb_color_t twoplayer_color = FOREST_GREEN_POLY;

  if (state->singleplayer) {
    singleplayer_color = DARKER_FOREST_GREEN_POLY;
  } else {
    twoplayer_color = DARKER_FOREST_GREEN_POLY;
  }

  // 1 PLAYER button
  vector_t corner2 = {200.0, 1140.0};
  list_t *oneplayer_button = make_rectangle(corner2, 500.0, 200.0);
  sdl_draw_polygon(oneplayer_button, singleplayer_color);
  vector_t one_player_loc = {250.0, 1130.0};
  SDL_Texture *oneplayer =
      sdl_load_text(state, "1 PLAYER", state->text, SDL_WHITE, one_player_loc);

  // 2 PLAYER button
  vector_t corner3 = {900.0, 1140.0};
  list_t *twoplayer_button = make_rectangle(corner3, 500.0, 200.0);
  sdl_draw_polygon(twoplayer_button, twoplayer_color);
  vector_t two_players_loc = {920.0, 1130.0};
  SDL_Texture *twoplayer = sdl_load_text(state, "2 PLAYERS", state->text,
                                         SDL_WHITE, two_players_loc);

  // Gamemodes
  vector_t gamemode_loc = {540.0, 1320.0};
  SDL_Texture *gamemode =
      sdl_load_text(state, "GAMEMODES", state->text, SDL_BLACK, gamemode_loc);

  // Selects Tanks
  vector_t select_title_loc = {540.0, 930.0};
  SDL_Texture *select_tank = sdl_load_text(state, "Select Tanks", state->text,
                                           SDL_BLACK, select_title_loc);

  // player 1 and 2 locs
  vector_t player1_loc = {250.0, 800.0};
  SDL_Texture *player1_select =
      sdl_load_text(state, "player 1", state->text, SHADE_GREEN, player1_loc);

  vector_t player2_loc = {980.0, 800.0};
  SDL_Texture *player2_select =
      sdl_load_text(state, "player 2", state->text, SDL_RED, player2_loc);

  rgb_color_t tank1_color = FOREST_GREEN;
  rgb_color_t tank2_color = YELLOW;
  rgb_color_t tank3_color = BLUE;
  rgb_color_t tank4_color = RED;
  rgb_color_t tank5_color = FOREST_GREEN;
  rgb_color_t tank6_color = YELLOW;
  rgb_color_t tank7_color = BLUE;
  rgb_color_t tank8_color = RED;

  if (state->player1_tank_type == DEFAULT_TANK_TYPE) {
    tank1_color = DARKER_FOREST_GREEN;
  } else if (state->player1_tank_type == GRAVITY_TANK_TYPE) {
    tank2_color = DARKER_YELLOW;
  } else if (state->player1_tank_type == SNIPER_TANK_TYPE) {
    tank3_color = DARKER_BLUE;
  } else {
    tank4_color = DARKER_RED;
  }

  if (state->player2_tank_type == DEFAULT_TANK_TYPE) {
    tank5_color = DARKER_FOREST_GREEN;
  } else if (state->player2_tank_type == GRAVITY_TANK_TYPE) {
    tank6_color = DARKER_YELLOW;
  } else if (state->player2_tank_type == SNIPER_TANK_TYPE) {
    tank7_color = DARKER_BLUE;
  } else {
    tank8_color = DARKER_RED;
  }

  // player 1 tanks
  vector_t tank1_corner = {120.0, 600.0};
  list_t *tank1_box = make_rectangle(tank1_corner, 200.0, 100.0);
  sdl_draw_polygon(tank1_box, tank1_color);
  vector_t tank1_loc = {135.0, 600.0};
  SDL_Texture *tank1 =
      sdl_load_text(state, "default", state->select_tank, SDL_WHITE, tank1_loc);

  vector_t tank2_corner = {460.0, 600.0};
  list_t *tank2_box = make_rectangle(tank2_corner, 200.0, 100.0);
  sdl_draw_polygon(tank2_box, tank2_color);
  vector_t tank2_loc = {475.0, 600.0};
  SDL_Texture *tank2 =
      sdl_load_text(state, "gravity", state->select_tank, SDL_WHITE, tank2_loc);

  vector_t tank3_corner = {120.0, 400.0};
  list_t *tank3_box = make_rectangle(tank3_corner, 200.0, 100.0);
  sdl_draw_polygon(tank3_box, tank3_color);
  vector_t tank3_loc = {145.0, 400.0};
  SDL_Texture *tank3 =
      sdl_load_text(state, "sniper", state->select_tank, SDL_WHITE, tank3_loc);

  vector_t tank4_corner = {460.0, 400.0};
  list_t *tank4_box = make_rectangle(tank4_corner, 200.0, 100.0);
  sdl_draw_polygon(tank4_box, tank4_color);
  vector_t tank4_loc = {475.0, 400.0};
  SDL_Texture *tank4 =
      sdl_load_text(state, "gatling", state->select_tank, SDL_WHITE, tank4_loc);

  // player 2 tanks
  double shiftx = 750.0;
  vector_t tank5_corner = {120.0 + shiftx, 600.0};
  list_t *tank5_box = make_rectangle(tank5_corner, 200.0, 100.0);
  sdl_draw_polygon(tank5_box, tank5_color);
  vector_t tank5_loc = {135.0 + shiftx, 600.0};
  SDL_Texture *tank5 =
      sdl_load_text(state, "default", state->select_tank, SDL_WHITE, tank5_loc);

  vector_t tank6_corner = {460.0 + shiftx, 600.0};
  list_t *tank6_box = make_rectangle(tank6_corner, 200.0, 100.0);
  sdl_draw_polygon(tank6_box, tank6_color);
  vector_t tank6_loc = {475.0 + shiftx, 600.0};
  SDL_Texture *tank6 =
      sdl_load_text(state, "gravity", state->select_tank, SDL_WHITE, tank6_loc);

  vector_t tank7_corner = {120.0 + shiftx, 400.0};
  list_t *tank7_box = make_rectangle(tank7_corner, 200.0, 100.0);
  sdl_draw_polygon(tank7_box, tank7_color);
  vector_t tank7_loc = {145.0 + shiftx, 400.0};
  SDL_Texture *tank7 =
      sdl_load_text(state, "sniper", state->select_tank, SDL_WHITE, tank7_loc);

  vector_t tank8_corner = {460.0 + shiftx, 400.0};
  list_t *tank8_box = make_rectangle(tank8_corner, 200.0, 100.0);
  sdl_draw_polygon(tank8_box, tank8_color);
  vector_t tank8_loc = {475.0 + shiftx, 400.0};
  SDL_Texture *tank8 =
      sdl_load_text(state, "gatling", state->select_tank, SDL_WHITE, tank8_loc);

  // go back button
  vector_t go_back_corner = {550.0, 220.0};
  list_t *go_back_button = make_rectangle(go_back_corner, 500.0, 180.0);
  sdl_draw_polygon(go_back_button, BLACK);
  vector_t go_back_loc = {680.0, 220.0};
  SDL_Texture *go_back =
      sdl_load_text(state, "Back", state->text, SDL_WHITE, go_back_loc);

  sdl_show();
  SDL_DestroyTexture(oneplayer);
  SDL_DestroyTexture(twoplayer);
  SDL_DestroyTexture(gamemode);
  SDL_DestroyTexture(select_tank);
  SDL_DestroyTexture(player1_select);
  SDL_DestroyTexture(player2_select);
  SDL_DestroyTexture(tank1);
  SDL_DestroyTexture(tank2);
  SDL_DestroyTexture(tank3);
  SDL_DestroyTexture(tank4);
  SDL_DestroyTexture(tank5);
  SDL_DestroyTexture(tank6);
  SDL_DestroyTexture(tank7);
  SDL_DestroyTexture(tank8);
  SDL_DestroyTexture(go_back);
}

void game_starter(state_t *state) {
  make_players(state);
  make_health_bars(state);
  map_init(state->scene);
  show_scoreboard(state, 0, 0);
  // creates collisions
  for (size_t i = 2; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    if (*(size_t *)body_get_info(body) == RECTANGLE_OBSTACLE_TYPE ||
        *(size_t *)body_get_info(body) == TRIANGLE_OBSTACLE_TYPE) {
      create_physics_collision(state->scene, COLLISION_ELASTICITY,
                               scene_get_body(state->scene, 0),
                               scene_get_body(state->scene, i));
      create_physics_collision(state->scene, COLLISION_ELASTICITY,
                               scene_get_body(state->scene, 1),
                               scene_get_body(state->scene, i));
    }
  }
}

void handler(char key, key_event_type_t type, double held_time, state_t *state,
             vector_t loc) {
  if (state->is_menu) {
    switch (key) {
    case MOUSE_CLICK: {
      if (start_button_pressed(loc)) {
        state->is_menu = false;
        game_starter(state);
        // make players here in order to be able to change the type in the menu
        break;
      } else if (options_button_pressed(loc)) {
        state->is_menu = false;
        state->is_options = true;
        break;
      }
    }
    }
  } else if (state->is_options) {
    switch (key) { // for the options page
    case MOUSE_CLICK: {
      if (single_player_pressed(loc)) {
        state->singleplayer = true;
        // state->player2_tank_type = DEFAULT_TANK_TYPE;
        break;
      } else if (multiplayer_pressed(loc)) {
        state->singleplayer = false;
        break;
      } else if (player1_default_pressed(loc)) {
        // change later
        state->player1_tank_type = DEFAULT_TANK_TYPE;
        break;
      } else if (player1_gravity_pressed(loc)) {
        state->player1_tank_type = GRAVITY_TANK_TYPE;
        break;
      } else if (player1_sniper_pressed(loc)) {
        state->player1_tank_type = SNIPER_TANK_TYPE;
        break;
      } else if (player1_gatling_pressed(loc)) {
        state->player1_tank_type = GATLING_TANK_TYPE;
        break;
      } else if (player2_default_pressed(loc)) {
        state->player2_tank_type = DEFAULT_TANK_TYPE;
        break;
      } else if (player2_gravity_pressed(loc)) {
        state->player2_tank_type = GRAVITY_TANK_TYPE;
        break;
      } else if (player2_sniper_pressed(loc)) {
        state->player2_tank_type = SNIPER_TANK_TYPE;
        break;
      } else if (player2_gatling_pressed(loc)) {
        state->player2_tank_type = GATLING_TANK_TYPE;
        break;
      } else if (go_back_pressed(loc)) {
        state->is_options = false;
        state->is_menu = true;
        break;
      }
    }
    }

  } else {
    body_t *player1 = scene_get_body(state->scene, (size_t)0);
    body_t *player2 = scene_get_body(state->scene, (size_t)1);
    tank_handler(key, type, held_time, state, player1, PLAYER1_COLOR);
    if (!state->singleplayer) {
      tank_handler2(key, type, held_time, state, player2, PLAYER2_COLOR);
    }
  }
}

state_t *emscripten_init() {
  init_sounds();
  vector_t min = VEC_ZERO;
  vector_t max = {MAX_WIDTH_GAME, MAX_HEIGHT_GAME};
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->time = 0.0;
  state->scene = scene_init();
  state->player1_score = 0;
  state->player2_score = 0;
  state->player1_tank_type = DEFAULT_TANK_TYPE; //
  state->player2_tank_type = DEFAULT_TANK_TYPE; //
  state->singleplayer = false; // could comment this out for it to work
  state->is_options = false;
  state->is_round_end = false;

  menu_init(state);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  if (state->is_menu) {
    menu_pop_up(state);
    sdl_on_key((key_handler_t)handler);
  } else if (state->is_options) {
    options_pop_up(state);
    sdl_on_key((key_handler_t)handler);
  } else {
    double dt = time_since_last_tick();
    sdl_on_key((key_handler_t)handler);
    state->time += dt;
    if (state->is_round_end) {
      death_sound();
      while (dt < DEATH_PAUSE_TIME) {
        dt += time_since_last_tick();
      }
      reset_game(state);
    }
    state->is_round_end = check_round_end(state);

    // add time to player bodies for reload
    body_t *player1 = scene_get_body(state->scene, 0);
    body_t *player2 = scene_get_body(state->scene, 1);
    body_set_time(player1, body_get_time(player1) + dt);
    body_set_time(player2, body_get_time(player2) + dt);

    if (state->singleplayer) {
      move_ai(state, dt);
      body_set_ai_time(player2, body_get_ai_time(player2) + dt);
    }

    // add time to bullet bodies to see if they should disappear
    for (size_t i = 2; i < scene_bodies(state->scene); i++) {
      body_t *body = scene_get_body(state->scene, i);
      if (*(size_t *)body_get_info(body) == BULLET_TYPE ||
          *(size_t *)body_get_info(body) == SNIPER_BULLET_TYPE ||
          *(size_t *)body_get_info(body) == GATLING_BULLET_TYPE ||
          *(size_t *)body_get_info(body) == GRAVITY_BULLET_TYPE) {
        body_set_time(body, body_get_time(body) + dt);
        if (body_get_time(body) > BULLET_DISAPPEAR_TIME) {
          body_remove(body);
        }
      }
    }

    // //update health bar
    body_t *health_bar_p1 = scene_get_body(state->scene, 2);
    body_set_shape(health_bar_p1, make_health_bar_p1(body_get_health(player1)));

    body_t *health_bar_p2 = scene_get_body(state->scene, 3);
    body_set_shape(health_bar_p2, make_health_bar_p2(body_get_health(player2)));

    scene_tick(state->scene, dt);
    sdl_render_scene(state->scene);
    show_scoreboard(state, state->player1_score, state->player2_score);
    check_end_game(state);
  }
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
