#include "polygon.h"
#include "list.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double polygon_area(list_t *polygon) {
  // shoelace method
  double sum = 0.0;
  for (size_t i = 0; i < list_size(polygon) - 1; i++) {
    vector_t *cur = list_get(polygon, i);
    vector_t *nxt = list_get(polygon, i + 1);

    double first = cur->x * nxt->y;
    double second = cur->y * nxt->x;
    sum += first - second;
  }
  vector_t *cur = list_get(polygon, list_size(polygon) - 1);
  vector_t *nxt = list_get(polygon, 0);

  double first = cur->x * nxt->y;
  double second = cur->y * nxt->x;
  sum += first - second;
  return fabs(sum / 2);
}

vector_t polygon_centroid(list_t *polygon) {
  double center_x = 0.0;
  for (size_t i = 0; i < list_size(polygon) - 1; i++) {
    vector_t *cur = list_get(polygon, i);
    vector_t *nxt = list_get(polygon, i + 1);
    center_x += (cur->x + nxt->x) * vec_cross(*cur, *nxt);
  }
  vector_t *cur = list_get(polygon, list_size(polygon) - 1);
  vector_t *nxt = list_get(polygon, 0);
  center_x += (cur->x + nxt->x) * vec_cross(*cur, *nxt);
  center_x = center_x / (6 * polygon_area(polygon));

  double center_y = 0.0;
  for (size_t i = 0; i < list_size(polygon) - 1; i++) {
    vector_t *cur = list_get(polygon, i);
    vector_t *nxt = list_get(polygon, i + 1);
    center_y += (cur->y + nxt->y) * vec_cross(*cur, *nxt);
  }
  vector_t *cur2 = list_get(polygon, list_size(polygon) - 1);
  vector_t *nxt2 = list_get(polygon, 0);
  center_y += (cur2->y + nxt2->y) * vec_cross(*cur, *nxt);
  center_y = center_y / (6 * polygon_area(polygon));

  vector_t center = {center_x, center_y};
  return center;
}

void polygon_translate(list_t *polygon, vector_t translation) {
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t *vector = list_get(polygon, i);
    vector_t translated = vec_add(*vector, translation);
    vector->x = translated.x;
    vector->y = translated.y;
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t *vector = list_get(polygon, i);
    vector_t subtracted = vec_subtract(*vector, point);
    vector_t rotated = vec_rotate(subtracted, angle);
    vector_t final = vec_add(rotated, point);
    vector->x = final.x;
    vector->y = final.y;
  }
}
