////////////////////////////////
// NOTE: Color Helpers

internal u32
color_pack(u8 r, u8 g, u8 b, u8 a) {
  u32 result = ((a << 8*3)|(r << 8*2)|(g << 8*1)|(b << 8*0));
  return(result);
}

internal void
color_unpack(u32 color, u8 *r, u8 *g, u8 *b, u8 *a) {
  *a = extract8(color, 3);
  *r = extract8(color, 2);
  *g = extract8(color, 1);
  *b = extract8(color, 0);
}

internal u32
color_from_normal(Vector4 v) {
  u8 r = (u8)(255.0f*v.x);
  u8 g = (u8)(255.0f*v.y);
  u8 b = (u8)(255.0f*v.z);
  u8 a = (u8)(255.0f*v.w);
  u32 result = color_pack(r, g, b, a);
  return(result);
}

internal Vector4
normal_from_color(u32 color) {
  u8 r, g, b, a;
  color_unpack(color, &r, &g, &b, &a);
  Vector4 result = {r/255.0f, g/255.0f, b/255.0f, a/255.0f};
  return(result);
}

internal u32
color_mul(u32 color, Vector4 v) {
  u8 r, g, b, a;
  color_unpack(color, &r, &g, &b, &a);
  r = (u8)(r*v.x);
  g = (u8)(g*v.y);
  b = (u8)(b*v.z);
  a = (u8)(a*v.w);
  u32 result = color_pack(r, g, b, a);
  return(result);
}

////////////////////////////////
// NOTE: Texture

internal R_Texture
r_texture_make(Arena *arena, u32 w, u32 h, u32 *data) {
  R_Texture result = {0};
  result.w = w;
  result.h = h;
  result.pitch = w;
  result.data = data;
  if (result.data == 0) {
    result.data = push_array(arena, u32, w*h);
  }
  return(result);
}

////////////////////////////////
// NOTE: Font

internal R_Font
r_font_make(Arena *arena,
            u32 atlas_w, u32 atlas_h, u32 *atlas_data,
            u32 glyph_w, u32 glyph_h,
            String8 chars) {
  R_Font result = {0};
  result.chars = str8_copy(arena, chars);
  result.glyph_w = glyph_w;
  result.glyph_h = glyph_h;
  result.glyph_count_x = atlas_w/glyph_w;
  result.glyph_count_y = atlas_h/glyph_h;
  result.atlas = r_texture_make(arena, atlas_w, atlas_h, atlas_data);
  return(result);
}

internal u64
r_font_glyph_idx_from_char(R_Font font, u8 c) {
  u64 result = 0;
  for (u64 i = 0; i < font.chars.size; i += 1) {
    if (c == font.chars.str[i]) {
      result = i;
      break;
    }
  }
  return(result);
}

internal Vector2
r_text_size_from_font(R_Font font, String8 text) {
  Vector2 result = {(f32)text.size*font.glyph_w, (f32)font.glyph_h};
  return(result);
}

////////////////////////////////
// NOTE: Draw Calls

internal void
r_clear(R_Texture output, u32 color) {
  u32 *dst_line = output.data;
  for (s32 y = 0; y < output.h; y += 1) {
    u32 *dst_pixel = dst_line;
    for (s32 x = 0; x < output.w; x += 1) {
      *dst_pixel++ = color;
    }
    dst_line += output.pitch;
  }
}

internal void
r_rect(R_Texture output,
       s32 x0, s32 y0, s32 x1, s32 y1,
       u32 color) {
  if (x0 > x1) swap_t(s32, x0, x1);
  if (y0 > y1) swap_t(s32, y0, y1);
  s32 min_x = x0;
  s32 min_y = y0;
  s32 max_x = x1;
  s32 max_y = y1;
  if (min_x < 0) min_x = 0;
  if (min_y < 0) min_y = 0;
  if (max_x > output.w) max_x = output.w;
  if (max_y > output.h) max_y = output.h;
  u32 *dst_line = output.data + min_y*output.pitch + min_x;
  for (s32 y = min_y; y < max_y; y += 1) {
    u32 *dst_pixel = dst_line;
    for (s32 x = min_x; x < max_x; x += 1) {
      *dst_pixel++ = color;
    }
    dst_line += output.pitch;
  }
}

internal void
r_img(R_Texture output,
      s32 x0, s32 y0, s32 x1, s32 y1,
      s32 u0, s32 v0, s32 u1, s32 v1,
      u32 color, R_Texture texture) {
  if (x0 > x1) swap_t(s32, x0, x1);
  if (y0 > y1) swap_t(s32, y0, y1);
  s32 min_x = x0;
  s32 min_y = y0;
  s32 max_x = x1;
  s32 max_y = y1;
  s32 min_u = max(u0, 0);
  s32 min_v = max(v0, 0);
  s32 max_u = min(u1, texture.w);
  s32 max_v = min(v1, texture.h);
  f32 dst_w = (f32)(max_x - min_x);
  f32 dst_h = (f32)(max_y - min_y);
  s32 src_w = max_u - min_u;
  s32 src_h = max_v - min_v;
  if (min_x < 0) min_x = 0;
  if (min_y < 0) min_y = 0;
  if (max_x > output.w) max_x = output.w;
  if (max_y > output.h) max_y = output.h;
  u32 *src_line = texture.data + min_v*texture.pitch + min_u;
  u32 *dst_line = output.data + min_y*output.pitch + min_x;
  for (s32 y = min_y; y < max_y; y += 1) {
    f32 dst_y = (f32)(y - min_y);
    f32 mul_y = dst_y/dst_h;
    s32 src_v = (s32)(mul_y*src_h);
    u32 *src_pixel = src_line + src_v*texture.pitch;
    u32 *dst_pixel = dst_line;
    for (s32 x = min_x; x < max_x; x += 1) {
      f32 dst_x = (f32)(x - min_x);
      f32 mul_x = dst_x/dst_w;
      s32 src_u = (s32)(mul_x*src_w);
      u32 src_color = *(src_pixel + src_u);
      if (src_color != 0xffff00ff) {
        *dst_pixel = color_mul(src_color, normal_from_color(color));
      }
      dst_pixel += 1;
    }
    dst_line += output.pitch;
  }
}

internal void
r_char(R_Texture output, R_Font font, s32 x, s32 y, u32 color, u8 c) {
  u64 glyph_idx = r_font_glyph_idx_from_char(font, c);
  u32 glyph_w = font.glyph_w;
  u32 glyph_h = font.glyph_h;
  u32 u = (u32)(glyph_w*(glyph_idx%font.glyph_count_x));
  u32 v = (u32)(glyph_h*(glyph_idx/font.glyph_count_x));
  r_img(output,
        x, y, x+glyph_w, y+glyph_h,
        u, v, u+glyph_w, v+glyph_h,
        color, font.atlas);
};

internal void
r_text(R_Texture output, R_Font font, s32 x, s32 y, u32 color, String8 text) {
  s32 dst_x = x;
  s32 dst_y = y;
  for (u32 i = 0; i < text.size; i += 1) {
    r_char(output, font, dst_x, dst_y, color, text.str[i]);
    dst_x += font.glyph_w;
  }
}

////////////////////////////////
// NOTE: Window Blitting

internal void
r_window_blit(WM_Window window, R_Texture texture, R_Scaling_Mode mode) {
  BITMAPINFO bitmap_info = {0};
  bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
  bitmap_info.bmiHeader.biWidth = texture.w;
  bitmap_info.bmiHeader.biHeight = -texture.h;
  bitmap_info.bmiHeader.biPlanes = 1;
  bitmap_info.bmiHeader.biBitCount = 32;
  bitmap_info.bmiHeader.biCompression = BI_RGB;

  Vector2 window_size = wm_window_size(window);
  s32 wnd_w = (s32)window_size.x;
  s32 wnd_h = (s32)window_size.y;

  void *src_data = texture.data;
  s32 src_w = texture.w;
  s32 src_h = texture.h;
  s32 src_x = 0;
  s32 src_y = 0;

  s32 dst_w = wnd_w;
  s32 dst_h = wnd_h;
  s32 dst_x = 0;
  s32 dst_y = 0;

  switch (mode) {
    case R_Scaling_Mode_ASPECT_RATIO: {
      f32 ratio_x = ((f32)wnd_w/(f32)src_w);
      f32 ratio_y = ((f32)wnd_h/(f32)src_h);
      f32 ratio = (ratio_x < ratio_y) ? ratio_x : ratio_y;
      dst_w = (s32)(src_w*ratio);
      dst_h = (s32)(src_h*ratio);
      dst_x = (wnd_w - dst_w)/2;
      dst_y = (wnd_h - dst_h)/2;
    } break;
    case R_Scaling_Mode_FULLSCREEN: {
      // NOTE: do nothing
    } break;
    case R_Scaling_Mode_NO_SCALING: {
      dst_w = src_w;
      dst_h = src_h;
      dst_x = (wnd_w - dst_w)/2;
      dst_y = (wnd_h - dst_h)/2;
    } break;
    case R_Scaling_Mode_INTEGER_SCALING: {
      s32 ratio_x = wnd_w/src_w;
      s32 ratio_y = wnd_h/src_h;
      s32 ratio = (ratio_x < ratio_y) ? ratio_x : ratio_y;
      dst_w = src_w*ratio;
      dst_h = src_h*ratio;
      dst_x = (wnd_w - dst_w)/2;
      dst_y = (wnd_h - dst_h)/2;
    } break;
    default: {
      invalid_path;
    } break;
  }

  StretchDIBits(w32_wm_window_from_handle(window)->hdc,
                dst_x, dst_y, dst_w, dst_h,
                src_x, src_y, src_w, src_h,
                src_data, &bitmap_info,
                DIB_RGB_COLORS, SRCCOPY);
}
