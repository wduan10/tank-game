#include "map.h"
#include "body.h"
#include "color.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

// scene info
double TOP_WALL = 1200.0;
double BOTTOM_WALL = 100.0;
double LEFT_WALL = 0.0;
double RIGHT_WALL = 1520.0;

// obstacle stats
double OBSTACLE_MASS = INFINITY;
const size_t RECTANGLE_OBSTACLE_TYPE = 20;
const size_t TRIANGLE_OBSTACLE_TYPE = 21;
const double TRIANGLE_DAMAGE = 5.0;
rgb_color_t OBSTACLE_COLOR_1 = {0.76, 0.76, 0.76};
rgb_color_t OBSTACLE_COLOR_2 = {0.35, 0.35, 0.35};
rgb_color_t OBSTACLE_COLOR_3 = {0.57, 0.59, 0.60};

list_t *make_rectangle(vector_t corner, double width, double height) {
  list_t *rectangle = list_init(4, (free_func_t)free);
  vector_t *point1 = malloc(sizeof(vector_t));
  assert(point1 != NULL);
  point1->x = corner.x;
  point1->y = corner.y;
  list_add(rectangle, point1);
  vector_t *point2 = malloc(sizeof(vector_t));
  assert(point2 != NULL);
  point2->x = corner.x;
  point2->y = corner.y - height;
  list_add(rectangle, point2);
  vector_t *point3 = malloc(sizeof(vector_t));
  assert(point3 != NULL);
  point3->x = corner.x + width;
  point3->y = corner.y - height;
  list_add(rectangle, point3);
  vector_t *point4 = malloc(sizeof(vector_t));
  assert(point4 != NULL);
  point4->x = corner.x + width;
  point4->y = corner.y;
  list_add(rectangle, point4);
  return rectangle;
}

list_t *make_vert_triangle(vector_t bisector_point, double perp_bisector) {
  list_t *triangle = list_init(3, (free_func_t)free);
  vector_t *point1 = malloc(sizeof(vector_t));
  assert(point1 != NULL);
  point1->x = bisector_point.x + (perp_bisector / sqrt(3));
  point1->y = bisector_point.y;
  list_add(triangle, point1);
  vector_t *point2 = malloc(sizeof(vector_t));
  assert(point2 != NULL);
  point2->x = bisector_point.x;
  point2->y = bisector_point.y + perp_bisector;
  list_add(triangle, point2);
  vector_t *point3 = malloc(sizeof(vector_t));
  assert(point3 != NULL);
  point3->x = bisector_point.x - (perp_bisector / sqrt(3));
  point3->y = bisector_point.y;
  list_add(triangle, point3);
  return triangle;
}

list_t *make_horz_triangle(vector_t bisector_point, double perp_bisector) {
  list_t *triangle = list_init(3, (free_func_t)free);
  vector_t *point1 = malloc(sizeof(vector_t));
  assert(point1 != NULL);
  point1->x = bisector_point.x;
  point1->y = bisector_point.y - (perp_bisector / sqrt(3));
  list_add(triangle, point1);
  vector_t *point2 = malloc(sizeof(vector_t));
  assert(point2 != NULL);
  point2->x = bisector_point.x + perp_bisector;
  point2->y = bisector_point.y;
  list_add(triangle, point2);
  vector_t *point3 = malloc(sizeof(vector_t));
  assert(point3 != NULL);
  point3->x = bisector_point.x;
  point3->y = bisector_point.y + (perp_bisector / sqrt(3));
  list_add(triangle, point3);
  return triangle;
}

void spawn_rectangle(scene_t *scene, vector_t corner, double width,
                     double height, rgb_color_t color) {
  list_t *points = make_rectangle(corner, width, height);
  int *type = malloc(sizeof(int));
  *type = RECTANGLE_OBSTACLE_TYPE;
  body_t *rectangle = body_init_with_info(points, OBSTACLE_MASS, color, type,
                                          (free_func_t)free);
  scene_add_body(scene, rectangle);
}

void spawn_vert_triangle(scene_t *scene, vector_t bisector_point,
                         double perp_bisector, rgb_color_t color) {
  list_t *points = make_vert_triangle(bisector_point, perp_bisector);
  int *type = malloc(sizeof(int));
  *type = TRIANGLE_OBSTACLE_TYPE;
  body_t *triangle = body_init_with_info(points, OBSTACLE_MASS, color, type,
                                         (free_func_t)free);
  scene_add_body(scene, triangle);
}

void spawn_horz_triangle(scene_t *scene, vector_t bisector_point,
                         double perp_bisector, rgb_color_t color) {
  list_t *points = make_horz_triangle(bisector_point, perp_bisector);
  int *type = malloc(sizeof(int));
  *type = TRIANGLE_OBSTACLE_TYPE;
  body_t *triangle = body_init_with_info(points, OBSTACLE_MASS, color, type,
                                         (free_func_t)free);
  scene_add_body(scene, triangle);
}

void map_init(scene_t *scene) {
  // top row obstacles
  vector_t corner1 = {LEFT_WALL, TOP_WALL};
  spawn_rectangle(scene, corner1, 80.0, 200.0, OBSTACLE_COLOR_1);

  vector_t corner2 = {LEFT_WALL + 80.0, TOP_WALL};
  spawn_rectangle(scene, corner2, 80.0, 80.0, OBSTACLE_COLOR_2);

  vector_t corner3 = {LEFT_WALL + 160.0, TOP_WALL};
  spawn_rectangle(scene, corner3, 80.0, 80.0, OBSTACLE_COLOR_3);

  vector_t corner4 = {LEFT_WALL + 240.0, TOP_WALL};
  spawn_rectangle(scene, corner4, 200.0, 140.0, OBSTACLE_COLOR_1);

  vector_t corner5 = {LEFT_WALL + 440.0, TOP_WALL};
  // make this one a spawn_pentagon later
  spawn_rectangle(scene, corner5, 210.0, 140.0, OBSTACLE_COLOR_2);

  vector_t corner6 = {LEFT_WALL + 650.0, TOP_WALL};
  spawn_rectangle(scene, corner6, 200.0, 100.0, OBSTACLE_COLOR_3);

  vector_t corner7 = {LEFT_WALL + 850.0, TOP_WALL};
  spawn_rectangle(scene, corner7, 280.0, 100.0, OBSTACLE_COLOR_3);

  vector_t corner8 = {LEFT_WALL + 1130, TOP_WALL};
  spawn_rectangle(scene, corner8, 70.0, 100.0, OBSTACLE_COLOR_2);

  vector_t bisector_point1 = {LEFT_WALL + 1165.0, TOP_WALL - 100.0};
  spawn_vert_triangle(scene, bisector_point1, -60.0, OBSTACLE_COLOR_2);

  vector_t corner9 = {LEFT_WALL + 1200, TOP_WALL};
  spawn_rectangle(scene, corner9, 70.0, 100.0, OBSTACLE_COLOR_3);

  vector_t bisector_point2 = {LEFT_WALL + 1235.0, TOP_WALL - 100.0};
  spawn_vert_triangle(scene, bisector_point2, -60.0, OBSTACLE_COLOR_3);

  vector_t corner10 = {LEFT_WALL + 1270, TOP_WALL};
  spawn_rectangle(scene, corner10, 70.0, 100.0, OBSTACLE_COLOR_2);

  vector_t bisector_point3 = {LEFT_WALL + 1305.0, TOP_WALL - 100.0};
  spawn_vert_triangle(scene, bisector_point3, -60.0, OBSTACLE_COLOR_2);

  vector_t corner11 = {LEFT_WALL + 1340, TOP_WALL};
  spawn_rectangle(scene, corner11, 255.0, 100.0, OBSTACLE_COLOR_1);

  // make left wall
  vector_t corner12 = {LEFT_WALL, TOP_WALL - 200.0};
  spawn_rectangle(scene, corner12, 80.0, 300.0, OBSTACLE_COLOR_2);

  vector_t corner13 = {LEFT_WALL, TOP_WALL - 500.0};
  spawn_rectangle(scene, corner13, 120.0, 250.0, OBSTACLE_COLOR_3);

  vector_t corner14 = {LEFT_WALL, TOP_WALL - 750.0};
  spawn_rectangle(scene, corner14, 120.0, 200.0, OBSTACLE_COLOR_1);

  vector_t corner15 = {LEFT_WALL, TOP_WALL - 950.0};
  spawn_rectangle(scene, corner15, 120.0, 150.0, OBSTACLE_COLOR_2);

  // make bottom wall
  vector_t corner16 = {LEFT_WALL, BOTTOM_WALL};
  spawn_rectangle(scene, corner16, 400.0, 100.0, OBSTACLE_COLOR_3);

  vector_t corner17 = {LEFT_WALL + 400.0, BOTTOM_WALL};
  spawn_rectangle(scene, corner17, 160.0, 100.0, OBSTACLE_COLOR_2);

  vector_t corner18 = {LEFT_WALL + 560.0, BOTTOM_WALL + 400.0};
  spawn_rectangle(scene, corner18, 60.0, 500.0, OBSTACLE_COLOR_3);

  vector_t bisector_point4 = {LEFT_WALL + 706, 0.0};
  spawn_vert_triangle(scene, bisector_point4, 150.0, OBSTACLE_COLOR_2);

  vector_t bisector_point5 = {LEFT_WALL + 878, 0.0};
  spawn_vert_triangle(scene, bisector_point5, 150.0, OBSTACLE_COLOR_2);

  vector_t bisector_point6 = {LEFT_WALL + 1050, 0.0};
  spawn_vert_triangle(scene, bisector_point6, 150.0, OBSTACLE_COLOR_2);

  vector_t corner19 = {LEFT_WALL + 1136, BOTTOM_WALL};
  spawn_rectangle(scene, corner19, 464.0, 100.0, OBSTACLE_COLOR_3);

  // make right wall
  vector_t corner20 = {RIGHT_WALL, TOP_WALL - 100.0};
  spawn_rectangle(scene, corner20, 80.0, 200.0, OBSTACLE_COLOR_1);

  vector_t corner21 = {RIGHT_WALL, TOP_WALL - 300.0};
  spawn_rectangle(scene, corner21, 80.0, 300.0, OBSTACLE_COLOR_1);

  vector_t corner22 = {RIGHT_WALL, TOP_WALL - 600.0};
  spawn_rectangle(scene, corner22, 80.0, 80.0, OBSTACLE_COLOR_2);

  vector_t bisector_point7 = {1520, TOP_WALL - 640.0};
  spawn_horz_triangle(scene, bisector_point7, -70.0, OBSTACLE_COLOR_2);

  vector_t corner23 = {RIGHT_WALL, TOP_WALL - 680.0};
  spawn_rectangle(scene, corner23, 80.0, 160.0, OBSTACLE_COLOR_1);

  vector_t corner24 = {RIGHT_WALL, TOP_WALL - 840.0};
  spawn_rectangle(scene, corner24, 80.0, 80.0, OBSTACLE_COLOR_2);

  vector_t bisector_point8 = {1520, TOP_WALL - 880.0};
  spawn_horz_triangle(scene, bisector_point8, -70.0, OBSTACLE_COLOR_2);

  vector_t corner25 = {RIGHT_WALL, TOP_WALL - 920.0};
  spawn_rectangle(scene, corner25, 80.0, 180.0, OBSTACLE_COLOR_1);

  // make inside obstacles
  vector_t corner26 = {LEFT_WALL + 1136, BOTTOM_WALL + 80.0};
  spawn_rectangle(scene, corner26, 80.0, 80.0, OBSTACLE_COLOR_2);

  vector_t bisector_point9 = {LEFT_WALL + 1216, BOTTOM_WALL + 40.0};
  spawn_horz_triangle(scene, bisector_point9, 70.0, OBSTACLE_COLOR_2);

  vector_t corner27 = {LEFT_WALL + 1136, BOTTOM_WALL + 260.0};
  spawn_rectangle(scene, corner27, 80.0, 180.0, OBSTACLE_COLOR_1);

  vector_t corner28 = {LEFT_WALL + 1136, BOTTOM_WALL + 340.0};
  spawn_rectangle(scene, corner28, 80.0, 80.0, OBSTACLE_COLOR_2);

  vector_t bisector_point10 = {LEFT_WALL + 1216, BOTTOM_WALL + 300.0};
  spawn_horz_triangle(scene, bisector_point10, 70.0, OBSTACLE_COLOR_2);

  vector_t corner29 = {LEFT_WALL + 560.0, BOTTOM_WALL + 550.0};
  spawn_rectangle(scene, corner29, 60.0, 150.0, OBSTACLE_COLOR_2);

  vector_t corner30 = {LEFT_WALL + 360.0, BOTTOM_WALL + 550.0};
  spawn_rectangle(scene, corner30, 200.0, 70.0, OBSTACLE_COLOR_2);

  vector_t corner31 = {LEFT_WALL + 850.0, TOP_WALL - 100.0};
  spawn_rectangle(scene, corner31, 50.0, 200.0, OBSTACLE_COLOR_2);

  vector_t corner32 = {LEFT_WALL + 850.0, TOP_WALL - 100.0};
  spawn_rectangle(scene, corner32, 50.0, 400.0, OBSTACLE_COLOR_3);

  vector_t corner33 = {LEFT_WALL + 850.0, TOP_WALL - 500.0};
  spawn_rectangle(scene, corner33, 50.0, 200, OBSTACLE_COLOR_3);

  vector_t corner34 = {LEFT_WALL + 1050.0, TOP_WALL - 280.0};
  spawn_rectangle(scene, corner34, 330.0, 35.0, OBSTACLE_COLOR_3);

  vector_t corner35 = {LEFT_WALL + 1050.0, TOP_WALL - 450.0};
  spawn_rectangle(scene, corner35, 330.0, 35.0, OBSTACLE_COLOR_3);

  vector_t bisector_point11 = {LEFT_WALL + 545.0, TOP_WALL - 140.0};
  spawn_vert_triangle(scene, bisector_point11, -180.0, OBSTACLE_COLOR_2);
}