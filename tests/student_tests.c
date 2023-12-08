#include "color.h"
#include "forces.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

list_t *make_shape() {
  list_t *shape = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){-1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){1, 1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-1, 1};
  list_add(shape, v);
  return shape;
}

double spring_energy(body_t *b1, body_t *b2, double k) {
  vector_t distance =
      vec_subtract(body_get_centroid(b1), body_get_centroid(b2));
  return .5 * k * vec_dot(distance, distance);
}

double kinetic_energy(body_t *body) {
  vector_t velocity = body_get_velocity(body);
  return body_get_mass(body) * vec_dot(velocity, velocity) / 2;
}

void test_gravity() {
  const double M1 = 1;
  const double M2 = 100000;
  const double G = 1e3;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  scene_t *scene = scene_init();
  body_t *grav_obj = body_init(make_shape(), M2, (rgb_color_t){0, 0, 0});
  scene_add_body(scene, grav_obj);
  body_t *pulled_obj = body_init(make_shape(), M1, (rgb_color_t){0, 0, 0});
  scene_add_body(scene, pulled_obj);
  create_newtonian_gravity(scene, G, grav_obj, pulled_obj);
  for (int i = 0; i < STEPS; i++) {
    vector_t v = vec_subtract(body_get_centroid(grav_obj),
                              body_get_centroid(pulled_obj));
    double magnitude = pow(vec_dot(v, v), 0.5);
    assert(within(1e-1, magnitude, 0));
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

void test_drag() {
  const double M = 100;
  const double K = 2;
  const double V = 5;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  double elapsed_time = 0;
  scene_t *scene = scene_init();
  body_t *mass = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_velocity(mass, (vector_t){V, 0});
  scene_add_body(scene, mass);
  create_drag(scene, K, mass);
  for (int i = 0; i < STEPS; i++) {
    elapsed_time = i * DT;
    assert(vec_isclose(body_get_velocity(mass),
                       (vector_t){V * pow(M_E, -K * elapsed_time / M), 0}));
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

void test_spring_energy() {
  const double M = 100;
  const double K = 2;
  const double A = 3;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  scene_t *scene = scene_init();
  body_t *mass = body_init(make_shape(), M, (rgb_color_t){0, 0, 0});
  body_set_centroid(mass, (vector_t){A, 0});
  scene_add_body(scene, mass);
  body_t *anchor = body_init(make_shape(), INFINITY, (rgb_color_t){0, 0, 0});
  scene_add_body(scene, anchor);
  create_spring(scene, K, mass, anchor);
  double initial_energy = spring_energy(mass, anchor, K);
  for (int i = 0; i < STEPS; i++) {
    double new_energy = spring_energy(mass, anchor, K) + kinetic_energy(mass);
    assert(within(1e-1, new_energy, initial_energy));
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_gravity)
  DO_TEST(test_drag)
  DO_TEST(test_spring_energy)

  puts("student_tests PASS");
}
