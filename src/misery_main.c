#define WM_INIT_MANUAL

#include "base/base.h"
#include "wm/wm.h"
#include "r/ogl/r_ogl.h"
#include "sr/sr.h"

#include "base/base.c"
#include "wm/wm.c"
#include "r/ogl/r_ogl.c"
#include "sr/sr.c"

typedef struct M_State M_State;
struct M_State {
  Arena *arena;
  WM_Window window;
  SR_Canvas *canvas;
  GLuint canvas_tex;
  GLuint shader;
  GLuint all_purpose_vao;
  GLuint scratch_buffer_64kb;
};

global M_State *m_state = 0;

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

  SR_Canvas *canvas = m_state->canvas;
  sr_fill(canvas, 0x101010);
  sr_rect(canvas, 32, 32, 64, 64, 0xaa2020);

  Vector2 window_size = wm_window_get_size(m_state->window);
  f32 wnd_w = window_size.x;
  f32 wnd_h = window_size.y;

  f32 src_w = (f32)canvas->width;
  f32 src_h = (f32)canvas->height;
  f32 src_x = 0.0f;
  f32 src_y = 0.0f;

  f32 ratio_x = wnd_w/src_w;
  f32 ratio_y = wnd_h/src_h;
  f32 ratio = (ratio_x < ratio_y) ? ratio_x : ratio_y;

  f32 dst_w = src_w*ratio;
  f32 dst_h = src_h*ratio;
  f32 dst_x = (wnd_w - dst_w)/2;
  f32 dst_y = (wnd_h - dst_h)/2;

  f32 vertices[] = {
    dst_x, dst_y, dst_x+dst_w, dst_y+dst_h,
    src_x, src_y, src_w, src_h,
  };

  r_ogl_window_select(m_state->window);

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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, canvas->width, canvas->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, canvas->data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glUniform2f(glGetUniformLocation(m_state->shader, "u_viewport_size_px"), wnd_w, wnd_h);

  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);

  r_ogl_window_swap(m_state->window);

  scratch_end(scratch);
  return(should_quit);
}

internal void
entry_point(int argc, char **argv) {
  wm_init();
  r_ogl_init();

#define X(name, r, p) name = (name##_Proc_Type *)r_ogl_load_proc(#name);
  R_OGL_PROC_XLIST;
#undef X

  // NOTE: init main state
  {
    Arena *arena = arena_alloc();
    m_state = push_array(arena, M_State, 1);
    m_state->arena = arena;
  }

  // NOTE: build shaders
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

  // NOTE: canvas & texture
  {
    u32 canvas_w = 128;
    u32 canvas_h = 128;
    m_state->canvas = sr_canvas_alloc(m_state->arena, canvas_w, canvas_h, canvas_w);
    glGenTextures(1, &m_state->canvas_tex);
  }

  // NOTE: open window
  {
    String8 window_name = s("misery");
    Vector2 window_size = v2(800, 600);
    m_state->window = wm_window_open(window_name, window_size);
    r_ogl_window_equip(m_state->window);
  }

  // NOTE: do first paint
  wm_window_first_paint(m_state->window);
  for (;!frame(););
}
