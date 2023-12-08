#include "star.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

typedef struct star {
  list_t *polygon;
  vector_t *velocity;
  double rotation;
  bool just_moved;
  double r;
  double g;
  double b;
} star_t;

list_t *make_star(vector_t center, double length, int star_points) {

  list_t *poly = list_init(star_points * 2, &free);

  for (size_t i = 0; i < star_points; i++) {
    vector_t *top = malloc(sizeof(vector_t));
    assert(top != NULL);
    top->x = center.x;
    top->y = center.y + length;
    // using law of sines
    double height_2 = (length * sin(M_PI / ((double)star_points * 2))) /
                      sin(M_PI - M_PI * 3 / 2 / star_points);
    vector_t *middle = malloc(sizeof(vector_t));
    assert(middle != NULL);
    middle->x =
        center.x - (height_2 * cos(M_PI / 2 - M_PI / (double)star_points));
    middle->y =
        center.y + (height_2 * sin(M_PI / 2 - M_PI / (double)star_points));
    // add to list
    list_add(poly, top);
    list_add(poly, middle);
    // rotate
    polygon_rotate(poly, -2 * M_PI / ((double)star_points), center);
  }

  return poly;
}

double rand_num(double min, double max) {
  double range = (max - min);
  double div = RAND_MAX / range;
  return min + ((double)rand() / div);
}

star_t *star_init(int vertices, double height, double length, double speed,
                  double angle, double rotation_speed) {
  star_t *star = malloc(sizeof(star_t));
  assert(star != NULL);
  vector_t start = {0.0, height};
  star->polygon = make_star(start, length, vertices);
  vector_t *velocity = malloc(sizeof(vector_t));
  assert(velocity != NULL);
  velocity->x = speed * cos(angle);
  velocity->y = -1 * speed * sin(angle);
  star->velocity = velocity;
  star->rotation = rotation_speed;
  star->just_moved = false;
  star->r = rand_num(0.0, 1.0);
  star->g = rand_num(0.0, 1.0);
  star->b = rand_num(0.0, 1.0);
  return star;
}

void star_translate(star_t *star, vector_t vec) {
  polygon_translate(star->polygon, vec);
}
void star_rotate(star_t *star, double angle, vector_t point) {
  polygon_rotate(star->polygon, angle, point);
}

vector_t *get_star_velocity(star_t *star) { return star->velocity; }

list_t *get_star_polygon(star_t *star) { return star->polygon; }

double get_star_rotation(star_t *star) { return star->rotation; }

double get_star_red_val(star_t *star) { return star->r; }

double get_star_green_val(star_t *star) { return star->g; }

double get_star_blue_val(star_t *star) { return star->b; }

bool get_star_just_moved(star_t *star) { return star->just_moved; }

void set_star_just_moved(star_t *star, bool val) { star->just_moved = val; }

void star_free(star_t *star) {
  assert(star != NULL);

  list_free(star->polygon);
  free(star);
}
