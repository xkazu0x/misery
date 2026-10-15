#define WM_INIT_MANUAL

#include "base/base.h"
#include "wm/wm.h"
#include "r/ogl/r_ogl.h"
#include "sr/sr.h"

#include "base/base.c"
#include "wm/wm.c"
#include "r/ogl/r_ogl.c"
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

#define TILE_SIZE 8

#define MAP_NUM_COLS 8
#define MAP_NUM_ROWS 8

#define MAP_W MAP_NUM_COLS*TILE_SIZE
#define MAP_H MAP_NUM_ROWS*TILE_SIZE

#define SCREEN_W 128
#define SCREEN_H 128

global u32 map[MAP_NUM_ROWS*MAP_NUM_COLS] = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 1, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 1, 0, 0, 1, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
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

typedef struct M_State M_State;
struct M_State {
  Arena *arena;
  WM_Window window;
  SR_Canvas canvas;
  GLuint canvas_tex;
  GLuint shader;
  GLuint all_purpose_vao;
  GLuint scratch_buffer_64kb;
  WM_Key curr_key_state[WM_Key_COUNT];
  WM_Key prev_key_state[WM_Key_COUNT];
  f32 x;
  f32 y;
  f32 dim;
  f32 dir_delta;
  f32 dir_speed;
  f32 dir_angle;
  f32 mov_delta;
  f32 mov_speed;
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
get_map(s32 x, s32 y) {
  u32 result = 0;
  if (x >= 0 && x < MAP_NUM_COLS && y >= 0 && y < MAP_NUM_ROWS) {
    result = map[y*MAP_NUM_COLS + x];
  }
  return(result);
}

internal void
draw_2d(SR_Canvas canvas) {
  // NOTE: grid
  {
    s32 x0 = 0;
    s32 x1 = MAP_NUM_COLS*TILE_SIZE - 1;
    s32 y0 = 0;
    s32 y1 = MAP_NUM_ROWS*TILE_SIZE - 1;
    u32 color = C_DARK_BLUE;
    for (s32 dx = 0; dx < MAP_NUM_COLS; dx += 1) {
      sr__line_vert(canvas, dx*TILE_SIZE, y0, y1, color);
    }
    for (s32 dy = 0; dy < MAP_NUM_ROWS; dy += 1) {
      sr__line_horz(canvas, dy*TILE_SIZE, x0, x1, color);
    }
  }

  // NOTE: map
  {
    for (s32 dy = 0; dy < MAP_NUM_ROWS; dy += 1) {
      for (s32 dx = 0; dx < MAP_NUM_COLS; dx += 1) {
        if (get_map(dx, dy) == 1) {
          s32 x = dx*TILE_SIZE;
          s32 y = dy*TILE_SIZE;
          s32 w = TILE_SIZE;
          s32 h = TILE_SIZE;
          sr_rectf_wh(canvas, x, y, w, h, C_DARK_GREEN);
        }
      }
    }
  }

  // NOTE: pos cell
  {
    s32 d = TILE_SIZE;
    s32 x = floor_f32(m_state->x/d);
    s32 y = floor_f32(m_state->y/d);
    sr_rectf_wh(canvas, x*d, y*d, d, d, C_BLUE);
  }

  // NOTE: dir
  {
    s32 len = 8.0f;
    s32 x0 = m_state->x;
    s32 y0 = m_state->y;
    s32 x1 = x0 + cos_f32(m_state->dir_angle)*len;
    s32 y1 = y0 + sin_f32(m_state->dir_angle)*len;
    sr_line(canvas, x0, y0, x1, y1, C_ORANGE);
  }

  // NOTE: pos
  {
    s32 d = m_state->dim/2;
    s32 x = m_state->x;
    s32 y = m_state->y;
    sr_circf(canvas, x, y, d, C_RED);
  }
}

internal void
draw_3d(SR_Canvas canvas) {
  for (s32 y = 0; y < canvas.h; y += 1) {
    for (s32 x = 0; x < canvas.w; x += 1) {
      u32 color = ((y%255) << 8) | ((x%255) << 0);
      canvas.data[y*canvas.pitch + x] = color;
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
    m_state->dir_delta = 0.0f;
    m_state->mov_delta = 0.0f;

    if (key_down(WM_Key_LEFT))  m_state->dir_delta =-1.0f;
    if (key_down(WM_Key_RIGHT)) m_state->dir_delta = 1.0f;

    if (key_down(WM_Key_UP))    m_state->mov_delta = 1.0f;
    if (key_down(WM_Key_DOWN))  m_state->mov_delta =-1.0f;

    m_state->dir_angle += m_state->dir_delta*m_state->dir_speed;
    f32 dir_x = cos_f32(m_state->dir_angle);
    f32 dir_y = sin_f32(m_state->dir_angle);

    f32 mov_step = m_state->mov_delta*m_state->mov_speed;
    f32 new_x = m_state->x + dir_x*mov_step;
    f32 new_y = m_state->y + dir_y*mov_step;

    s32 tile_x = (s32)floor_f32(new_x/TILE_SIZE);
    s32 tile_y = (s32)floor_f32(new_y/TILE_SIZE);
    if (get_map(tile_x, tile_y) != 1) {
      m_state->x = new_x;
      m_state->y = new_y;
    }
  }

  // NOTE: draw
  {
    SR_Canvas canvas = m_state->canvas;
    SR_Canvas canvas2d = sr_canvas_from(canvas, 0, 0, MAP_W, MAP_H);
    SR_Canvas canvas3d = sr_canvas_from(canvas, MAP_W, 0, SCREEN_W, SCREEN_H);

    sr_fill(canvas, C_BLACK);
    draw_2d(canvas2d);
    draw_3d(canvas3d);
  }

  // NOTE: end
  r_end();
  memory_copy_array(m_state->prev_key_state, m_state->curr_key_state);

  scratch_end(scratch);
  return(should_quit);
}


internal void
entry_point(int argc, char **argv) {
  Arena *arena = arena_alloc();
  m_state = push_array(arena, M_State, 1);
  m_state->arena = arena;

  wm_init();
  r_init();

  s32 canvas_w = SCREEN_W+MAP_W;
  s32 canvas_h = SCREEN_H;
  u32 *canvas_data = push_array(arena, u32, canvas_w*canvas_h);
  m_state->canvas = sr_canvas_make(canvas_data, canvas_w, canvas_h, canvas_w);
  glGenTextures(1, &m_state->canvas_tex);

  String8 window_name = s("misery");
  Vector2 window_size = vector2_make(800, 600);
  m_state->window = wm_window_open(window_name, window_size);
  r_ogl_os_window_equip(m_state->window);

  m_state->x = 2.0f*TILE_SIZE;
  m_state->y = 2.0f*TILE_SIZE;
  m_state->dim = 2.0f;
  m_state->dir_delta = 0.0f;
  m_state->dir_speed = rad_from_deg_f32(4.0f);
  m_state->dir_angle = 0.0f;
  m_state->mov_delta = 0.0f;
  m_state->mov_speed = 0.3f;

  wm_window_first_paint(m_state->window);

  for (;!frame(););
}
