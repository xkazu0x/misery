internal SR_Canvas
sr_canvas_zero(void) {
  SR_Canvas result = {0};
  return(result);
}

internal SR_Canvas
sr_canvas_make(u32 *data, s32 w, s32 h, s32 pitch) {
  SR_Canvas result = {0};
  result.w = w;
  result.h = h;
  result.pitch = pitch;
  result.data = data;
  return(result);
}

internal SR_Canvas
sr_canvas_clip(SR_Canvas canvas, s32 x, s32 y, s32 w, s32 h) {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x > canvas.w) x = canvas.w;
  if (y > canvas.h) y = canvas.h;
  if (x + w > canvas.w) w = canvas.w - x;
  if (y + h > canvas.h) h = canvas.h - y;
  SR_Canvas result = sr_canvas_make((canvas.data + y*canvas.pitch + x), w, h, canvas.pitch);
  return(result);
}

internal SR_Canvas
sr_canvas_alloc(Arena *arena, s32 w, s32 h) {
  u32 *data = push_array(arena, u32, w*h);
  SR_Canvas result = sr_canvas_make(data, w, h, w);
  return(result);
}

internal SR_Font
sr_font_make(Arena *arena, SR_Canvas atlas, s32 glyph_w, s32 glyph_h, String8 *chars) {
  SR_Font result = {0};
  result.atlas = atlas;
  result.chars = chars;
  result.glyph_w = glyph_w;
  result.glyph_h = glyph_h;
  result.num_glyph_x = atlas.w/glyph_w;
  result.num_glyph_y = atlas.h/glyph_h;
  result.glyphs = push_array(arena, SR_Canvas, chars->size);
  for (u64 idx = 0; idx < chars->size; idx += 1) {
    s32 x = glyph_w*(idx%result.num_glyph_x);
    s32 y = glyph_h*(idx/result.num_glyph_x);
    result.glyphs[idx] = sr_canvas_clip(atlas, x, y, glyph_w, glyph_h);
  }
  return(result);
}

internal Vector2
sr_font_text_size(SR_Font font, String8 text) {
  Vector2 result = vector2_make(0, font.glyph_h);
  s32 dx = 0;
  for (u64 idx = 0; idx < text.size; idx += 1) {
    u8 c = text.str[idx];
    switch (c) {
      case '\n': {
        dx = 0;
        result.y += font.glyph_h;
        continue;
      } break;
      case '\t': {
        dx += font.glyph_w*2;
        continue;
      } break;
    }
    dx += font.glyph_w;
    if (dx > result.x) result.x = dx;
  }
  return(result);
}

internal u32
sr_pget(SR_Canvas canvas, s32 x, s32 y) {
  u32 result = 0;
  if (x >= 0 && x < canvas.w && y >= 0 && y < canvas.h) {
    result = canvas.data[y*canvas.pitch + x];
  }
  return(result);
}

internal void
sr_pset(SR_Canvas canvas, s32 x, s32 y, u32 color) {
  if (x >= 0 && x < canvas.w && y >= 0 && y < canvas.h) {
    canvas.data[y*canvas.pitch + x] = color;
  }
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
sr_line(SR_Canvas canvas, s32 x0, s32 y0, s32 x1, s32 y1, u32 color) {
#if 0
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
        sr_pset(canvas, y, x, color);
      } else {
        sr_pset(canvas, x, y, color);
      }
      if (d >= 0) {
        y += dir;
        d = d - 2*dx;
      }
      d = d + 2*dy;
    }
  }
#else
  s32 dx = x1 - x0;
  s32 dy = y1 - y0;
  s32 len = (abs_t(s32, dx) > abs_t(s32, dy)) ? abs_t(s32, dx) : abs_t(s32, dy);
  f32 x_inc = dx/(f32)len;
  f32 y_inc = dy/(f32)len;
  f32 x = x0;
  f32 y = y0;
  for (s32 idx = 0; idx <= len; idx += 1) {
    sr_pset(canvas, round_t(s32, x), round_t(s32, y), color);
    x += x_inc;
    y += y_inc;
  }
#endif
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
    sr_lineh(canvas, cy+y, cx-x, cx+x, color);
    if (y != 0) {
      sr_lineh(canvas, cy-y, cx-x, cx+x, color);
    }
    if (err < 0) {
      err += 2*y + 3;
    } else {
      if (x != y) {
        sr_lineh(canvas, cy+x, cx-y, cx+y, color);
        if (x != 0) {
          sr_lineh(canvas, cy-x, cx-y, cx+y, color);
        }
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
    sr_pset(canvas, cx+x, cy+y, color);
    sr_pset(canvas, cx-x, cy+y, color);
    sr_pset(canvas, cx+x, cy-y, color);
    sr_pset(canvas, cx-x, cy-y, color);
    sr_pset(canvas, cx+y, cy+x, color);
    sr_pset(canvas, cx-y, cy+x, color);
    sr_pset(canvas, cx+y, cy-x, color);
    sr_pset(canvas, cx-y, cy-x, color);
  }
}

internal void
sr_image(SR_Canvas canvas,
         s32 x0, s32 y0, s32 x1, s32 y1,
         s32 u0, s32 v0, s32 u1, s32 v1,
         SR_Canvas image, u32 color) {
  if (x0 > x1) swap_t(s32, x0, x1);
  if (y0 > y1) swap_t(s32, y0, y1);
  s32 dw = x1 - x0;
  s32 dh = y1 - y0;
  s32 sw = u1 - u0;
  s32 sh = v1 - v0;
  f32 xm = (1.0f/dw)*sw;
  f32 ym = (1.0f/dh)*sh;
  if (x0 < 0) {u0 = -x0; x0 = 0;}
  if (y0 < 0) {v0 = -y0; y0 = 0;}
  if (x1 > canvas.w - 1) x1 = canvas.w - 1;
  if (y1 > canvas.h - 1) y1 = canvas.h - 1;
  for (s32 y = y0; y <= y1; y += 1) {
    s32 v = round_t(s32, (y - y0)*ym) + v0;
    for (s32 x = x0; x <= x1; x += 1) {
      s32 u = round_t(s32, (x - x0)*xm) + u0;
      u32 c = image.data[v*image.pitch + u];
      if (c != 0xffff00ff) {
        canvas.data[y*canvas.pitch + x] = bgra32_mul32(c, color);
      }
    }
  }
}

internal void
sr_char(SR_Canvas canvas, s32 x, s32 y, char c, SR_Font font, u32 color) {
  u64 idx = str8_find_needle(*font.chars, str8_make((u8 *)(&c), 1), 0);
  SR_Canvas glyph = font.glyphs[idx];
  sr_image(canvas, x, y, x+glyph.w-1, y+glyph.h-1, 0, 0, glyph.w-1, glyph.h-1, glyph, color);
}

internal void
sr_text(SR_Canvas canvas, s32 x, s32 y, String8 text, SR_Font font, u32 color) {
  s32 xx = x;
  s32 yy = y;
  for (u64 idx = 0; idx < text.size; idx += 1) {
    u8 c = text.str[idx];
    switch (c) {
      case '\n': {
        xx = x;
        yy += font.glyph_h;
        continue;
      } break;
      case '\t': {
        xx += font.glyph_w*2;
        continue;
      } break;
    }
    sr_char(canvas, xx, yy, c, font, color);
    xx += font.glyph_w;
  }
}
