#include "vector.h"
#include <math.h>
#include <stdio.h>

const vector_t VEC_ZERO = {0.0, 0.0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t resultant;
  resultant.x = v1.x + v2.x;
  resultant.y = v1.y + v2.y;
  return resultant;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  vector_t resultant;
  resultant = vec_add(v1, vec_negate(v2));
  return resultant;
}

vector_t vec_negate(vector_t v) {
  vector_t result;
  result.x = -v.x;
  result.y = -v.y;
  return result;
}

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t result;
  result.x = v.x * scalar;
  result.y = v.y * scalar;
  return result;
}

double vec_dot(vector_t v1, vector_t v2) {
  double resultant = v1.x * v2.x + v1.y * v2.y;
  return resultant;
}

double vec_cross(vector_t v1, vector_t v2) {
  double resultant = (v1.x * v2.y) - (v2.x * v1.y);
  return resultant;
}

vector_t vec_rotate(vector_t v, double angle) {
  double cosine = cos(angle);
  double sine = sin(angle);
  vector_t rotated;
  rotated.x = v.x * cosine - v.y * sine;
  rotated.y = v.x * sine + v.y * cosine;
  return rotated;
}