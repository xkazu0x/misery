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

internal SR_Font
sr_font_make(Arena *arena, SR_Canvas atlas, s32 gw, s32 gh, String8 chars) {
  SR_Font result = {0};
  result.atlas = atlas;
  result.gw = gw;
  result.gh = gh;
  result.gx = atlas.w/gw;
  result.gy = atlas.h/gh;
  result.chars = str8_copy(arena, chars);
  result.glyphs = push_array(arena, SR_Canvas, result.chars.size);
  for (u64 idx = 0; idx < result.chars.size; idx += 1) {
    s32 x = gw*(idx%result.gx);
    s32 y = gh*(idx/result.gx);
    result.glyphs[idx] = sr_canvas_from(atlas, x, y, gw, gh);
  }
  return(result);
}

internal void
sr_font_calc_dim(SR_Font font, String8 text, s32 *w, s32 *h) {
  s32 mx = 0;
  s32 my = 0;
  s32 dx = 0;
  for (u64 idx = 0; idx < text.size; idx += 1) {
    u8 c = text.str[idx];
    switch (c) {
      case '\n': {
        dx = 0;
        my += font.gh;
        continue;
      } break;
      case '\t': {
        dx += font.gw*2;
        continue;
      } break;
    }
    dx += font.gw;
    if (dx > mx) mx = dx;
  }
  if (w) *w = mx;
  if (h) *h = my;
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
sr_img(SR_Canvas canvas,
       s32 x0, s32 y0, s32 x1, s32 y1,
       s32 u0, s32 v0, s32 u1, s32 v1,
       SR_Canvas image, u32 color) {
  if (x0 > x1) swap_t(s32, x0, x1);
  if (y0 > y1) swap_t(s32, y0, y1);
  s32 min_x = x0;
  s32 min_y = y0;
  s32 max_x = x1;
  s32 max_y = y1;
  s32 dst_w = max_x - min_x;
  s32 dst_h = max_y - min_y;
  s32 min_u = max(u0, 0);
  s32 min_v = max(v0, 0);
  s32 max_u = min(u1, image.w);
  s32 max_v = min(v1, image.h);
  s32 src_w = max_u - min_u;
  s32 src_h = max_v - min_v;
  if (min_x < 0) min_x = 0;
  if (min_y < 0) min_y = 0;
  if (max_x > canvas.w) max_x = canvas.w;
  if (max_y > canvas.h) max_y = canvas.h;
  u32 *src_row = image.data  + min_v*image.pitch  + min_u;
  u32 *dst_row = canvas.data + min_y*canvas.pitch + min_x;
  f32 mw = 1.0f/dst_w;
  f32 mh = 1.0f/dst_h;
  for (s32 y = min_y; y < max_y; y += 1) {
    s32 src_v = (s32)((y - min_y)*mh*src_h);
    u32 *src_pixel = src_row + src_v*image.pitch;
    u32 *dst_pixel = dst_row;
    for (s32 x = min_x; x < max_x; x += 1) {
      s32 src_u = (s32)((x - min_x)*mw*src_w);
      u32 src_color = *(src_pixel + src_u);
      if (src_color != 0xffff00ff) {
        *dst_pixel = bgra32_mul(src_color, color);
      }
      dst_pixel += 1;
    }
    dst_row += canvas.pitch;
  }
}

internal void
sr_char(SR_Canvas canvas, s32 x, s32 y, char c, SR_Font font, u32 color) {
  u64 idx = str8_find_needle(font.chars, str8_make((u8 *)(&c), 1), 0);
  SR_Canvas g = font.glyphs[idx];
  sr_img(canvas, x, y, x+g.w, y+g.h, 0, 0, g.w, g.h, g, color);
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
        yy += font.gh;
        continue;
      } break;
      case '\t': {
        xx += font.gw*2;
        continue;
      } break;
    }
    sr_char(canvas, xx, yy, c, font, color);
    xx += font.gw;
  }
}
