internal SR_Canvas *
sr_canvas_alloc(Arena *arena, s32 width, s32 height, s32 pitch) {
  SR_Canvas *result = push_array_no_zero(arena, SR_Canvas, 1);
  result->width = width;
  result->height = height;
  result->pitch = pitch;
  result->data = push_array_no_zero(arena, u32, result->width*result->height);
  return(result);
}

internal void
sr_fill(SR_Canvas *canvas, u32 color) {
  u32 *dst_row = canvas->data;
  for (s32 y = 0; y < canvas->height; y += 1) {
    u32 *dst_pixel = dst_row;
    for (s32 x = 0; x < canvas->width; x += 1) {
      *dst_pixel++ = color;
    }
    dst_row += canvas->pitch;
  }
}

internal void
sr_rect(SR_Canvas *canvas,
        s32 x0, s32 y0,
        s32 x1, s32 y1,
        u32 color) {
  if (x0 > x1) swap_t(s32, x0, x1);
  if (y0 > y1) swap_t(s32, y0, y1);
  s32 min_x = x0;
  s32 min_y = y0;
  s32 max_x = x1;
  s32 max_y = y1;
  if (min_x < 0) min_x = 0;
  if (min_y < 0) min_y = 0;
  if (max_x > canvas->width)  max_x = canvas->width;
  if (max_y > canvas->height) max_y = canvas->height;
  u32 *dst_row = canvas->data + (min_y*canvas->pitch + min_x);
  for (s32 y = min_y; y < max_y; y += 1) {
    u32 *dst_pixel = dst_row;
    for (s32 x = min_x; x < max_x; x += 1) {
      *dst_pixel++ = color;
    }
    dst_row += canvas->pitch;
  }
}
