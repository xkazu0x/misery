#ifndef SR_H
#define SR_H

typedef struct SR_Canvas SR_Canvas;
struct SR_Canvas {
  s32 w;
  s32 h;
  s32 p;
  u32 *data;
};

internal SR_Canvas sr_canvas_make(u32 *data, s32 w, s32 h, s32 p);

internal void sr_fill(SR_Canvas *canvas, u32 color);
internal void sr_pixel(SR_Canvas *canvas, s32 x, s32 y, u32 color);
internal void sr_line(SR_Canvas *canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
internal void sr_rect(SR_Canvas *canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
internal void sr_rectl(SR_Canvas *canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
internal void sr_circ(SR_Canvas *canvas, s32 cx, s32 cy, s32 r, u32 color);
internal void sr_circl(SR_Canvas *canvas, s32 cx, s32 cy, s32 r, u32 color);

#endif // SR_H
