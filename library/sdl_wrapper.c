#include "sdl_wrapper.h"
#include "body.h"
#include "state.h"
#include "text.h"
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

SDL_Texture *img = NULL;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

vector_t get_scene_position(vector_t window_pos, vector_t window_center) {
  vector_t pixel_center_offset = {
      +window_pos.x - window_center.x,
      -window_pos.y + window_center.y,
  };
  double scale = get_scene_scale(window_center);
  vector_t scene_center_offset = vec_multiply(1.0 / scale, pixel_center_offset);
  return scene_center_offset;
}

// from pixel to scene coordinate
vector_t sdl_mouse_position() {
  int x, y;
  SDL_GetMouseState(&x, &y);
  vector_t window_center = get_window_center();
  return get_scene_position((vector_t){x, y}, window_center);
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return SPACE;

  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  TTF_Init();
}

bool sdl_is_done(state_t *state) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type, held_time, state, VEC_ZERO);
      break;
    case SDL_MOUSEBUTTONUP:
      if (key_handler == NULL) {
        break;
      }
      key_event_type_t type_mouse = event->type =
          SDL_MOUSEBUTTONDOWN ? KEY_PRESSED : KEY_RELEASED;
      type_mouse = event->type == SDL_MOUSEMOTION ? MOUSE_ENGAGED : type_mouse;

      vector_t loc = (vector_t){event->motion.x, event->motion.y};
      key = MOUSE_CLICK;

      held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type_mouse, held_time, state, loc);
      break;
    case SDL_MOUSEMOTION:
      if (key_handler == NULL) {
        break;
      }
      type_mouse = event->type =
          SDL_MOUSEBUTTONDOWN ? KEY_PRESSED : KEY_RELEASED;
      type_mouse = event->type == SDL_MOUSEMOTION ? MOUSE_ENGAGED : type_mouse;

      loc = (vector_t){event->motion.x, event->motion.y};
      key = MOUSE_MOVED;

      held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type_mouse, held_time, state, loc);
      break;
    case SDL_MOUSEBUTTONDOWN:
      break;
    }
  }
  free(event);
  return false;
}

SDL_Texture *sdl_load_text(state_t *state, char *words, text_t *text,
                           SDL_Color color, vector_t loc) {
  TTF_Font *font = text_get_font(text);

  SDL_Surface *surfaceMessage = TTF_RenderText_Solid(font, words, color);
  SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

  // scale from vector_t to pixel
  vector_t window_center = get_window_center();
  vector_t coords = get_window_position(loc, window_center);

  SDL_Rect Message_rect;              // create a rect
  Message_rect.x = coords.x;          // controls the rect's x coordinate
  Message_rect.y = coords.y;          // controls the rect's y coordinte
  Message_rect.w = surfaceMessage->w; // controls the width of the rect
  Message_rect.h = surfaceMessage->h; // controls the height of the rect

  SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

  SDL_FreeSurface(surfaceMessage);

  return Message;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);
  assert(0 <= color.r && color.r <= 1);
  assert(0 <= color.g && color.g <= 1);
  assert(0 <= color.b && color.b <= 1);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  int *w = malloc(sizeof(int));
  int *h = malloc(sizeof(int));
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    if (body_get_image_path(body) == NULL) {
      sdl_draw_polygon(shape, body_get_color(body));
    } else {
      img = IMG_LoadTexture(renderer, body_get_image_path(body));
      double angle = body_get_rotation(body) * -(180 / M_PI); // set the angle.
      SDL_RendererFlip flip = SDL_FLIP_NONE; // the flip of the texture.
      SDL_QueryTexture(img, NULL, NULL, w, h);
      SDL_Rect texr;
      vector_t window_center = get_window_center();
      vector_t coord = {body_get_centroid(body).x - 40,
                        body_get_centroid(body).y + 50};
      SDL_Point center = {16, 20};
      vector_t pixel = get_window_position(coord, window_center);
      texr.x = pixel.x;
      texr.y = pixel.y;
      texr.w = 40;
      texr.h = 40;
      SDL_RenderCopyEx(renderer, img, NULL, &texr, angle, &center, flip);
    }
    list_free(shape);
  }
  sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}
