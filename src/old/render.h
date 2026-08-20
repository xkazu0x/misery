#ifndef RENDER_H
#define RENDER_H

typedef enum R_Scaling_Mode {
  R_Scaling_Mode_ASPECT_RATIO,
  R_Scaling_Mode_FULLSCREEN,
  R_Scaling_Mode_NO_SCALING,
  R_Scaling_Mode_INTEGER_SCALING,
} R_Scaling_Mode;

typedef struct R_Texture R_Texture;
struct R_Texture {
  s32 w, h;
  s32 pitch;
  u32 *data;
};

typedef struct R_Font R_Font;
struct R_Font {
  String8 chars;
  u32 glyph_w;
  u32 glyph_h;
  u32 glyph_count_x;
  u32 glyph_count_y;
  R_Texture atlas;
};

////////////////////////////////
// NOTE: Color Helpers

internal u32     color_pack(u8 r, u8 g, u8 b, u8 a);
internal void    color_unpack(u32 color, u8 *r, u8 *g, u8 *b, u8 *a);
internal u32     color_from_normal(Vector4 v);
internal Vector4 normal_from_color(u32 color);
internal u32     color_mul(u32 color, Vector4 v);

////////////////////////////////
// NOTE: Texture

internal R_Texture r_texture_make(Arena *arena, u32 w, u32 h, u32 *data);

////////////////////////////////
// NOTE: Font

internal R_Font  r_font_make(Arena *arena, u32 atlas_w, u32 atlas_h, u32 *atlas_data, u32 glyph_w, u32 glyph_h, String8 chars);
internal u64     r_font_glyph_idx_from_char(R_Font font, u8 c);
internal Vector2 r_text_size_from_font(R_Font font, String8 text);

////////////////////////////////
// NOTE: Draw Calls

internal void r_clear(R_Texture output, u32 color);
internal void r_rect(R_Texture output, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
internal void r_img(R_Texture output, s32 x0, s32 y0, s32 x1, s32 y1, s32 u0, s32 v0, s32 u1, s32 v1, u32 color, R_Texture texture);
internal void r_char(R_Texture output, R_Font font, s32 x, s32 y, u32 color, u8 c);
internal void r_text(R_Texture output, R_Font font, s32 x, s32 y, u32 color, String8 text);

////////////////////////////////
// NOTE: Window Blitting

internal void r_window_blit(WM_Window window, R_Texture texture, R_Scaling_Mode mode);

#endif // RENDER_H
