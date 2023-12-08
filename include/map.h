#ifndef __map_H__
#define __map_H__

#include "body.h"
#include "list.h"
#include "scene.h"
#include "sdl_wrapper.h"

extern const size_t RECTANGLE_OBSTACLE_TYPE;
extern const size_t TRIANGLE_OBSTACLE_TYPE;
extern const double TRIANGLE_DAMAGE;

list_t *make_rectangle(vector_t corner, double width, double height);

/**
 * This function initializes the game map and is called once
 *
 * @param scene the scene
 */
void map_init(scene_t *scene);

#endif // #ifndef __STAR_H__