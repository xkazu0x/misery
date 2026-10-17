internal SR_Canvas
sr_canvas_zero(void) {
  SR_Canvas result = {0};
  return(result);
}

internal SR_Canvas
sr_canvas_make(u32 *data, s32 w, s32 h, s32 pitch) {
  SR_Canvas result;
  result.data = data;
  result.w = w;
  result.h = h;
  result.pitch = pitch;
  return(result);
}

internal SR_Canvas
sr_canvas_from(SR_Canvas canvas, s32 x, s32 y, s32 w, s32 h) {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x > canvas.w) x = canvas.w;
  if (y > canvas.h) y = canvas.h;
  if (x + w > canvas.w) w = canvas.w - x;
  if (y + h > canvas.h) h = canvas.h - y;
  SR_Canvas result = sr_canvas_make((canvas.data + y*canvas.pitch + x), w, h, canvas.pitch);
  return(result);
}

internal void
sr_fill(SR_Canvas canvas, u32 color) {
  for (s32 y = 0; y < canvas.h; y += 1) {
    for (s32 x = 0; x < canvas.w; x += 1) {
      canvas.data[y*canvas.pitch + x] = color;
    }
  }
}

internal void
sr_pixel(SR_Canvas canvas, s32 x, s32 y, u32 color) {
  if (x >= 0 && x < canvas.w && y >= 0 && y < canvas.h) {
    canvas.data[y*canvas.pitch + x] = color;
  }
}

internal void
sr_lineh(SR_Canvas canvas, s32 y, s32 x0, s32 x1, u32 color) {
  if (x0 < 0) x0 = 0;
  if (x1 > canvas.w - 1) x1 = canvas.w - 1;
  if (y >= 0 && y < canvas.h) {
    for (s32 x = x0; x <= x1; x += 1) {
      canvas.data[y*canvas.pitch + x] = color;
    }
  }
}

internal void
sr_linev(SR_Canvas canvas, s32 x, s32 y0, s32 y1, u32 color) {
  if (y0 < 0) y0 = 0;
  if (y1 > canvas.h - 1) y1 = canvas.h - 1;
  if (x >= 0 && x < canvas.h) {
    for (s32 y = y0; y <= y1; y += 1) {
      canvas.data[y*canvas.pitch + x] = color;
    }
  }
}

internal void
sr_line(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color) {
  b32 is_steep = (abs_t(s32, y1 - y0) > abs_t(s32, x1 - x0));
  if (is_steep) {
    swap_t(s32, x0, y0);
    swap_t(s32, x1, y1);
  }
  if (x0 > x1) {
    swap_t(s32, x0, x1);
    swap_t(s32, y0, y1);
  }
  s32 dx = x1 - x0;
  s32 dy = y1 - y0;
  s32 dir = (dy < 0) ? -1 : 1;
  dy *= dir;
  if (dx != 0) {
    s32 y = y0;
    s32 d = 2*dy - dx;
    for (s32 x = x0; x <= x1; x += 1) {
      if (is_steep) {
        sr_pixel(canvas, y, x, color);
      } else {
        sr_pixel(canvas, x, y, color);
      }
      if (d >= 0) {
        y += dir;
        d = d - 2*dx;
      }
      d = d + 2*dy;
    }
  }
}

internal void
sr_rectf(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color) {
  if (x0 > x1) swap_t(s32, x0, x1);
  if (y0 > y1) swap_t(s32, y0, y1);
  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 > canvas.w - 1) x1 = canvas.w - 1;
  if (y1 > canvas.h - 1) y1 = canvas.h - 1;
  for (s32 y = y0; y <= y1; y += 1) {
    for (s32 x = x0; x <= x1; x += 1) {
      canvas.data[y*canvas.pitch + x] = color;
    }
  }
}

internal void
sr_rectl(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color) {
  sr_lineh(canvas, y0, x0, x1, color);
  sr_lineh(canvas, y1, x0, x1, color);
  sr_linev(canvas, x0, y0, y1, color);
  sr_linev(canvas, x1, y0, y1, color);
}

internal void
sr_circf(SR_Canvas canvas, s32 cx, s32 cy, s32 r, u32 color) {
  s32 x = r;
  s32 err = 1 - r;
  for (s32 y = 0; y <= x; y += 1) {
    sr__circf_lines_horz(canvas, cx, cy, x, y, color);
    if (err < 0) {
      err += 2*y + 3;
    } else {
      if (x != y) {
        sr__circf_lines_horz(canvas, cx, cy, y, x, color);
      }
      x -= 1;
      err += 2*(y - x) + 3;
    }
  }
}

internal void
sr_circl(SR_Canvas canvas, s32 cx, s32 cy, s32 r, u32 color) {
  r = abs_t(s32, r);
  s32 y = -r;
  s32 p = -r;
  for (s32 x = 0; x < -y; x += 1) {
    if (p > 0) {
      y += 1;
      p += 2*(x + y) + 1;
    } else {
      p += 2*x + 1;
    }
    sr_pixel(canvas, cx+x, cy+y, color);
    sr_pixel(canvas, cx-x, cy+y, color);
    sr_pixel(canvas, cx+x, cy-y, color);
    sr_pixel(canvas, cx-x, cy-y, color);
    sr_pixel(canvas, cx+y, cy+x, color);
    sr_pixel(canvas, cx-y, cy+x, color);
    sr_pixel(canvas, cx+y, cy-x, color);
    sr_pixel(canvas, cx-y, cy-x, color);
  }
}

internal void
sr__circf_lines_horz(SR_Canvas canvas, s32 cx, s32 cy, s32 x, s32 y, u32 color) {
  sr_lineh(canvas, cy+y, cx-x, cx+x, color);
  if (y != 0) {
    sr_lineh(canvas, cy-y, cx-x, cx+x, color);
  }
}
