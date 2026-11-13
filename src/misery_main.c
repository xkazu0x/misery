#include "base/base.h"
#include "sh/sh.h"
#include "wm/wm.h"
#include "r/ogl/r_ogl.h"
#include "img/img.h"
#include "sr/sr.h"

#include "base/base.c"
#include "sh/sh.c"
#include "wm/wm.c"
#include "r/ogl/r_ogl.c"
#include "img/img.c"
#include "sr/sr.c"

#if OS_WINDOWS
# include <timeapi.h>
# pragma comment(lib, "winmm")
#endif

#define C_BLACK                 0x000000
#define C_DARK_BLUE             0x1D2B53
#define C_DARK_PURPLE           0x7E2553
#define C_DARK_GREEN            0x008751
#define C_BROWN                 0xAB5236
#define C_DARK_GREY             0x5F574F
#define C_LIGHT_GREY            0xC2C3C7
#define C_WHITE                 0xFFF1E8
#define C_RED                   0xFF004D
#define C_ORANGE                0xFFA300
#define C_YELLOW                0xFFEC27
#define C_GREEN                 0x00E436
#define C_BLUE                  0x29ADFF
#define C_LAVENDER              0x83769C
#define C_PINK                  0xFF77A8
#define C_LIGHT_PEACH           0xFFCCAA

#define SCREEN_W 128
#define SCREEN_H 128

#define TILE_SIZE 32

#define MAP_NUM_COLS 8
#define MAP_NUM_ROWS 8

#define MAP_W MAP_NUM_COLS*TILE_SIZE
#define MAP_H MAP_NUM_ROWS*TILE_SIZE

#define MINI_MAP_SCALE 1.0f/(TILE_SIZE/8.0f)
#define MINI_MAP_W MAP_W*MINI_MAP_SCALE
#define MINI_MAP_H MAP_H*MINI_MAP_SCALE

global u32 world_map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
  {1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 2, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 2, 0, 0, 3, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 1, 1, 1, 1},
};

global String8 r_ogl_rect_vshad_src = str8_lit_comp(
  "#version 330 core\n"
  "\n"
  "in vec4 c_dst_rect_px;\n"
  "in vec4 c_src_rect_px;\n"
  "\n"
  "out vec2 v_tex_zo;\n"
  "\n"
  "uniform sampler2D u_tex;\n"
  "uniform vec2 u_viewport_size_px;\n"
  "\n"
  "void main(void) {\n"
  "  // NOTE: constants\n"
  "  vec2 verts_no[] = vec2[](vec2(-1, -1), vec2(-1, +1), vec2(+1, -1), vec2(+1, +1));\n"
  "  \n"
  "  // NOTE: find dst position\n"
  "  vec2 dst_half_size_px = (c_dst_rect_px.zw - c_dst_rect_px.xy)/2;\n"
  "  vec2 dst_center_px    = (c_dst_rect_px.zw + c_dst_rect_px.xy)/2;\n"
  "  vec2 dst_pos_px       = verts_no[gl_VertexID]*dst_half_size_px + dst_center_px;\n"
  "  \n"
  "  // NOTE: find src position\n"
  "  vec2 src_half_size_px = (c_src_rect_px.zw - c_src_rect_px.xy)/2;\n"
  "  vec2 src_center_px    = (c_src_rect_px.zw + c_src_rect_px.xy)/2;\n"
  "  vec2 src_pos_px       = verts_no[gl_VertexID]*src_half_size_px + src_center_px;\n"
  "  \n"
  "  // NOTE: fill outputs\n"
  "  ivec2 u_tex_size_px_i = textureSize(u_tex, 0);\n"
  "  vec2 u_tex_size_px = vec2(float(u_tex_size_px_i.x), float(u_tex_size_px_i.y));\n"
  "  {\n"
  "    gl_Position = vec4(2*dst_pos_px.x/u_viewport_size_px.x - 1,\n"
  "                       2*(1 - dst_pos_px.y/u_viewport_size_px.y) - 1,\n"
  "                       0.0f, 1.0f);\n"
  "    v_tex_zo = src_pos_px/u_tex_size_px;\n"
  "  }\n"
  "}"
);

global String8 r_ogl_rect_fshad_src = str8_lit_comp(
  "#version 330 core\n"
  "\n"
  "in vec2 v_tex_zo;\n"
  "\n"
  "out vec4 f_color;\n"
  "\n"
  "uniform sampler2D u_tex;\n"
  "\n"
  "void main(void) {\n"
  "  f_color = texture(u_tex, v_tex_zo);\n"
  "}"
);

typedef u32 Ray_Flags;
enum {
  Ray_Flag_HIT     = (1 << 0),
  Ray_Flag_IS_VERT = (1 << 1),
};

typedef struct Ray Ray;
struct Ray {
  u32 idx;
  f32 x;
  f32 y;
  f32 len;
  f32 angle;
  Ray_Flags flags;
};

typedef struct Entity Entity;
struct Entity {
  u32 image_idx;
  f32 x;
  f32 y;
};

typedef struct MS_State MS_State;
struct MS_State {
  Arena *perm_arena;
  Arena *tran_arena;

  SR_Font font;
  u32 image_count;
  SR_Canvas *images;
  SR_Canvas canvas;
  WM_Window window;

  Vector2 window_size;
  Range2  canvas_rect;

  GLuint canvas_tex;
  GLuint shader;
  GLuint all_purpose_vao;
  GLuint scratch_buffer_64kb;

  WM_Key curr_key_state[WM_Key_COUNT];
  WM_Key prev_key_state[WM_Key_COUNT];

  // NOTE: time spec
  f32 refresh_rate;
  f32 target_sec_per_frame;

  // NOTE: time info
  u64 prev_time;
  u64 curr_time;
  u64 usec_per_frame;

  // NOTE: entity
  u32 num_entities;
  Entity *entities;

  // NOTE: camera
  f32 dim;
  f32 pos_x;
  f32 pos_y;
  f32 mov_delta;
  f32 mov_speed;
  f32 dir_delta;
  f32 dir_speed;
  f32 dir_angle;
  f32 fov_angle;
  f32 proj_plane_len;

  // NOTE: ray cast
  u32 num_wall_rays;
  Ray *wall_rays;

  u32 num_sprite_rays;
  Ray *sprite_rays;
};

global MS_State *ms_state = 0;

internal b32
key_down(WM_Key key) {
  b32 result = ms_state->curr_key_state[key];
  return(result);
}

internal b32
key_pressed(WM_Key key) {
  b32 result = ms_state->curr_key_state[key] && !ms_state->prev_key_state[key];
  return(result);
}

internal b32
key_release(WM_Key key) {
  b32 result = !ms_state->curr_key_state[key] && ms_state->prev_key_state[key];
  return(result);
}

internal u32
get_map_tile(s32 x, s32 y) {
  u32 result = 0;
  if (x >= 0 && x < MAP_NUM_COLS && y >= 0 && y < MAP_NUM_ROWS) {
    result = world_map[y][x];
  }
  return(result);
}

internal u32
get_map_tile_from_pos(f32 x, f32 y) {
  s32 tile_x = floor_f32(x/TILE_SIZE);
  s32 tile_y = floor_f32(y/TILE_SIZE);
  u32 result = get_map_tile(tile_x, tile_y);
  return(result);
}

internal b32
is_facing_down(f32 angle) {
  b32 result = (angle > 0.0f && angle < pi32);
  return(result);
}

internal b32
is_facing_up(f32 angle) {
  b32 result = !is_facing_down(angle);
  return(result);
}

internal b32
is_facing_right(f32 angle) {
  b32 result = (angle > pi32*1.5f || angle < pi32*0.5f);
  return(result);
}

internal b32
is_facing_left(f32 angle) {
  b32 result = !is_facing_right(angle);
  return(result);
}

internal f32
angle_wrap_2pi(f32 angle) {
  angle = mod_f32(angle, tau32);
  if (angle < 0.0f) angle += tau32;
  return(angle);
}

internal f32
distance_between_points(f32 x0, f32 y0, f32 x1, f32 y1) {
  f32 result = sqrt_f32(pow2(x1 - x0) + pow2(y1 - y0));
  return(result);
}

internal Ray
ray_make(Ray_Flags flags) {
  Ray result = {0};
  result.len = max_f32;
  result.flags = flags;
  return(result);
}

internal Ray
ray_cast__horz(f32 x, f32 y, f32 angle) {
  Ray result = ray_make(0);
  f32 intercept_x, intercept_y;
  f32 step_x, step_y;

  intercept_y = floor_f32(y/TILE_SIZE)*TILE_SIZE;
  intercept_y += (is_facing_down(angle)) ? TILE_SIZE : 0;
  intercept_x = x + (intercept_y - y)/tan_f32(angle);

  step_y = TILE_SIZE;
  step_y *= (is_facing_up(angle)) ? -1.0f : 1.0f;

  step_x = TILE_SIZE/tan_f32(angle);
  step_x *= (is_facing_left(angle)  && step_x > 0.0f) ? -1.0f : 1.0f;
  step_x *= (is_facing_right(angle) && step_x < 0.0f) ? -1.0f : 1.0f;

  f32 dx = intercept_x;
  f32 dy = intercept_y;

  while (dx >= 0 && dx <= MAP_W && dy >= 0 && dy <= MAP_H) {
    f32 xx = dx;
    f32 yy = dy;
    if (is_facing_up(angle)) yy -= 1.0f;
    u32 map_idx = get_map_tile_from_pos(xx, yy);
    if (map_idx != 0) {
      result.idx = map_idx;
      result.x = dx;
      result.y = dy;
      result.len = distance_between_points(x, y, dx, dy);
      result.angle = angle;
      result.flags |= Ray_Flag_HIT;
      break;
    } else {
      dx += step_x;
      dy += step_y;
    }
  }

  return(result);
}

internal Ray
ray_cast__vert(f32 x, f32 y, f32 angle) {
  Ray result = ray_make(Ray_Flag_IS_VERT);
  f32 intercept_x, intercept_y;
  f32 step_x, step_y;

  intercept_x = floor_f32(x/TILE_SIZE)*TILE_SIZE;
  intercept_x += (is_facing_right(angle)) ? TILE_SIZE : 0;
  intercept_y = y + (intercept_x - x)*tan_f32(angle);

  step_x = TILE_SIZE;
  step_x *= (is_facing_left(angle)) ? -1.0f : 1.0f;

  step_y = TILE_SIZE*tan_f32(angle);
  step_y *= (is_facing_up(angle)   && step_y > 0.0f) ? -1.0f : 1.0f;
  step_y *= (is_facing_down(angle) && step_y < 0.0f) ? -1.0f : 1.0f;

  f32 dx = intercept_x;
  f32 dy = intercept_y;

  while (dx >= 0 && dx <= MAP_W && dy >= 0 && dy <= MAP_H) {
    f32 xx = dx;
    f32 yy = dy;
    if (is_facing_left(angle)) xx -= 1;
    u32 map_idx = get_map_tile_from_pos(xx, yy);
    if (map_idx != 0) {
      result.idx = map_idx;
      result.x = dx;
      result.y = dy;
      result.len = distance_between_points(x, y, dx, dy);
      result.angle = angle;
      result.flags |= Ray_Flag_HIT;
      break;
    } else {
      dx += step_x;
      dy += step_y;
    }
  }

  return(result);
}

internal Ray
ray_cast(f32 x, f32 y, f32 angle) {
  angle = angle_wrap_2pi(angle);
  Ray rh = ray_cast__horz(x, y, angle);
  Ray rv = ray_cast__vert(x, y, angle);
  Ray result = (rv.len < rh.len) ? rv : rh;
  return(result);
}

internal void
ray_cast_update(void) {
  f32 cam_x = ms_state->pos_x;
  f32 cam_y = ms_state->pos_y;
  f32 cam_dir_angle = ms_state->dir_angle;
  f32 cam_fov_angle = ms_state->fov_angle;
  f32 cam_half_fov_angle = cam_fov_angle*0.5f;
  f32 cam_proj_plane_len = ms_state->proj_plane_len;

  // NOTE: wall
  ms_state->num_wall_rays = SCREEN_W;
  ms_state->wall_rays = push_array(ms_state->tran_arena, Ray, ms_state->num_wall_rays);
  for (u32 ray_idx = 0; ray_idx < ms_state->num_wall_rays; ray_idx += 1) {
    f32 undistorted_angle = atan_f32((ray_idx - ms_state->num_wall_rays/2.0f)/cam_proj_plane_len) + cam_dir_angle;
    ms_state->wall_rays[ray_idx] = ray_cast(cam_x, cam_y, undistorted_angle);
  }

  // NOTE: sprite
  ms_state->num_sprite_rays = 0;
  ms_state->sprite_rays = push_array(ms_state->tran_arena, Ray, ms_state->num_entities);
  for (u32 idx = 0; idx < ms_state->num_entities; idx += 1) {
    Entity *e = &ms_state->entities[idx];
    f32 dx = e->x - cam_x;
    f32 dy = e->y - cam_y;
    f32 angle = atan2_f32(dy, dx);
    f32 dangle = cam_dir_angle - angle;
    if (dangle >  pi32) dangle -= tau32;
    if (dangle < -pi32) dangle += tau32;
    dangle = abs_f32(dangle);
    if (dangle < (cam_half_fov_angle + 0.3f)) {
      Ray *ray = &ms_state->sprite_rays[ms_state->num_sprite_rays];
      {
        ray->idx = e->image_idx;
        ray->x = e->x;
        ray->y = e->y;
        ray->len = distance_between_points(ray->x, ray->y, cam_x, cam_y);
        ray->angle = dangle;
        ray->flags |= Ray_Flag_HIT;
      }
      ms_state->num_sprite_rays += 1;
    }
    u32 num_unused_rays = ms_state->num_entities - ms_state->num_sprite_rays;
    arena_pop(ms_state->tran_arena, num_unused_rays*sizeof(Ray));
  }

  // NOTE: sprite ray cast sort by length
  for (u32 a_idx = 0; a_idx < ms_state->num_sprite_rays; a_idx += 1) {
    for (u32 b_idx = 0; b_idx < ms_state->num_sprite_rays; b_idx += 1) {
      if (ms_state->sprite_rays[a_idx].len > ms_state->sprite_rays[b_idx].len) {
        Ray tmp = ms_state->sprite_rays[a_idx];
        ms_state->sprite_rays[a_idx] = ms_state->sprite_rays[b_idx];
        ms_state->sprite_rays[b_idx] = tmp;
      }
    }
  }
}

internal void
ray_cast_render(SR_Canvas canvas) {
  f32 cam_x = ms_state->pos_x;
  f32 cam_y = ms_state->pos_y;
  f32 cam_dir_angle = ms_state->dir_angle;
  f32 cam_proj_plane_len = ms_state->proj_plane_len;

  // NOTE: wall projection
  for (u32 x = 0; x < ms_state->num_wall_rays; x += 1) {
    Ray ray = ms_state->wall_rays[x];
    if (ray.flags & Ray_Flag_HIT) {
      f32 perp_len = cos_f32(ray.angle - cam_dir_angle)*ray.len;
      f32 proj_h = (TILE_SIZE/perp_len)*cam_proj_plane_len;

      s32 y0 = (SCREEN_H - proj_h)/2.0f;
      s32 y1 = y0 + proj_h - 1;

      SR_Canvas image = ms_state->images[ray.idx - 1];
      f32 ray_hit_pos = (ray.flags & Ray_Flag_IS_VERT) ? ray.y : ray.x;
      f32 tile_rel_pos = ((s32)ray_hit_pos) % TILE_SIZE;
      s32 u = (tile_rel_pos/TILE_SIZE)*image.w;
      f32 ym = (1.0f/proj_h)*image.h;

      for (s32 y = y0; y <= y1; y += 1) {
        s32 v = (y - y0)*ym;
        u32 color = sr_pget(image, u, v);
        if (ray.flags & Ray_Flag_IS_VERT) {
          color = bgra32_muls_rgb(color, 0.5f);
        }
        sr_pset(canvas, x, y, color);
      }
    }
  }

  // NOTE: sprite projection
  for (u32 idx = 0; idx < ms_state->num_sprite_rays; idx += 1) {
    Ray ray = ms_state->sprite_rays[idx];

    f32 perp_len = cos_f32(ray.angle)*ray.len;
    f32 proj_h = (TILE_SIZE/perp_len)*cam_proj_plane_len;
    f32 proj_w = proj_h;

    f32 dangle = atan2_f32(ray.y - cam_y, ray.x - cam_x) - cam_dir_angle;
    s32 proj_x = tan_f32(dangle)*cam_proj_plane_len;

    SR_Canvas image = ms_state->images[ray.idx];
    s32 y0 = (SCREEN_H - proj_h)/2.0f;
    s32 x0 = (SCREEN_W - proj_w)/2.0f + proj_x;
    s32 x1 = x0 + proj_w;
    s32 y1 = y0 + proj_h;
    f32 xm = (image.w/proj_w);
    f32 ym = (image.h/proj_h);

    for (s32 y = y0; y < y1; y += 1) {
      s32 v = (y - y0)*ym;
      for (s32 x = x0; x < x1; x += 1) {
        s32 u = (x - x0)*xm;
        u32 color = sr_pget(image, u, v);
        if (ray.len < ms_state->wall_rays[x].len && color != 0xffff00ff) {
          sr_pset(canvas, x, y, color);
        }
      }
    }
  }
}

internal void
mini_map_render(SR_Canvas canvas) {
  // NOTE: grid
  {
    s32 x0 = 0;
    s32 y0 = 0;
    s32 x1 = (MAP_NUM_COLS*TILE_SIZE - 1)*MINI_MAP_SCALE;
    s32 y1 = (MAP_NUM_ROWS*TILE_SIZE - 1)*MINI_MAP_SCALE;
    u32 color = C_DARK_GREY;
    for (s32 dx = 0; dx < MAP_NUM_COLS; dx += 1) {
      sr_linev(canvas, dx*TILE_SIZE*MINI_MAP_SCALE, y0, y1, color);
    }
    for (s32 dy = 0; dy < MAP_NUM_ROWS; dy += 1) {
      sr_lineh(canvas, dy*TILE_SIZE*MINI_MAP_SCALE, x0, x1, color);
    }
  }

  // NOTE: map
  {
    for (s32 dy = 0; dy < MAP_NUM_ROWS; dy += 1) {
      for (s32 dx = 0; dx < MAP_NUM_COLS; dx += 1) {
        u32 map_idx = get_map_tile(dx, dy);
        if (map_idx != 0) {
          SR_Canvas image = ms_state->images[map_idx - 1];
          s32 x0 = dx*TILE_SIZE*MINI_MAP_SCALE;
          s32 y0 = dy*TILE_SIZE*MINI_MAP_SCALE;
          s32 x1 = x0 + (TILE_SIZE - 1)*MINI_MAP_SCALE;
          s32 y1 = y0 + (TILE_SIZE - 1)*MINI_MAP_SCALE;
          s32 u0 = 0;
          s32 v0 = 0;
          s32 u1 = image.w - 1;
          s32 v1 = image.h - 1;
          sr_image(canvas, x0, y0, x1, y1, u0, v0, u1, v1, image, ~0u);
        }
      }
    }
  }

  // NOTE: pos cell
  {
    s32 x0 = floor_f32(ms_state->pos_x/TILE_SIZE)*TILE_SIZE*MINI_MAP_SCALE;
    s32 y0 = floor_f32(ms_state->pos_y/TILE_SIZE)*TILE_SIZE*MINI_MAP_SCALE;
    s32 x1 = x0 + (TILE_SIZE - 1)*MINI_MAP_SCALE;
    s32 y1 = y0 + (TILE_SIZE - 1)*MINI_MAP_SCALE;
    sr_rectf(canvas, x0, y0, x1, y1, C_BLUE);
  }

  // NOTE: wall ray cast
  {
    f32 cam_x = ms_state->pos_x;
    f32 cam_y = ms_state->pos_y;
    for (u32 idx = 0; idx < ms_state->num_wall_rays; idx += 1) {
      Ray ray = ms_state->wall_rays[idx];
      if (ray.flags & Ray_Flag_HIT) {
        f32 ray_x = ray.x;
        f32 ray_y = ray.y;
        if (is_facing_right(ray.angle)) ray_x -= 1;
        if (is_facing_down(ray.angle))  ray_y -= 1;
        s32 x0 = cam_x*MINI_MAP_SCALE;
        s32 y0 = cam_y*MINI_MAP_SCALE;
        s32 x1 = ray_x*MINI_MAP_SCALE;
        s32 y1 = ray_y*MINI_MAP_SCALE;
        sr_line(canvas, x0, y0, x1, y1, C_YELLOW);
      }
    }
  }

  // NOTE: dir
  {
    s32 len = 6;
    s32 x0 = ms_state->pos_x*MINI_MAP_SCALE;
    s32 y0 = ms_state->pos_y*MINI_MAP_SCALE;
    s32 x1 = x0 + cos_f32(ms_state->dir_angle)*len;
    s32 y1 = y0 + sin_f32(ms_state->dir_angle)*len;
    sr_line(canvas, x0, y0, x1, y1, C_ORANGE);
  }

  // NOTE: pos
  {
    s32 d = ms_state->dim;
    s32 x = ms_state->pos_x*MINI_MAP_SCALE;
    s32 y = ms_state->pos_y*MINI_MAP_SCALE;
    sr_circf(canvas, x, y, d, C_RED);
  }

  // NOTE: sprite ray cast
  {
    for (u32 idx = 0; idx < ms_state->num_sprite_rays; idx += 1) {
      Ray ray = ms_state->sprite_rays[idx];
      SR_Canvas image = ms_state->images[ray.idx];
      s32 x0 = (ray.x - TILE_SIZE/2)*MINI_MAP_SCALE;
      s32 y0 = (ray.y - TILE_SIZE/2)*MINI_MAP_SCALE;
      s32 x1 = (ray.x + TILE_SIZE/2 - 1)*MINI_MAP_SCALE;
      s32 y1 = (ray.y + TILE_SIZE/2 - 1)*MINI_MAP_SCALE;
      s32 u0 = 0;
      s32 v0 = 0;
      s32 u1 = image.w - 1;
      s32 v1 = image.h - 1;
      sr_image(canvas, x0, y0, x1, y1, u0, v0, u1, v1, image, ~0u);
    }
  }
}

internal b32
frame(void) {
  b32 should_quit = 0;
  Temp scratch = scratch_begin(0, 0);

  WM_Event_List events = wm_get_events(scratch.arena, 0);
  for_each_node(WM_Event, event, events.first) {
    switch (event->type) {
      case WM_Event_Type_WINDOW_CLOSE: {
        should_quit = 1;
      } break;
      case WM_Event_Type_PRESS:
      case WM_Event_Type_RELEASE: {
        ms_state->curr_key_state[event->key] = (event->type == WM_Event_Type_PRESS);
      } break;
    }
  }

  // NOTE: find canvas rect in window space
  {
    Vector2 window_size = wm_window_get_size(ms_state->window);
    if (window_size.x != ms_state->window_size.x ||
        window_size.y != ms_state->window_size.y) {
      f32 wnd_w = window_size.x;
      f32 wnd_h = window_size.y;
      f32 src_w = ms_state->canvas.w;
      f32 src_h = ms_state->canvas.h;
      f32 ratio_x = floor_f32(wnd_w/src_w);
      f32 ratio_y = floor_f32(wnd_h/src_h);
      f32 ratio = (ratio_x < ratio_y) ? ratio_x : ratio_y;
      f32 dst_w = src_w*ratio;
      f32 dst_h = src_h*ratio;
      f32 dst_x = (wnd_w - dst_w)/2.0f;
      f32 dst_y = (wnd_h - dst_h)/2.0f;
      ms_state->window_size = window_size;
      ms_state->canvas_rect = range2_make(dst_x, dst_y, dst_x+dst_w, dst_y+dst_h);
    }
  }

  // NOTE: update
  {
    f32 dt = ms_state->target_sec_per_frame;

    // NOTE: camera movement
    {
      ms_state->mov_delta = 0.0f;
      ms_state->dir_delta = 0.0f;

      if (key_down(WM_Key_UP))    ms_state->mov_delta = 1.0f;
      if (key_down(WM_Key_DOWN))  ms_state->mov_delta =-1.0f;
      if (key_down(WM_Key_LEFT))  ms_state->dir_delta =-1.0f;
      if (key_down(WM_Key_RIGHT)) ms_state->dir_delta = 1.0f;

      ms_state->dir_angle += ms_state->dir_delta*ms_state->dir_speed*dt;
      ms_state->dir_angle = angle_wrap_2pi(ms_state->dir_angle);

      f32 dir_x = cos_f32(ms_state->dir_angle);
      f32 dir_y = sin_f32(ms_state->dir_angle);

      f32 mov_step = ms_state->mov_delta*ms_state->mov_speed*dt;

      f32 new_pos_x = ms_state->pos_x + dir_x*mov_step;
      f32 new_pos_y = ms_state->pos_y + dir_y*mov_step;

      if (get_map_tile_from_pos(new_pos_x, new_pos_y) == 0) {
        ms_state->pos_x = new_pos_x;
        ms_state->pos_y = new_pos_y;
      }
    }

    ray_cast_update();
  }

  // NOTE: draw
  {
    SR_Canvas canvas = ms_state->canvas;
    SR_Canvas clip0 = sr_canvas_clip(canvas, 0, 0, MINI_MAP_W, MINI_MAP_H);
    SR_Canvas clip1 = sr_canvas_clip(canvas, MINI_MAP_W, 0, SCREEN_W, SCREEN_H);

    sr_fill(canvas, C_BLACK);
    mini_map_render(clip0);
    ray_cast_render(clip1);

    // NOTE: time info
    {
      Temp scratch = scratch_begin(0, 0);
      f32 ms_per_frame = ms_state->usec_per_frame/thousand(1.0f);
      f32 frames_per_sec = 1.0f/(ms_state->usec_per_frame/million(1.0f));
      SR_Font font = ms_state->font;
      String8 text = str8f(scratch.arena, "%.2ffps\n%.2fms", frames_per_sec, ms_per_frame);
      Vector2 size = sr_font_text_size(font, text);
      s32 x = canvas.w - size.x - 1;
      s32 y = 0;
      sr_rectf(canvas, x, y, x+size.x, y+size.y, C_BLACK);
      sr_text(canvas, x, y, text, ms_state->font, C_LAVENDER);
      scratch_end(scratch);
    }
  }

  // NOTE: flip
  {
    Vector2 window_size = ms_state->window_size;
    Range2  canvas_rect = ms_state->canvas_rect;
    Vector2 canvas_size = vector2_make(ms_state->canvas.w, ms_state->canvas.h);

    f32 vertices[] = {
      canvas_rect.min.x, canvas_rect.min.y,
      canvas_rect.max.x, canvas_rect.max.y,
      0.0f,              0.0f,
      canvas_size.x,     canvas_size.y,
    };

    r_ogl_os_window_select(ms_state->window);

    glViewport(0, 0, (GLsizei)window_size.x, (GLsizei)window_size.y);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ms_state->shader);
    glBindVertexArray(ms_state->all_purpose_vao);

    glBindBuffer(GL_ARRAY_BUFFER, ms_state->scratch_buffer_64kb);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(4*sizeof(f32)));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ms_state->canvas_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ms_state->canvas.w, ms_state->canvas.h, 0, GL_BGRA, GL_UNSIGNED_BYTE, ms_state->canvas.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glUniform2f(glGetUniformLocation(ms_state->shader, "u_viewport_size_px"), window_size.x, window_size.y);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);

    r_ogl_os_window_swap(ms_state->window);
  }

  // NOTE: sleep
  {
    u64 curr_time = get_time_us();
    f32 sec_per_frame = (curr_time - ms_state->prev_time)/million(1.0f);
    if (sec_per_frame < ms_state->target_sec_per_frame) {
      u32 ms_to_sleep = (u32)((ms_state->target_sec_per_frame - sec_per_frame)*thousand(1.0f));
      if (ms_to_sleep > 0) {
        sleep_ms(ms_to_sleep);
      }
      while (sec_per_frame < ms_state->target_sec_per_frame) {
        sec_per_frame = (get_time_us() - ms_state->prev_time)/million(1.0f);
      }
    }
  }

  ms_state->curr_time = get_time_us();
  ms_state->usec_per_frame = ms_state->curr_time - ms_state->prev_time;
  ms_state->prev_time = ms_state->curr_time;

  memory_copy_array(ms_state->prev_key_state, ms_state->curr_key_state);
  arena_clear(ms_state->tran_arena);
  scratch_end(scratch);

  return(should_quit);
}

internal SR_Canvas
generate_test_image(Arena *arena, s32 w, s32 h, u32 color) {
  SR_Canvas result = sr_canvas_alloc(arena, w, h);
  for (s32 y = 0; y < result.h; y += 1) {
    for (s32 x = 0; x < result.w; x += 1) {
      result.data[y*result.pitch + x] = (((x+y)%2) == 0) ? color : 0xff000000;
    }
  }
  return(result);
}

internal void
entry_point(int argc, char **argv) {
#if OS_WINDOWS
  timeBeginPeriod(1);
#endif

  // NOTE: init global state
  {
    Arena *arena = arena_alloc();
    ms_state = push_array(arena, MS_State, 1);
    ms_state->perm_arena = arena;
    ms_state->tran_arena = arena_alloc();
  }

  // NOTE: init opengl
  {
    r_ogl_os_init();

#define X(name, r, p) name = (name##_Proc_Type *)r_ogl_os_load_proc(#name);
    R_OGL_PROC_XLIST;
#undef X

    // NOTE: build shader
    {
      struct {GLenum type; String8 *src; GLuint out; String8 errors;} stages[] = {
        {GL_VERTEX_SHADER,   &r_ogl_rect_vshad_src},
        {GL_FRAGMENT_SHADER, &r_ogl_rect_fshad_src},
      };

      for_each_element(idx, stages) {
        stages[idx].out = glCreateShader(stages[idx].type);
        GLint src_size = (GLint)stages[idx].src->size;
        glShaderSource(stages[idx].out, 1, (const GLchar **)&stages[idx].src->str, &src_size);
        glCompileShader(stages[idx].out);
        GLint info_log_length = 0;
        GLint status = 0;
        glGetShaderiv(stages[idx].out, GL_COMPILE_STATUS, &status);
        glGetShaderiv(stages[idx].out, GL_INFO_LOG_LENGTH, &info_log_length);
        if (info_log_length != 0) {
          stages[idx].errors.size = info_log_length;
          stages[idx].errors.str = push_array(ms_state->perm_arena, u8, info_log_length+1);
          glGetShaderInfoLog(stages[idx].out, info_log_length, 0, (char *)stages[idx].errors.str);
          sh_message(1, str8_lit("OpenGL: Shader Compilation Error"), stages[idx].errors);
        }
      }

      GLuint program = glCreateProgram();
      for_each_element(idx, stages) {
        glAttachShader(program, stages[idx].out);
      }

      glLinkProgram(program);
      glValidateProgram(program);
      ms_state->shader = program;
    }

    glGenVertexArrays(1, &ms_state->all_purpose_vao);

    glGenBuffers(1, &ms_state->scratch_buffer_64kb);
    glBindBuffer(GL_ARRAY_BUFFER, ms_state->scratch_buffer_64kb);
    glBufferData(GL_ARRAY_BUFFER, KB(64), 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  // NOTE: find data path
  {
    String8 exec_path = get_process_info()->exec_path;
    String8 data_path = str8_cat(ms_state->perm_arena, exec_path, s("/../data/"));
    if (!set_current_path(data_path)) {
      Temp scratch = scratch_begin(0, 0);
      String8 message = str8f(scratch.arena, "Failed to set current path to \"%.*s\".", str8_varg(data_path));
      sh_message(1, str8_lit("Fatal Error"), message);
      abort_self(1);
      scratch_end(scratch);
    }
  }

  // NOTE: make font
  {
    IMG_Data img = img_data_from_file_path(ms_state->perm_arena, s("pico8_font.bmp"));
    SR_Canvas atlas = sr_canvas_make(img.data, img.w, img.h, img.w);
    String8 chars = str8_copy(ms_state->perm_arena, s(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "));
    ms_state->font = sr_font_make(ms_state->perm_arena, atlas, 4, 6, &chars);
  }

  // NOTE: make test images
  {
    s32 w = 4;
    s32 h = 4;
    ms_state->image_count = 5;
    ms_state->images = push_array(ms_state->perm_arena, SR_Canvas, ms_state->image_count);
    ms_state->images[0] = generate_test_image(ms_state->perm_arena, w, h, 0xff0000ff);
    ms_state->images[1] = generate_test_image(ms_state->perm_arena, w, h, 0xff00ff00);
    ms_state->images[2] = generate_test_image(ms_state->perm_arena, w, h, 0xffff0000);
    ms_state->images[3] = generate_test_image(ms_state->perm_arena, w, h, 0xffffff00);
    ms_state->images[4] = generate_test_image(ms_state->perm_arena, w, h, 0xff00ffff);
  }

  // NOTE: make canvas
  {
    s32 canvas_w = SCREEN_W + MINI_MAP_W;
    s32 canvas_h = max(SCREEN_H, MINI_MAP_H);
    ms_state->canvas = sr_canvas_alloc(ms_state->perm_arena, canvas_w, canvas_h);
    glGenTextures(1, &ms_state->canvas_tex);
  }

  // NOTE: open window
  {
    String8 window_name = s("misery");
    Vector2 window_size = vector2_make(800, 600);
    ms_state->window = wm_window_open(window_name, window_size);
    r_ogl_os_window_equip(ms_state->window);
  }

  {
    ms_state->num_entities = 2;
    ms_state->entities = push_array(ms_state->perm_arena, Entity, ms_state->num_entities);
    {
      Entity *e = &ms_state->entities[0];
      e->image_idx = 4;
      e->x = 2*TILE_SIZE + TILE_SIZE/2;
      e->y = 2*TILE_SIZE + TILE_SIZE/2;
    }
    {
      Entity *e = &ms_state->entities[1];
      e->image_idx = 3;
      e->x = 1*TILE_SIZE + TILE_SIZE/2;
      e->y = 1*TILE_SIZE + TILE_SIZE/2;
    }
  }

  ms_state->dim = 2.0f;
  ms_state->pos_x = (MAP_NUM_ROWS/2)*TILE_SIZE;
  ms_state->pos_y = (MAP_NUM_COLS/2)*TILE_SIZE;
  ms_state->dir_delta = 0.0f;
  ms_state->dir_speed = rad_from_deg_f32(160.0f);
  ms_state->dir_angle = 0.0f;
  ms_state->mov_delta = 0.0f;
  ms_state->mov_speed = TILE_SIZE*2.0f;
  ms_state->fov_angle = rad_from_deg_f32(80.0f);
  ms_state->proj_plane_len = 0.5f*SCREEN_W/tan_f32(0.5f*ms_state->fov_angle);

  ms_state->refresh_rate = 60.0f;
  ms_state->target_sec_per_frame = 1.0f/ms_state->refresh_rate;

  ms_state->prev_time = get_time_us();
  wm_window_first_paint(ms_state->window);

  for (;!frame(););

#if OS_WINDOWS
  timeEndPeriod(1);
#endif
}
