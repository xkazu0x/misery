#ifndef SR_H
#define SR_H

typedef struct SR_Canvas SR_Canvas;
struct SR_Canvas {
  s32 width;
  s32 height;
  s32 pitch;
  u32 *data;
};

internal SR_Canvas *sr_canvas_alloc(Arena *arena, s32 width, s32 height, s32 pitch);

internal void sr_fill(SR_Canvas *canvas, u32 color);
internal void sr_rect(SR_Canvas *canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);

#endif // SR_H
