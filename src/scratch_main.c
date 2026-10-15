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
};

global M_State *m_state = 0;

internal void
r_init(void) {
  // NOTE: do os-specific portion of work
  r_ogl_os_init();

  // NOTE: load gl procedures
#define X(name, r, p) name = (name##_Proc_Type *)r_ogl_os_load_proc(#name);
  R_OGL_PROC_XLIST;
#undef X

  // NOTE: build all shaders
  {
    // NOTE: compile
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

    // NOTE: attach compilations to program
    GLuint program = glCreateProgram();
    for_each_element(idx, stages) {
      glAttachShader(program, stages[idx].out);
    }

    // NOTE: link/validate/store
    glLinkProgram(program);
    glValidateProgram(program);
    m_state->shader = program;
  }

  // NOTE: setup resources
  {
    // NOTE: all-purpose VAO
    glGenVertexArrays(1, &m_state->all_purpose_vao);

    // NOTE: scratch buffer
    {
      glGenBuffers(1, &m_state->scratch_buffer_64kb);
      glBindBuffer(GL_ARRAY_BUFFER, m_state->scratch_buffer_64kb);
      glBufferData(GL_ARRAY_BUFFER, KB(64), 0, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }
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

  glViewport(0, 0, wnd_w, wnd_h);
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

global u32 bayer2[2*2] = {
  0, 2,
  3, 1,
};

global u32 bayer4[4*4] = {
   0,  8,  2, 10,
  12,  4, 14,  6,
   3, 11,  1,  9,
  15,  7, 13,  5,
};

global u32 bayer8[8*8] = {
   0, 32,  8, 40,  2, 34, 10, 42,
  48, 16, 56, 24, 50, 18, 58, 26,
  12, 44,  4, 36, 14, 46,  6, 38,
  60, 28, 52, 20, 62, 30, 54, 22,
   3, 35, 11, 43,  1, 33,  9, 41,
  51, 19, 59, 27, 49, 17, 57, 25,
  15, 47,  7, 39, 13, 45,  5, 37,
  63, 31, 55, 23, 61, 29, 53, 21,
};

internal f32
get_bayer2(s32 x, s32 y) {
  f32 result = ((bayer2[(y%2)*2 + x%2])*(1.0f/4.0f) - 0.5f);
  return(result);
}

internal f32
get_bayer4(s32 x, s32 y) {
  f32 result = ((bayer4[(y%4)*4 + x%4])*(1.0f/16.0f) - 0.5f);
  return(result);
}

internal f32
get_bayer8(s32 x, s32 y) {
  f32 result = ((bayer8[(y%8)*8 + x%8])*(1.0f/64.0f) - 0.5f);
  return(result);
}

internal void
dithering_quantization_value_mapping_example(SR_Canvas canvas) {
  u32 color_map[] = {
    0x06060e,
    0x361c7d,
    0xd34f0d,
    0xf99e90,
    // 0x000100,
    // 0x431723,
    // 0x46464f,
    // 0x928e80,
  };

  u32 num_color_map = array_count(color_map);

  // NOTE: fill with gradient
  for (s32 y = 0; y < canvas.h; y += 1) {
    for (s32 x = 0; x < canvas.w; x += 1) {
      u32 color = ((y%256) << 16) | ((x%256) << 8);
      canvas.data[y*canvas.p + x] = color; 
    }
  }

  // NOTE: apply dithering, quantization and value mappping
  for (s32 y = 0; y < canvas.h; y += 1) {
    for (s32 x = 0; x < canvas.w; x += 1) {
      u32 pixel = canvas.data[y*canvas.p + x];
      Vector4 color = rgba4_from_bgra32(pixel);

      // NOTE: bayer dithering
      f32 s = 0.1f;
      f32 m = get_bayer2(x, y);
      color.xyz = vector3_adds(color.xyz, s*m);

      // NOTE: color quantization
      u32 color_per_channel = 4;
      u32 n = (color_per_channel - 1);
      for (u64 idx = 0; idx < 3; idx += 1) {
        color.v[idx] = floor_f32(color.v[idx]*n + 0.5f)/n;
      }

      // NOTE: value mapping
      f32 t = (color.x+color.y+color.z)/3.0f;
      u32 color_map_idx = round_t(u32, t*num_color_map);
      canvas.data[y*canvas.p + x] = color_map[color_map_idx];

      // canvas.data[y*canvas.p + x] = bgra32_from_rgba4(color);
    }
  }
}

internal void
wheel_example(SR_Canvas canvas) {
  sr_fill(canvas, C_BLACK);

  s32 off_x = canvas.w/2;
  s32 off_y = canvas.h/2;
  s32 dim = 2;

  local f32 off_theta = 0.0f;
  off_x = off_x + cos_f32(off_theta)*16.0f;
  off_y = off_y + sin_f32(off_theta)*16.0f;

  f32 radius = 96.0f;
  u32 points_per_turn = 16;
  f32 turn_step = 1.0f/points_per_turn;
  f32 turn = 0.0f;
  for (u32 idx = 0; idx < points_per_turn; idx += 1) {
    f32 theta = rad_from_turn_f32(turn) + off_theta;
    s32 err0 = cos_f32(off_theta*2.0f)*32.0f;
    s32 err1 = sin_f32(off_theta*2.0f)*32.0f;
    s32 err = (idx%2 == 0) ? err0 : err1;
    s32 x = off_x + round_t(s32, cos_f32(theta)*(radius - err));
    s32 y = off_y + round_t(s32, sin_f32(theta)*(radius - err));
    // s32 x = off_x + round_t(s32, cos_f32(theta)*sin_f32(off_theta)*radius);
    // s32 y = off_y + round_t(s32, sin_f32(theta)*cos_f32(off_theta)*radius);
    sr_line(canvas, off_x, off_y, x, y, C_WHITE);
    sr_rect(canvas, x-dim-1, y-dim-1, x+dim+1, y+dim+1, C_DARK_BLUE);
    sr_circ(canvas, x, y, dim, C_RED);
    turn += turn_step;
  }

  sr_circ(canvas, off_x, off_y, dim*3, C_DARK_BLUE);
  sr_circl(canvas, off_x, off_y, dim*2, C_DARK_PURPLE);
  sr_circ(canvas, off_x, off_y, 1, C_RED);

  off_theta += 0.01f;
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
    }
  }

  SR_Canvas canvas = m_state->canvas;
  // dithering_quantization_value_mapping_example(canvas);
  wheel_example(canvas);
  r_end();

  scratch_end(scratch);
  return(should_quit);
}

internal void
entry_point(int argc, char **argv) {
  // NOTE: init state
  {
    Arena *arena = arena_alloc();
    m_state = push_array(arena, M_State, 1);
    m_state->arena = arena;
  }

  // NOTE: init layers
  {
    wm_init();
    r_init();
  }

  // NOTE: canvas & texture
  s32 canvas_w = 256;
  s32 canvas_h = 256;
  u32 *canvas_data = push_array(m_state->arena, u32, canvas_w*canvas_h);
  m_state->canvas = sr_canvas_make(canvas_data, canvas_w, canvas_h, canvas_w);
  glGenTextures(1, &m_state->canvas_tex);

  // NOTE: open window
  String8 window_name = s("misery");
  Vector2 window_size = vector2_make(800, 600);
  m_state->window = wm_window_open(window_name, window_size);
  r_ogl_os_window_equip(m_state->window);

  // NOTE: do first paint
  wm_window_first_paint(m_state->window);
  for (;!frame(););
}
