#ifndef __BODY_H__
#define __BODY_H__

#include "color.h"
#include "list.h"
#include "vector.h"
#include <stdbool.h>

// types used across files
extern const double BULLET_DAMAGE;
extern const size_t WALL_TYPE;
extern const size_t BULLET_TYPE;
extern const size_t DEFAULT_TANK_TYPE;
extern const size_t HEALTH_BAR_TYPE;
extern const size_t SNIPER_BULLET_TYPE;
extern const size_t GATLING_BULLET_TYPE;

// ai modes
extern const size_t AI_UP;
extern const size_t AI_DOWN;
extern const size_t AI_UP_LEFT;
extern const size_t AI_UP_RIGHT;
extern const size_t AI_DOWN_LEFT;
extern const size_t AI_DOWN_RIGHT;
extern const size_t AI_180;
extern const size_t AI_90_LEFT;
extern const size_t AI_90_RIGHT;

/**
 * A rigid body constrained to the plane.
 * Implemented as a polygon with uniform density.
 * Bodies can accumulate forces and impulses during each tick.
 * Angular physics (i.e. torques) are not currently implemented.
 */
typedef struct body body_t;

/**
 * Graphic struct that represents a visual element
 * on the screen, including text.
 */
typedef struct graphic graphic_t;

/**
 * Initializes a body without any info.
 * Acts like body_init_with_info() where info and info_freer are NULL.
 */
body_t *body_init(list_t *shape, double mass, rgb_color_t color);

/**
 * Allocates memory for a body with the given parameters.
 * The body is initially at rest.
 * Asserts that the mass is positive and that the required memory is allocated.
 *
 * @param shape a list of vectors describing the initial shape of the body
 * @param mass the mass of the body (if INFINITY, stops the body from moving)
 * @param color the color of the body, used to draw it on the screen
 * @param info additional information to associate with the body,
 *   e.g. its type if the scene has multiple types of bodies
 * @param info_freer if non-NULL, a function call on the info to free it
 * @return a pointer to the newly allocated body
 */
body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer);

/**
 * Releases the memory allocated for a body.
 *
 * @param body a pointer to a body returned from body_init()
 */
void body_free(body_t *body);

/**
 * Gets the current shape of a body.
 * Returns a newly allocated vector list, which must be list_free()d.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the polygon describing the body's current position
 */
list_t *body_get_shape(body_t *body);

/**
 * Gets the current center of mass of a body.
 * While this could be calculated with polygon_centroid(), that becomes too slow
 * when this function is called thousands of times every tick.
 * Instead, the body should store its current centroid.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the body's center of mass
 */
vector_t body_get_centroid(body_t *body);

/**
 * Gets the current velocity of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the body's velocity vector
 */
vector_t body_get_velocity(body_t *body);

/**
 * Gets the current rotation of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the body's rotation angle
 */
double body_get_rotation(body_t *body);

/**
 * Gets the mass of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the mass passed to body_init(), which must be greater than 0
 */
double body_get_mass(body_t *body);

double body_get_time(body_t *body);

double body_get_health(body_t *body);

/**
 * Gets the display color of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the color passed to body_init(), as an (R, G, B) tuple
 */
rgb_color_t body_get_color(body_t *body);

/**
 * Gets the information associated with a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the info passed to body_init()
 */
void *body_get_info(body_t *body);

double body_get_magnitude(body_t *);

bool body_get_just_collided(body_t *body);

void body_set_graphic(body_t *body, graphic_t *graphic);

void body_combine_mass(body_t *body1, body_t *body2);
/**
 * Translates a body to a new position.
 * The position is specified by the position of the body's center of mass.
 *
 * @param body a pointer to a body returned from body_init()
 * @param x the body's new centroid
 */
void body_set_centroid(body_t *body, vector_t x);

/**
 * Changes a body's velocity (the time-derivative of its position).
 *
 * @param body a pointer to a body returned from body_init()
 * @param v the body's new velocity
 */
void body_set_velocity(body_t *body, vector_t v);

void body_set_shape(body_t *body, list_t *shape);

void body_set_rotation_speed(body_t *body, double w);

void body_set_magnitude(body_t *body, double magnitude);

void body_set_health(body_t *body, double health);

/**
 * Changes a body's orientation in the plane.
 * The body is rotated about its center of mass.
 * Note that the angle is *absolute*, not relative to the current orientation.
 *
 * @param body a pointer to a body returned from body_init()
 * @param angle the body's new angle in radians. Positive is counterclockwise.
 */
void body_set_rotation(body_t *body, double angle);

/**
 * Applies a force to a body over the current tick.
 * If multiple forces are applied in the same tick, they should be added.
 * Should not change the body's position or velocity; see body_tick().
 *
 * @param body a pointer to a body returned from body_init()
 * @param force the force vector to apply
 */
void body_add_force(body_t *body, vector_t force);

/**
 * Applies an impulse to a body.
 * An impulse causes an instantaneous change in velocity,
 * which is useful for modeling collisions.
 * If multiple impulses are applied in the same tick, they should be added.
 * Should not change the body's position or velocity; see body_tick().
 *
 * @param body a pointer to a body returned from body_init()
 * @param impulse the impulse vector to apply
 */
void body_add_impulse(body_t *body, vector_t impulse);

void body_set_time(body_t *body, double time);

void body_set_rotation_empty(body_t *body, double rotation);
/**
 * Updates the body after a given time interval has elapsed.
 * Sets acceleration and velocity according to the forces and impulses
 * applied to the body during the tick.
 * The body should be translated at the *average* of the velocities before
 * and after the tick.
 * Resets the forces and impulses accumulated on the body.
 *
 * @param body the body to tick
 * @param dt the number of seconds elapsed since the last tick
 */
void body_tick(body_t *body, double dt);

/**
 * Marks a body for removal--future calls to body_is_removed() will return true.
 * Does not free the body.
 * If the body is already marked for removal, does nothing.
 *
 * @param body the body to mark for removal
 */
void body_remove(body_t *body);

/**
 * Returns whether a body has been marked for removal.
 * This function returns false until body_remove() is called on the body,
 * and returns true afterwards.
 *
 * @param body the body to check
 * @return whether body_remove() has been called on the body
 */
bool body_is_removed(body_t *body);

double body_get_distance(vector_t body1_centroid, vector_t body2_centroid);

double body_get_mass(body_t *body);

size_t body_get_ai_mode(body_t *body);

void body_set_ai_mode(body_t *body, size_t mode);

double body_get_ai_time(body_t *body);

void body_set_ai_time(body_t *body, double time);

void body_set_just_collided(body_t *body, bool just_collided);

void body_set_image_path(body_t *body, char *image_path);

char *body_get_image_path(body_t *body);

body_t *init_default_tank(vector_t center, double side_length,
                          vector_t velocity, double mass, rgb_color_t color,
                          double max_health, size_t tank_type);

body_t *init_gravity_tank(vector_t center, double side_length,
                          vector_t velocity, double mass, rgb_color_t color,
                          double max_health, size_t tank_type);

body_t *init_sniper_tank(vector_t center, double side_length, vector_t velocity,
                         double mass, rgb_color_t color, double max_health,
                         size_t tank_type);

body_t *init_gatling_tank(vector_t center, double side_length,
                          vector_t velocity, double mass, rgb_color_t color,
                          double max_health, size_t tank_type);

#endif // #ifndef __BODY_H__
