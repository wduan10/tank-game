#include "star.h"
#include "test_util.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

double MAX_WIDTH = 1000.0;
double MAX_HEIGHT = 500.0;
double LENGTH = 60.0;
double GRAVITATIONAL_CONSTANT = -900.81;
double DAMPING_CONSTANT = 0.85;

double DEFAULT_SPEED = 200.0;
double DEFAULT_ANGLE = M_PI / 4;
double DEFAULT_ROTATION_SPEED = M_PI / 8;

// creates a star for our tests, with all the params necessary to intialize a
// star (from star_init)
star_t *make_default_star(size_t star_points, double max_height, double length,
                          double speed, double angle, double rotation_speed) {
  star_t *star = star_init((int)star_points, max_height, length, speed, angle,
                           rotation_speed);
  return star;
}

// this also tests star_init and star_free
void test_make_star() {
  for (int i = 3; i < 700; i += 6) {
    star_t *star =
        make_default_star((size_t)i, MAX_HEIGHT, LENGTH + i, DEFAULT_SPEED + i,
                          DEFAULT_ANGLE + i, DEFAULT_ROTATION_SPEED + i);
    vector_t *velocity_test = get_star_velocity(star);
    assert(vec_equal(
        (vector_t){velocity_test->x, velocity_test->y},
        (vector_t){(DEFAULT_SPEED + i) * cos(DEFAULT_ANGLE + i),
                   -1 * (DEFAULT_SPEED + i) * sin(DEFAULT_ANGLE + i)}));
    star_free(star);
    free(velocity_test);
  }
}

int main(int argc, char *argv[]) {
  // Run all tests? True if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_make_star);
  puts("star_init PASS");
  puts("star_free PASS");

  puts("star_tests PASS");
}
