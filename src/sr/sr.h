#ifndef SR_H
#define SR_H

typedef struct SR_Canvas SR_Canvas;
struct SR_Canvas {
  u32 *data;
  s32 w;
  s32 h;
  s32 pitch;
};

internal SR_Canvas sr_canvas_zero(void);
internal SR_Canvas sr_canvas_make(u32 *data, s32 w, s32 h, s32 pitch);
internal SR_Canvas sr_canvas_from(SR_Canvas canvas, s32 x, s32 y, s32 w, s32 h);

internal void sr_fill(SR_Canvas canvas, u32 color);
internal void sr_pixel(SR_Canvas canvas, s32 x, s32 y, u32 color);
internal void sr_lineh(SR_Canvas canvas, s32 y, s32 x0, s32 x1, u32 color);
internal void sr_linev(SR_Canvas canvas, s32 x, s32 y0, s32 y1, u32 color);
internal void sr_line(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
internal void sr_rectf(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
internal void sr_rectl(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
internal void sr_circf(SR_Canvas canvas, s32 cx, s32 cy, s32 r, u32 color);
internal void sr_circl(SR_Canvas canvas, s32 cx, s32 cy, s32 r, u32 color);

internal void sr__circf_lines_horz(SR_Canvas canvas, s32 cx, s32 cy, s32 x, s32 y, u32 color);

#endif // SR_H
