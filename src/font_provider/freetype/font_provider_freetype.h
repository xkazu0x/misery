#ifndef FONT_PROVIDER_FREETYPE_H
#define FONT_PROVIDER_FREETYPE_H

////////////////////////////////
// NOTE: Includes

#undef internal
#include <ft2build.h>
#include FT_FREETYPE_H
#define internal static

////////////////////////////////
// NOTE: State Types

typedef struct FP_FT_Font FP_FT_Font;
struct FP_FT_Font {
  FT_Face face;
};

typedef struct FP_FT_State FP_FT_State;
struct FP_FT_State {
  Arena *arena;
  FT_Library library;
};

////////////////////////////////
// NOTE: Globals

global FP_FT_State *fp_ft_state = 0;

////////////////////////////////
// NOTE: Helpers

internal FP_Handle  fp_ft_handle_from_font(FP_FT_Font font);
internal FP_FT_Font fp_ft_font_from_handle(FP_Handle handle);

#endif // FONT_PROVIDER_FREETYPE_H
