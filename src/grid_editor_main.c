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

#define TILE_SIZE 8

#define SCREEN_W 128
#define SCREEN_H 128

#define SCREEN_NUM_COLS (SCREEN_W/TILE_SIZE)
#define SCREEN_NUM_ROWS (SCREEN_H/TILE_SIZE)

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

typedef struct GE_State GE_State;
struct GE_State {
  Arena *perm_arena;
  Arena *tran_arena;

  SR_Font   font;
  SR_Canvas canvas;
  WM_Window window;

  Vector2 canvas_size;
  Vector2 window_size;
  Range2  output_rect;

  GLuint canvas_tex;
  GLuint shader;
  GLuint all_purpose_vao;
  GLuint scratch_buffer_64kb;

  f32 refresh_rate;
  f32 target_sec_per_frame;

  u64 prev_time;
  u64 curr_time;
  u64 us_per_frame;

  WM_Key curr_key_state[WM_Key_COUNT];
  WM_Key prev_key_state[WM_Key_COUNT];

  Vector2 window_mouse;
  Vector2 canvas_mouse;

  f32 cam_speed;
  Vector2 cam_pos;
  Vector2 cam_dpos;
};

#define GRID_DIM 8
global u32 grid_data[GRID_DIM*GRID_DIM];
global GE_State *ge_state = 0;

internal b32
key_down(WM_Key key) {
  b32 result = ge_state->curr_key_state[key];
  return(result);
}

internal b32
key_pressed(WM_Key key) {
  b32 result = ge_state->curr_key_state[key] && !ge_state->prev_key_state[key];
  return(result);
}

internal b32
key_release(WM_Key key) {
  b32 result = !ge_state->curr_key_state[key] && ge_state->prev_key_state[key];
  return(result);
}

internal Range2
range2_center(Vector2 c, Vector2 d) {
  Range2 result = {c.x-d.x/2, c.y-d.y/2, c.x+d.x/2, c.y+d.y/2};
  return(result);
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
        ge_state->curr_key_state[event->key] = (event->type == WM_Event_Type_PRESS);
      } break;
      case WM_Event_Type_MOUSE_MOVE: {
        ge_state->window_mouse = event->position;
      } break;
    }
  }

  // NOTE: find output rect
  {
    Vector2 window_size = wm_window_get_size(ge_state->window);

    f32 wnd_w = window_size.x;
    f32 wnd_h = window_size.y;

    f32 src_w = ge_state->canvas.w;
    f32 src_h = ge_state->canvas.h;

    f32 ratio_x = floor_f32(wnd_w/src_w);
    f32 ratio_y = floor_f32(wnd_h/src_h);
    f32 ratio = (ratio_x < ratio_y) ? ratio_x : ratio_y;

    f32 dst_w = src_w*ratio;
    f32 dst_h = src_h*ratio;
    f32 dst_x = (wnd_w - dst_w)/2;
    f32 dst_y = (wnd_h - dst_h)/2;

    ge_state->window_size = window_size;
    ge_state->output_rect = range2_make(dst_x, dst_y, dst_x+dst_w, dst_y+dst_h);
  }

  // NOTE: transform mouse window space to canvas space
  {
    Range2  output = ge_state->output_rect;
    Vector2 output_size = range2_dim(output);
    Vector2 canvas_size = ge_state->canvas_size;

    Vector2 mouse  = ge_state->window_mouse;
    mouse = vector2_sub(mouse, output.min);
    mouse = vector2_div(mouse, output_size);
    mouse = vector2_mul(mouse, canvas_size);

    if (mouse.x < 0.0f) mouse.x = 0.0f;
    if (mouse.y < 0.0f) mouse.y = 0.0f;
    if (mouse.x > canvas_size.x) mouse.x = canvas_size.x;
    if (mouse.y > canvas_size.y) mouse.y = canvas_size.y;

    ge_state->canvas_mouse = mouse;
  }

  // NOTE: update
  {
    f32 dt = ge_state->target_sec_per_frame;
    Vector2 cam_dpos = {0};
    if (key_down(WM_Key_UP))    cam_dpos.y =-1;
    if (key_down(WM_Key_DOWN))  cam_dpos.y = 1;
    if (key_down(WM_Key_LEFT))  cam_dpos.x =-1;
    if (key_down(WM_Key_RIGHT)) cam_dpos.x = 1;
    f32 cam_speed = ge_state->cam_speed;
    Vector2 old_cam_pos = ge_state->cam_pos;
    Vector2 new_cam_pos = vector2_add(old_cam_pos, vector2_muls(cam_dpos, cam_speed*dt));
    ge_state->cam_pos = new_cam_pos;
  }

  // NOTE: draw
  {
    SR_Canvas canvas = ge_state->canvas;
    sr_fill(canvas, C_BLACK);

    Vector2 cam_pos = ge_state->cam_pos;
    s32 off_x = -round_t(s32, cam_pos.x);
    s32 off_y = -round_t(s32, cam_pos.y);

    // NOTE: grid
    {
      // NOTE: blank
      for (u32 dy = 0; dy < SCREEN_NUM_ROWS; dy += 1) {
        for (u32 dx = 0; dx < SCREEN_NUM_COLS; dx += 1) {
          s32 x = (dx*TILE_SIZE + off_x) % SCREEN_W;
          s32 y = (dy*TILE_SIZE + off_y) % SCREEN_H;
          sr_pset(canvas, x, y, C_DARK_BLUE);
        }
      }

      // NOTE: inside
      for (u32 dy = 0; dy < GRID_DIM; dy += 1) {
        for (u32 dx = 0; dx < GRID_DIM; dx += 1) {
          s32 x0 = dx*TILE_SIZE + off_x;
          s32 y0 = dy*TILE_SIZE + off_y;
          s32 x1 = x0 + TILE_SIZE;
          s32 y1 = y0 + TILE_SIZE;
          sr_rectl(canvas, x0, y0, x1, y1, C_LAVENDER);
        }
      }

      // NOTE: border
      {
        s32 x0 = off_x;
        s32 y0 = off_y;
        s32 x1 = x0 + GRID_DIM*TILE_SIZE;
        s32 y1 = y0 + GRID_DIM*TILE_SIZE;
        sr_rectl(canvas, x0, y0, x1, y1, C_LIGHT_GREY);
      }

      // NOTE: offset line
      {
        s32 x0 = 0;
        s32 y0 = 0;
        s32 x1 = off_x;
        s32 y1 = off_y;
        sr_line(canvas, x0, y0, x1, y1, C_DARK_PURPLE);
      }

      // NOTE: selected cell
      {
        Vector2 p = ge_state->canvas_mouse;
        s32 mx = floor_f32(p.x);
        s32 my = floor_f32(p.y);
        s32 rx = (off_x % TILE_SIZE) - TILE_SIZE;
        s32 ry = (off_y % TILE_SIZE) - TILE_SIZE;
        s32 cx = (mx - rx)/TILE_SIZE;
        s32 cy = (my - ry)/TILE_SIZE;
        s32 x0 = cx*TILE_SIZE + rx;
        s32 y0 = cy*TILE_SIZE + ry;
        s32 x1 = x0 + TILE_SIZE;
        s32 y1 = y0 + TILE_SIZE;
        sr_rectl(canvas, x0, y0, x1, y1, C_BLUE);
      }
    }

    // NOTE: info
    {
      SR_Font font = ge_state->font;

      {
        Vector2 p = ge_state->canvas_mouse;
        s32 mx = floor_f32(p.x);
        s32 my = floor_f32(p.y);
        s32 rx = (off_x % TILE_SIZE) - TILE_SIZE;
        s32 ry = (off_y % TILE_SIZE) - TILE_SIZE;
        s32 cx = (mx - rx)/TILE_SIZE;
        s32 cy = (my - ry)/TILE_SIZE;
        String8 text0 = str8f(scratch.arena, "mouse %3i %3i", mx, my);
        String8 text1 = str8f(scratch.arena, " grid %3i %3i", cx, cy);
        s32 x = 0, y = 0;
        sr_text(canvas, x, y, text0, font, C_WHITE); y += font.glyph_h;
        sr_text(canvas, x, y, text1, font, C_WHITE);
      }

      // NOTE: time
      {
        Temp scratch = scratch_begin(0, 0);
        f32 mspf = ge_state->us_per_frame/thousand(1.0f);
        f32 fps = 1.0f/(ge_state->us_per_frame/million(1.0f));
        String8 text = str8f(scratch.arena, "%.2ffps\n%.2fms", fps, mspf);
        Vector2 size = sr_font_text_size(font, text);
        s32 x = canvas.w - size.x - 1, y = 0;
        sr_text(canvas, x, y, text, font, C_LAVENDER);
        scratch_end(scratch);
      }
    }
  }

  // NOTE: flip
  {
    Vector2 window_size = ge_state->window_size;
    Range2  output_rect = ge_state->output_rect;
    Vector2 canvas_size = vector2_make(ge_state->canvas.w, ge_state->canvas.h);

    f32 vertices[] = {
      output_rect.min.x, output_rect.min.y, output_rect.max.x, output_rect.max.y,
      0.0f, 0.0f, canvas_size.x, canvas_size.y,
    };

    r_ogl_os_window_select(ge_state->window);

    glViewport(0, 0, (GLsizei)window_size.x, (GLsizei)window_size.y);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ge_state->shader);
    glBindVertexArray(ge_state->all_purpose_vao);

    glBindBuffer(GL_ARRAY_BUFFER, ge_state->scratch_buffer_64kb);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(f32), (void *)(4*sizeof(f32)));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ge_state->canvas_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, canvas_size.x, canvas_size.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, ge_state->canvas.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glUniform2f(glGetUniformLocation(ge_state->shader, "u_viewport_size_px"), window_size.x, window_size.y);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);

    r_ogl_os_window_swap(ge_state->window);
  }

  // NOTE: reset frame state
  {
    memory_copy_array(ge_state->prev_key_state, ge_state->curr_key_state);
    arena_clear(ge_state->tran_arena);
  }

  // NOTE: sleep
  {
    u64 curr_time = get_time_us();
    f32 sec_per_frame = (curr_time - ge_state->prev_time)/million(1.0f);
    if (sec_per_frame < ge_state->target_sec_per_frame) {
      u32 ms_to_sleep = (u32)((ge_state->target_sec_per_frame - sec_per_frame)*thousand(1.0f));
      if (ms_to_sleep > 0) {
        sleep_ms(ms_to_sleep);
      }
      while (sec_per_frame < ge_state->target_sec_per_frame) {
        sec_per_frame = (get_time_us() - ge_state->prev_time)/million(1.0f);
      }
    }
  }

  ge_state->curr_time = get_time_us();
  ge_state->us_per_frame = ge_state->curr_time - ge_state->prev_time;
  ge_state->prev_time = ge_state->curr_time;

  scratch_end(scratch);
  return(should_quit);
}

internal void
entry_point(int argc, char **argv) {
#if OS_WINDOWS
  timeBeginPeriod(1);
#endif

  // NOTE: init misery state
  {
    Arena *arena = arena_alloc();
    ge_state = push_array(arena, GE_State, 1);
    ge_state->perm_arena = arena;
    ge_state->tran_arena = arena_alloc();
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
          stages[idx].errors.str = push_array(ge_state->perm_arena, u8, info_log_length+1);
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
      ge_state->shader = program;
    }

    glGenVertexArrays(1, &ge_state->all_purpose_vao);

    glGenBuffers(1, &ge_state->scratch_buffer_64kb);
    glBindBuffer(GL_ARRAY_BUFFER, ge_state->scratch_buffer_64kb);
    glBufferData(GL_ARRAY_BUFFER, KB(64), 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  // NOTE: find data path
  {
    String8 exec_path = get_process_info()->exec_path;
    String8 data_path = str8_cat(ge_state->perm_arena, exec_path, s("/../data/"));
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
    IMG_Data img = img_data_from_file_path(ge_state->perm_arena, s("pico8_font.bmp"));
    SR_Canvas atlas = sr_canvas_make(img.data, img.w, img.h, img.w);
    String8 chars = str8_copy(ge_state->perm_arena, s(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "));
    ge_state->font = sr_font_make(ge_state->perm_arena, atlas, 4, 6, &chars);
  }

  // NOTE: make canvas
  {
    Vector2 canvas_size = vector2_make(SCREEN_W, SCREEN_H);
    ge_state->canvas = sr_canvas_alloc(ge_state->perm_arena, canvas_size.x, canvas_size.y);
    ge_state->canvas_size = canvas_size;
    glGenTextures(1, &ge_state->canvas_tex);
  }

  // NOTE: open window
  {
    String8 window_name = s("misery");
    Vector2 window_size = vector2_make(800, 600);
    ge_state->window = wm_window_open(window_name, window_size);
    r_ogl_os_window_equip(ge_state->window);
  }

  {
    ge_state->cam_speed = 32.0f;
    ge_state->cam_pos = vector2_zero();
    ge_state->cam_dpos = vector2_zero();
  }

  ge_state->refresh_rate = 60.0f;
  ge_state->target_sec_per_frame = 1.0f/ge_state->refresh_rate;
  ge_state->prev_time = get_time_us();
  wm_window_first_paint(ge_state->window);

  for (;!frame(););

#if OS_WINDOWS
  timeEndPeriod(1);
#endif
}
