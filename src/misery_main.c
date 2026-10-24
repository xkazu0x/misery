#define WM_INIT_MANUAL

#include "base/base.h"
#include "wm/wm.h"
#include "r/ogl/r_ogl.h"
#include "img/img.h"
#include "sr/sr.h"

#include "base/base.c"
#include "wm/wm.c"
#include "r/ogl/r_ogl.c"
#include "img/img.c"
#include "sr/sr.c"

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

#define TILE_SIZE 16

#define MAP_NUM_COLS 8
#define MAP_NUM_ROWS 8

#define MAP_REAL_W MAP_NUM_COLS*TILE_SIZE
#define MAP_REAL_H MAP_NUM_ROWS*TILE_SIZE

#define MAP_MINI_SCALE 1.0f/(TILE_SIZE/8.0f)
#define MAP_MINI_W MAP_REAL_W*MAP_MINI_SCALE
#define MAP_MINI_H MAP_REAL_H*MAP_MINI_SCALE

global u32 world_map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
  {1, 1, 1, 1, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 1, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 1, 0, 0, 1, 0, 1},
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

typedef struct Ray Ray;
struct Ray {
  b32 hit;
  b32 hit_is_vert;
  f32 hit_x;
  f32 hit_y;
  f32 hit_len;
  u32 map_idx;
  f32 angle;
  b32 face_x;
  b32 face_y;
};

typedef struct M_State M_State;
struct M_State {
  Arena *arena;
  Arena *frame_arena;
  SR_Font font;
  WM_Window window;
  SR_Canvas canvas;
  GLuint canvas_tex;
  GLuint shader;
  GLuint all_purpose_vao;
  GLuint scratch_buffer_64kb;
  WM_Key curr_key_state[WM_Key_COUNT];
  WM_Key prev_key_state[WM_Key_COUNT];
  f32 refresh_rate;
  f32 target_sec_per_frame;
  u64 time_start;
  u64 time_end;
  u64 us_per_frame;
  f32 fov_angle;
  u32 ray_count;
  Ray *rays;
  f32 dim;
  f32 pos_x;
  f32 pos_y;
  f32 mov_delta;
  f32 mov_speed;
  f32 dir_delta;
  f32 dir_speed;
  f32 dir_angle;
};

global M_State *m_state = 0;

internal void
r_init(void) {
  r_ogl_os_init();

#define X(name, r, p) name = (name##_Proc_Type *)r_ogl_os_load_proc(#name);
  R_OGL_PROC_XLIST;
#undef X

  {
    struct {
      GLenum type;
      String8 *src;
      GLuint out;
      String8 errors;
    } stages[] = {
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
        Temp scratch = scratch_begin(0, 0);
        stages[idx].errors.size = info_log_length;
        stages[idx].errors.str = push_array(scratch.arena, u8, info_log_length+1);
        glGetShaderInfoLog(stages[idx].out, info_log_length, 0, (char *)stages[idx].errors.str);
        fprintf(stderr, "[ERROR] OGL: %.*s\n", str8_varg(stages[idx].errors));
        scratch_end(scratch);
      }
    }

    GLuint program = glCreateProgram();
    for_each_element(idx, stages) {
      glAttachShader(program, stages[idx].out);
    }

    glLinkProgram(program);
    glValidateProgram(program);
    m_state->shader = program;
  }

  glGenVertexArrays(1, &m_state->all_purpose_vao);

  glGenBuffers(1, &m_state->scratch_buffer_64kb);
  glBindBuffer(GL_ARRAY_BUFFER, m_state->scratch_buffer_64kb);
  glBufferData(GL_ARRAY_BUFFER, KB(64), 0, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

internal void
r_end(void) {
  Vector2 window_size = wm_window_get_size(m_state->window);
  f32 wnd_w = window_size.x;
  f32 wnd_h = window_size.y;

  f32 src_w = m_state->canvas.w;
  f32 src_h = m_state->canvas.h;
  f32 src_x = 0.0f;
  f32 src_y = 0.0f;

  f32 ratio_x = floor_f32(wnd_w/src_w);
  f32 ratio_y = floor_f32(wnd_h/src_h);
  f32 ratio = (ratio_x < ratio_y) ? ratio_x : ratio_y;

  f32 dst_w = src_w*ratio;
  f32 dst_h = src_h*ratio;
  f32 dst_x = (wnd_w - dst_w)/2;
  f32 dst_y = (wnd_h - dst_h)/2;

  f32 vertices[] = {
    dst_x, dst_y, dst_x+dst_w, dst_y+dst_h,
    src_x, src_y, src_w, src_h,
  };

  r_ogl_os_window_select(m_state->window);

  glViewport(0, 0, (GLsizei)wnd_w, (GLsizei)wnd_h);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(m_state->shader);
  glBindVertexArray(m_state->all_purpose_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_state->scratch_buffer_64kb);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(0, 1);
  glVertexAttribDivisor(1, 1);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(4*sizeof(f32)));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_state->canvas_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_state->canvas.w, m_state->canvas.h, 0, GL_BGRA, GL_UNSIGNED_BYTE, m_state->canvas.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glUniform2f(glGetUniformLocation(m_state->shader, "u_viewport_size_px"), wnd_w, wnd_h);

  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);

  r_ogl_os_window_swap(m_state->window);
}

internal b32
key_down(WM_Key key) {
  b32 result = m_state->curr_key_state[key];
  return(result);
}

internal b32
key_pressed(WM_Key key) {
  b32 result = m_state->curr_key_state[key] && !m_state->prev_key_state[key];
  return(result);
}

internal b32
key_release(WM_Key key) {
  b32 result = !m_state->curr_key_state[key] && m_state->prev_key_state[key];
  return(result);
}

internal u32
get_world_map_idx(s32 x, s32 y) {
  u32 result = 0;
  if (x >= 0 && x < MAP_NUM_COLS && y >= 0 && y < MAP_NUM_ROWS) {
    result = world_map[y][x];
  }
  return(result);
}

internal u32
get_world_map_idx_from_pos(f32 x, f32 y) {
  s32 tile_x = (s32)floor_f32(x/TILE_SIZE);
  s32 tile_y = (s32)floor_f32(y/TILE_SIZE);
  u32 result = get_world_map_idx(tile_x, tile_y);
  return(result);
}

internal Ray
ray_cast(f32 ray_x, f32 ray_y, f32 angle) {
  f32 intercept_x, intercept_y;
  f32 step_x, step_y;
  Ray result = {0};

  // NOTE: keep rangle between 0 <-> 2*pi
  f32 ray_angle = mod_f32(angle, tau32);
  if (ray_angle < 0.0f) ray_angle += tau32;

  // NOTE: find ray facing direction
  b32 face_y = (ray_angle > 0.0f      && ray_angle < pi32);
  b32 face_x = (ray_angle > pi32*1.5f || ray_angle < pi32*0.5f);

  //////////////////////////////////
  // NOTE: VERTICAL INTERSECTION

  b32 vert_hit = 0;
  f32 vert_hit_x = 0;
  f32 vert_hit_y = 0;
  u32 vert_map_idx = 0;
  {
    intercept_y = floor_f32(ray_y/TILE_SIZE)*TILE_SIZE;
    intercept_y += (face_y) ? TILE_SIZE : 0;
    intercept_x = ray_x + (intercept_y - ray_y)/tan_f32(ray_angle);

    step_y = TILE_SIZE;
    step_y *= (!face_y) ? -1.0f : 1.0f;

    step_x = TILE_SIZE/tan_f32(ray_angle);
    step_x *= (!face_x && step_x > 0.0f) ? -1.0f : 1.0f;
    step_x *= ( face_x && step_x < 0.0f) ? -1.0f : 1.0f;

    f32 dx = intercept_x;
    f32 dy = intercept_y;

    while (dx >= 0 && dx <= MAP_REAL_W && dy >= 0 && dy <= MAP_REAL_H) {
      f32 x = dx;
      f32 y = dy;
      if (!face_y) y -= 1.0f;
      u32 map_idx = get_world_map_idx_from_pos(x, y);
      if (map_idx == 1) {
        vert_hit = 1;
        vert_hit_x = dx;
        vert_hit_y = dy;
        vert_map_idx = map_idx;
        break;
      } else {
        dx += step_x;
        dy += step_y;
      }
    }
  } // NOTE: VERTICAL INTERSECTION

  ////////////////////////////////////
  // NOTE: HORIZONTAL INTERSECTION

  b32 horz_hit = 0;
  f32 horz_hit_x = 0;
  f32 horz_hit_y = 0;
  u32 horz_map_idx = 0;
  {
    intercept_x = floor_f32(ray_x/TILE_SIZE)*TILE_SIZE;
    intercept_x += (face_x) ? TILE_SIZE : 0;
    intercept_y = ray_y + (intercept_x - ray_x)*tan_f32(ray_angle);

    step_x = TILE_SIZE;
    step_x *= (!face_x) ? -1.0f : 1.0f;

    step_y = TILE_SIZE*tan_f32(ray_angle);
    step_y *= (!face_y && step_y > 0.0f) ? -1.0f : 1.0f;
    step_y *= ( face_y && step_y < 0.0f) ? -1.0f : 1.0f;

    f32 dx = intercept_x;
    f32 dy = intercept_y;

    while (dx >= 0 && dx <= MAP_REAL_W && dy >= 0 && dy <= MAP_REAL_H) {
      f32 x = dx;
      f32 y = dy;
      if (!face_x) x -= 1;
      u32 map_idx = get_world_map_idx_from_pos(x, y);
      if (map_idx == 1) {
        horz_hit = 1;
        horz_hit_x = dx;
        horz_hit_y = dy;
        horz_map_idx = map_idx;
        break;
      } else {
        dx += step_x;
        dy += step_y;
      }
    }
  } // NOTE: HORIZONTAL INTERSECTION

  if (vert_hit || horz_hit) {
    f32 vert_hit_len = max_f32;
    f32 horz_hit_len = max_f32;

    // NOTE: calculate hit distance
    if (vert_hit) {
      vert_hit_len = sqrt_f32(pow2(vert_hit_x - ray_x) + pow2(vert_hit_y - ray_y));
    }
    if (horz_hit) {
      horz_hit_len = sqrt_f32(pow2(horz_hit_x - ray_x) + pow2(horz_hit_y - ray_y));
    }

    // NOTE: fill ray
    result.hit = 1;
    result.hit_is_vert = (vert_hit_len < horz_hit_len);
    if (result.hit_is_vert) {
      result.hit_x = vert_hit_x;
      result.hit_y = vert_hit_y;
      result.hit_len = vert_hit_len;
      result.map_idx = vert_map_idx;
    } else {
      result.hit_x = horz_hit_x;
      result.hit_y = horz_hit_y;
      result.hit_len = horz_hit_len;
      result.map_idx = horz_map_idx;
    }
    result.angle = ray_angle;
    result.face_x = face_x;
    result.face_y = face_y;
  }

  return(result);
}

internal void
draw_2d(SR_Canvas canvas) {
  // NOTE: grid
  {
    s32 x0 = 0;
    s32 y0 = 0;
    s32 x1 = (MAP_NUM_COLS*TILE_SIZE - 1)*MAP_MINI_SCALE;
    s32 y1 = (MAP_NUM_ROWS*TILE_SIZE - 1)*MAP_MINI_SCALE;
    u32 color = C_DARK_BLUE;
    for (s32 dx = 0; dx < MAP_NUM_COLS; dx += 1) {
      sr_linev(canvas, dx*TILE_SIZE*MAP_MINI_SCALE, y0, y1, color);
    }
    for (s32 dy = 0; dy < MAP_NUM_ROWS; dy += 1) {
      sr_lineh(canvas, dy*TILE_SIZE*MAP_MINI_SCALE, x0, x1, color);
    }
  }

  // NOTE: map
  {
    for (s32 dy = 0; dy < MAP_NUM_ROWS; dy += 1) {
      for (s32 dx = 0; dx < MAP_NUM_COLS; dx += 1) {
        if (get_world_map_idx(dx, dy) == 1) {
          s32 x0 = dx*TILE_SIZE*MAP_MINI_SCALE;
          s32 y0 = dy*TILE_SIZE*MAP_MINI_SCALE;
          s32 x1 = x0 + (TILE_SIZE - 1)*MAP_MINI_SCALE;
          s32 y1 = y0 + (TILE_SIZE - 1)*MAP_MINI_SCALE;
          sr_rectf(canvas, x0, y0, x1, y1, C_DARK_GREEN);
        }
      }
    }
  }

  // NOTE: pos cell
  {
    s32 x0 = floor_f32(m_state->pos_x/TILE_SIZE)*TILE_SIZE*MAP_MINI_SCALE;
    s32 y0 = floor_f32(m_state->pos_y/TILE_SIZE)*TILE_SIZE*MAP_MINI_SCALE;
    s32 x1 = x0 + (TILE_SIZE - 1)*MAP_MINI_SCALE;
    s32 y1 = y0 + (TILE_SIZE - 1)*MAP_MINI_SCALE;
    sr_rectf(canvas, x0, y0, x1, y1, C_BLUE);
  }

  // NOTE: raycast
  {
    f32 ray_x = m_state->pos_x;
    f32 ray_y = m_state->pos_y;
    for (u32 idx = 0; idx < m_state->ray_count; idx += 1) {
      Ray ray = m_state->rays[idx];
      if (ray.hit) {
        f32 hit_x = ray.hit_x;
        f32 hit_y = ray.hit_y;
        if (ray.face_y) hit_y -= 1;
        if (ray.face_x) hit_x -= 1;
        s32 x0 = ray_x*MAP_MINI_SCALE;
        s32 y0 = ray_y*MAP_MINI_SCALE;
        s32 x1 = hit_x*MAP_MINI_SCALE;
        s32 y1 = hit_y*MAP_MINI_SCALE;
        sr_line(canvas, x0, y0, x1, y1, C_YELLOW);
      }
    }
  }

  // NOTE: dir
  {
    s32 len = 6;
    s32 x0 = m_state->pos_x*MAP_MINI_SCALE;
    s32 y0 = m_state->pos_y*MAP_MINI_SCALE;
    s32 x1 = x0 + cos_f32(m_state->dir_angle)*len;
    s32 y1 = y0 + sin_f32(m_state->dir_angle)*len;
    sr_line(canvas, x0, y0, x1, y1, C_ORANGE);
  }

  // NOTE: pos
  {
    s32 d = m_state->dim;
    s32 x = m_state->pos_x*MAP_MINI_SCALE;
    s32 y = m_state->pos_y*MAP_MINI_SCALE;
    sr_circf(canvas, x, y, d, C_RED);
  }
}

internal void
draw_3d(SR_Canvas canvas) {
  // NOTE: raycast project
  {
    f32 fov_angle = m_state->fov_angle;
    f32 plane_d = SCREEN_W*0.5f/tan_f32(fov_angle*0.5f);
    for (u32 idx = 0; idx < m_state->ray_count; idx += 1) {
      Ray ray = m_state->rays[idx];
      if (ray.hit) {
        f32 len = cos_f32(ray.angle - m_state->dir_angle)*ray.hit_len;
        f32 h = (TILE_SIZE/len)*plane_d;
        f32 y = (SCREEN_H - h)/2;
        u32 color = 0;
        if (ray.hit_is_vert) {
          color = (ray.face_y) ? C_RED : C_YELLOW;
        } else {
          color = (ray.face_x) ? C_GREEN : C_ORANGE;
        }
        sr_linev(canvas, idx, y, y+h-1, color);
      }
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
        m_state->curr_key_state[event->key] = (event->type == WM_Event_Type_PRESS);
      } break;
    }
  }

  // NOTE: update
  {
    f32 dt = m_state->target_sec_per_frame;

    m_state->mov_delta = 0.0f;
    m_state->dir_delta = 0.0f;

    if (key_down(WM_Key_UP))    m_state->mov_delta = 1.0f;
    if (key_down(WM_Key_DOWN))  m_state->mov_delta =-1.0f;
    if (key_down(WM_Key_LEFT))  m_state->dir_delta =-1.0f;
    if (key_down(WM_Key_RIGHT)) m_state->dir_delta = 1.0f;

    m_state->dir_angle += m_state->dir_delta*m_state->dir_speed*dt;

    f32 dir_x = cos_f32(m_state->dir_angle);
    f32 dir_y = sin_f32(m_state->dir_angle);

    f32 mov_step = m_state->mov_delta*m_state->mov_speed*dt;

    f32 new_pos_x = m_state->pos_x + dir_x*mov_step;
    f32 new_pos_y = m_state->pos_y + dir_y*mov_step;

    if (get_world_map_idx_from_pos(new_pos_x, new_pos_y) != 1) {
      m_state->pos_x = new_pos_x;
      m_state->pos_y = new_pos_y;
    }

    // NOTE: raycast
    {
      m_state->rays = push_array(m_state->frame_arena, Ray, m_state->ray_count);

      f32 fov_angle = m_state->fov_angle;
      u32 ray_count = m_state->ray_count;

      f32 ray_x = m_state->pos_x;
      f32 ray_y = m_state->pos_y;

      f32 ray_angle = m_state->dir_angle - fov_angle*0.5f;
      f32 ray_angle_step = fov_angle/ray_count;

      for (u32 idx = 0; idx < ray_count; idx += 1, ray_angle += ray_angle_step) {
        m_state->rays[idx] = ray_cast(ray_x, ray_y, ray_angle);
      }
    }
  }

  // NOTE: draw
  {
    SR_Canvas canvas = m_state->canvas;
    SR_Canvas canvas2d = sr_canvas_from(canvas, 0, 0, MAP_MINI_W, MAP_MINI_H);
    SR_Canvas canvas3d = sr_canvas_from(canvas, MAP_MINI_W, 0, SCREEN_W, SCREEN_H);

    sr_fill(canvas, C_BLACK);
    draw_2d(canvas2d);
    draw_3d(canvas3d);

    {
      Temp scratch = scratch_begin(0, 0);
      f32 ms_per_frame = m_state->us_per_frame/thousand(1.0f);
      f32 frames_per_sec = 1.0f/(m_state->us_per_frame/million(1.0f));
      String8 text = str8f(scratch.arena, "%.2ffps\n%.2fms", frames_per_sec, ms_per_frame);
      s32 w, h;
      sr_font_calc_dim(m_state->font, text, &w, &h);
      s32 x = canvas.w - w - 1;
      s32 y = 0;
      sr_rectf(canvas, x, y, x+w, y+h, C_BLACK);
      sr_text(canvas, x, y, text, m_state->font, C_LAVENDER);
      scratch_end(scratch);
    }
  }

  // NOTE: wait to flip
  {
    u64 time_end = get_time_us();
    f32 sec_per_frame = (time_end - m_state->time_start)/million(1.0f);
    if (sec_per_frame < m_state->target_sec_per_frame) {
      u32 ms_to_sleep = (u32)((m_state->target_sec_per_frame - sec_per_frame)*thousand(1.0f));
      if (ms_to_sleep > 0) {
        sleep_ms(ms_to_sleep);
      }
      while (sec_per_frame < m_state->target_sec_per_frame) {
        sec_per_frame = (get_time_us() - m_state->time_start)/million(1.0f);
      }
    }
  }

  m_state->time_end = get_time_us();
  m_state->us_per_frame = m_state->time_end - m_state->time_start;
  m_state->time_start = m_state->time_end;

  r_end();
  memory_copy_array(m_state->prev_key_state, m_state->curr_key_state);
  arena_clear(m_state->frame_arena);

  scratch_end(scratch);
  return(should_quit);
}

internal void
entry_point(int argc, char **argv) {
  Arena *arena = arena_alloc();
  m_state = push_array(arena, M_State, 1);
  m_state->arena = arena;
  m_state->frame_arena = arena_alloc();

  wm_init();
  r_init();

  String8 exec_path = get_process_info()->exec_path;
  String8 data_path = str8_cat(m_state->arena, exec_path, s("/../data/"));
  if (!set_current_path(data_path)) {
    fprintf(stderr, "[FATAL]: failed to set current path to: %.*s\n", str8_varg(data_path));
    abort_self(1);
  }

  IMG_Info img = img_load(m_state->arena, s("pico8_font.bmp"));
  SR_Canvas atlas = sr_canvas_make(img.data, img.w, img.h, img.w);
  String8 chars = str8_copy(m_state->arena, s(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "));
  m_state->font = sr_font_make(m_state->arena, atlas, 4, 6, &chars);

  s32 canvas_w = SCREEN_W + MAP_MINI_W;
  s32 canvas_h = max(SCREEN_H, MAP_MINI_H);
  u32 *canvas_data = push_array(arena, u32, canvas_w*canvas_h);
  m_state->canvas = sr_canvas_make(canvas_data, canvas_w, canvas_h, canvas_w);
  glGenTextures(1, &m_state->canvas_tex);

  String8 window_name = s("misery");
  Vector2 window_size = vector2_make(800, 600);
  m_state->window = wm_window_open(window_name, window_size);
  r_ogl_os_window_equip(m_state->window);

  m_state->fov_angle = rad_from_deg_f32(60.0f);
  m_state->ray_count = SCREEN_W;

  m_state->dim = 2.0f;
  m_state->pos_x = 2.0f*TILE_SIZE;
  m_state->pos_y = 2.0f*TILE_SIZE;
  m_state->dir_delta = 0.0f;
  m_state->dir_speed = rad_from_deg_f32(160.0f);
  m_state->dir_angle = 0.0f;
  m_state->mov_delta = 0.0f;
  m_state->mov_speed = TILE_SIZE*2.0f;

  m_state->refresh_rate = 60.0f;
  m_state->target_sec_per_frame = 1.0f/m_state->refresh_rate;

  m_state->time_start = get_time_us();
  wm_window_first_paint(m_state->window);

  for (;!frame(););
}

// TODO:
// - better collision
