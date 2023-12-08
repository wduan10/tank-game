#ifndef __STAR_H__
#define __STAR_H__

#include "polygon.h"
#include "sdl_wrapper.h"

typedef struct star star_t;

list_t *make_star(vector_t center, double length, int star_points);

double rand_num(double min, double max);

star_t *star_init(int vertices1, double height, double length, double speed,
                  double angle, double rotation_speed);

void star_rotate(star_t *star, double angle, vector_t point);

void star_translate(star_t *star, vector_t vec);

vector_t *get_star_velocity(star_t *star);

list_t *get_star_polygon(star_t *star);

double get_star_rotation(star_t *star);

double get_star_red_val(star_t *star);

double get_star_green_val(star_t *star);

double get_star_blue_val(star_t *star);

bool get_star_just_moved(star_t *star);

void set_star_just_moved(star_t *star, bool val);

void star_free(star_t *star);

#endif // #ifndef __STAR_H__