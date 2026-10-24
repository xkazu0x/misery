#ifndef SR_H
#define SR_H

typedef struct SR_Canvas SR_Canvas;
struct SR_Canvas {
  s32 w, h;
  s32 pitch;
  u32 *data;
};

typedef struct SR_Font SR_Font;
struct SR_Font {
  SR_Canvas atlas;
  String8 *chars;
  s32 glyph_w;
  s32 glyph_h;
  s32 num_glyph_x;
  s32 num_glyph_y;
  SR_Canvas *glyphs;
};

internal SR_Canvas sr_canvas_zero(void);
internal SR_Canvas sr_canvas_make(u32 *data, s32 w, s32 h, s32 pitch);
internal SR_Canvas sr_canvas_from(SR_Canvas canvas, s32 x, s32 y, s32 w, s32 h);
internal SR_Canvas sr_canvas_alloc(Arena *arena, s32 w, s32 h);

internal SR_Font sr_font_make(Arena *arena, SR_Canvas atlas, s32 glyph_w, s32 glyph_h, String8 *chars);
internal Vector2 sr_font_calc_text_size(SR_Font font, String8 text);

internal void sr_fill(SR_Canvas canvas, u32 color);
internal void sr_pixel(SR_Canvas canvas, s32 x, s32 y, u32 color);
internal void sr_lineh(SR_Canvas canvas, s32 y, s32 x0, s32 x1, u32 color);
internal void sr_linev(SR_Canvas canvas, s32 x, s32 y0, s32 y1, u32 color);
internal void sr_line(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
internal void sr_rectf(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
internal void sr_rectl(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
internal void sr_circf(SR_Canvas canvas, s32 cx, s32 cy, s32 r, u32 color);
internal void sr_circl(SR_Canvas canvas, s32 cx, s32 cy, s32 r, u32 color);
internal void sr_image(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, s32 u0, s32 v0, s32 u1, s32 v1, SR_Canvas image, u32 color);
internal void sr_char(SR_Canvas canvas, s32 x, s32 y, char c, SR_Font font, u32 color);
internal void sr_text(SR_Canvas canvas, s32 x, s32 y, String8 text, SR_Font font, u32 color);

#endif // SR_H
