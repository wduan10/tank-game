#include "text.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct text {
  TTF_Font *font;
  free_func_t text_freer;
} text_t;

text_t *text_init(TTF_Font *font, free_func_t text_free) {
  text_t *text = malloc(sizeof(text_t));
  assert(text != NULL);
  text->font = font;
  text->text_freer = text_free;
  return text;
}

TTF_Font *text_get_font(text_t *text) { return text->font; }

void text_free(text_t *text) {
  if (text->text_freer != NULL) {
    text->text_freer(text->font);
  }
  free(text);
}