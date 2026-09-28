#define WM_INIT_MANUAL

#include "base/base.h"
#include "wm/wm.h"
#include "r/ogl/r_ogl.h"

#include "base/base.c"
#include "wm/wm.c"
#include "r/ogl/r_ogl.c"

global String8 r_ogl_vshad_src = str8_lit_comp(
  "#version 330 core\n"
  "\n"
  "layout (location = 0) in vec3 pos;\n"
  "\n"
  "void main(void) {\n"
  "  gl_Position = vec4(pos.x, pos.y, pos.z, 1.0f);"
  "}"
);

global String8 r_ogl_fshad_src = str8_lit_comp(
  "#version 330 core\n"
  "\n"
  "out vec4 final_color;"
  "\n"
  "void main(void) {\n"
  "  final_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
  "}"
);

typedef struct R_OGL_State R_OGL_State;
struct R_OGL_State {
  Arena *arena;
  GLuint program;
  GLuint all_purpose_vao;
  GLuint vbo;
};

typedef struct M_State M_State;
struct M_State {
  Arena *arena;
  WM_Window window;
};

global R_OGL_State *r_ogl_state = 0;
global M_State *m_state = 0;

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

  Vector2 window_size = wm_window_get_size(m_state->window);
  r_ogl_window_select(m_state->window);

  glViewport(0, 0, (GLsizei)window_size.x, (GLsizei)window_size.y);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  {
    glUseProgram(r_ogl_state->program);
    glBindVertexArray(r_ogl_state->all_purpose_vao);
    {
      glBindBuffer(GL_ARRAY_BUFFER, r_ogl_state->vbo);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(f32), (void *)0);
      glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    glBindVertexArray(0);
    glUseProgram(0);
  }

  r_ogl_window_swap(m_state->window);

  scratch_end(scratch);
  return(should_quit);
}

internal void
r_init(void) {
  // NOTE: do os-specific portion of work
  r_ogl_init();

  // NOTE: top-level initialization
  Arena *arena = arena_alloc();
  r_ogl_state = push_array(arena, R_OGL_State, 1);
  r_ogl_state->arena = arena;

  // NOTE: load gl procedures
#define X(name, r, p) name = (name##_Proc_Type *)r_ogl_load_proc(#name);
  R_OGL_PROC_XLIST;
#undef X

  // NOTE: build shaders
  {
    struct {
      GLenum type;
      String8 *src;
      GLuint out;
      String8 errors;
    } stages[] = {
      {GL_VERTEX_SHADER,   &r_ogl_vshad_src},
      {GL_FRAGMENT_SHADER, &r_ogl_fshad_src},
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

    r_ogl_state->program = glCreateProgram();
    for_each_element(idx, stages) {
      glAttachShader(r_ogl_state->program, stages[idx].out);
    }

    glLinkProgram(r_ogl_state->program);
    glValidateProgram(r_ogl_state->program);
  }

  glGenVertexArrays(1, &r_ogl_state->all_purpose_vao);
}

internal void
entry_point(int argc, char **argv) {
  wm_init();
  r_init();

  {
    float vertices[] = {
      -0.5f,-0.5f, 0.0f,
       0.5f,-0.5f, 0.0f,
       0.0f, 0.5f, 0.0f,
    };

    glGenBuffers(1, &r_ogl_state->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r_ogl_state->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  Arena *arena = arena_alloc();
  m_state = push_array(arena, M_State, 1);
  m_state->arena = arena;

  String8 window_name = s("misery");
  Vector2 window_size = vec2(800, 600);

  m_state->window = wm_window_open(window_name, window_size);
  r_ogl_window_equip(m_state->window);

  wm_window_first_paint(m_state->window);
  for (;!frame(););
}
