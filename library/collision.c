#include "collision.h"
#include "forces.h"
#include "polygon.h"
#include "vector.h"
#include <assert.h>
#include <list.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

double const LARGE_NUM = INFINITY;
double const SMALL_NUM = -INFINITY;

void find_perp_axis(list_t *shape, list_t *axes) {
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t *p1 = list_get(shape, i);
    vector_t *p2 = list_get(shape, (i + 1) % list_size(shape));
    vector_t edge = vec_subtract(*p1, *p2);
    vector_t *axis = malloc(sizeof(vector_t));
    double magnitude = sqrt(edge.x * edge.x + edge.y * edge.y);
    assert(axis != NULL);
    axis->x = -edge.y / magnitude;
    axis->y = edge.x / magnitude;
    list_add(axes, axis);
  }
}

bool test_intersecting_projections(vector_t p1, vector_t p2) {
  if ((p1.x <= p2.x && p1.y >= p2.x) || (p2.x >= p1.x && p2.y <= p1.y) ||
      (p1.x <= p2.y && p1.y >= p2.y) || (p2.x <= p1.x && p2.y >= p1.x)) {
    return true;
  }
  return false;
}

vector_t get_projection(list_t *shape, vector_t *axis) {
  double min = LARGE_NUM;
  double max = SMALL_NUM;
  for (size_t i = 0; i < list_size(shape); i++) {
    vector_t *point = (vector_t *)list_get(shape, i);
    double proj = vec_dot(*axis, *point);
    if (proj > max) {
      max = proj;
    }
    if (proj < min) {
      min = proj;
    }
  }
  return (vector_t){min, max};
}

double calculate_overlap(vector_t p1, vector_t p2) {
  if (p1.x < p2.x) {
    return fabs(p2.x - p1.y);
  } else {
    return fabs(p1.x - p2.y);
  }
}

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  collision_info_t collision;
  size_t num_points = list_size(shape1) + list_size(shape2);
  list_t *axes = list_init(num_points, (free_func_t)free);
  find_perp_axis(shape1, axes);
  find_perp_axis(shape2, axes);

  double least_overlap = INFINITY;
  for (size_t i = 0; i < num_points; i++) {
    vector_t *curr_axis = list_get(axes, i);
    if (!test_intersecting_projections(get_projection(shape1, curr_axis),
                                       get_projection(shape2, curr_axis))) {
      list_free(shape1);
      list_free(shape2);
      list_free(axes);
      collision.collided = false;
      return collision;
    } else {
      double overlap = calculate_overlap(get_projection(shape1, curr_axis),
                                         get_projection(shape2, curr_axis));
      if (overlap < least_overlap) {
        least_overlap = overlap;
        collision.axis = *curr_axis;
      }
    }
  }
  collision.collided = true;
  list_free(shape1);
  list_free(shape2);
  list_free(axes);
  return collision;
}