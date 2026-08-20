#define BUILD_CONSOLE_INTERFACE 1

// NOTE: [h]
#include "base/base.h"
#include "window_manager/window_manager.h"
// #include "font_provider/font_provider.h"
#include "render/render.h"

// NOTE: [c]
#include "base/base.c"
#include "window_manager/window_manager.c"
// #include "font_provider/font_provider.c"
#include "render/render.c"

#undef internal
#include <ft2build.h>
#include FT_FREETYPE_H
#define internal static

typedef struct Font_Glyph_Info Font_Glyph_Info;
struct Font_Glyph_Info {
  f32 x0;
  f32 y0;
  f32 x1;
  f32 y1;
};

typedef struct Font Font;
struct Font {
  u32 first_char;
  u32 char_count;
  Font_Glyph_Info *char_infos;
  R_Texture atlas_texture;
};

typedef struct Font_State Font_State;
struct Font_State {
  Arena *arena;
  FT_Library library;
};

global Font_State *font_state = 0;

internal void
font_init(void) {
  Arena *arena = arena_alloc();
  font_state = push_array(arena, Font_State, 1);
  font_state->arena = arena;
  FT_Init_FreeType(&font_state->library);
}

internal Font
font_load_from_path(Arena *arena, String8 path, f32 size, u32 ascii_start, u32 ascii_end) {
  Temp scratch = scratch_begin(0, 0);
  String8 font_data = data_from_file_path(scratch.arena, path);

  Font font = {0};
  font.first_char = ascii_start;
  font.char_count = ascii_end - ascii_start;
  font.char_infos = push_array(arena, Font_Glyph_Info, font.char_count);

  // NOTE: create face
  FT_Face face = {0};
  FT_New_Memory_Face(font_state->library, font_data.str, (FT_Long)font_data.size, 0, &face);

  // NOTE: unpack font
  FT_Set_Pixel_Sizes(face, 0, (FT_UInt)((96.0f/72.0f)*size));
  s64 ascent = face->size->metrics.ascender >> 6;
  s64 height = face->size->metrics.height >> 6;

  // NOTE: measure & allocate atlas
  s32 total_width = 0;
  for (u64 ascii_idx = ascii_start; ascii_idx <= ascii_end; ascii_idx += 1) {
    FT_Load_Char(face, (FT_ULong)ascii_idx, FT_LOAD_RENDER);
    total_width += (face->glyph->advance.x >> 6);
  }
  s32 dim_x = total_width+1;
  s32 dim_y = (s32)height+1;
  u64 atlas_size = dim_x*dim_y*4;
  u8 *atlas_data = push_array(scratch.arena, u8, atlas_size);

  // NOTE: fill atlas w/ rasterization
  s32 baseline = (s32)ascent;
  s32 pen_x = 0;
  s32 pen_y = 0;
  for (u64 ascii_idx = ascii_start; ascii_idx <= ascii_end; ascii_idx += 1) {
    FT_Load_Char(face, (FT_ULong)ascii_idx, FT_LOAD_RENDER);
    FT_Bitmap *bmp = &face->glyph->bitmap;
    s32 top = face->glyph->bitmap_top;
    s32 left = face->glyph->bitmap_left;

    for (s32 row = 0; row < (s32)bmp->rows; row += 1) {
      s32 y = baseline - top + row;
      for (s32 col = 0; col < (s32)bmp->width; col += 1) {
        s32 x = pen_x + left + col;
        u64 off = (y*dim_x + x)*4;
        if (off+4 <= atlas_size) {
          atlas_data[off+0] = 255;
          atlas_data[off+1] = 255;
          atlas_data[off+2] = 255;
          atlas_data[off+3] = bmp->buffer[row*bmp->pitch + col];
        }
      }
    }

    Font_Glyph_Info *char_info = font.char_infos + ascii_idx - ascii_start;
    char_info->x0 = (f32)pen_x;
    char_info->y0 = (f32)pen_y;
    char_info->x1 = (f32)(pen_x+(face->glyph->advance.x >> 6));
    char_info->y1 = (f32)(pen_y+height);

    pen_x += (face->glyph->advance.x >> 6);
  }

  font.atlas_texture = r_texture_alloc(R_Resource_Type_STATIC, vector2_make((f32)dim_x, (f32)dim_y), atlas_data);
  FT_Done_Face(face);

  scratch_end(scratch);
  return(font);
}

typedef struct UI_Entity UI_Entity;
struct UI_Entity {
  Vector2 pos;
  Vector2 size;
  Vector4 color;
};

typedef struct UI_Entity_Node UI_Entity_Node;
struct UI_Entity_Node {
  UI_Entity_Node *next;
  UI_Entity v;
};

typedef struct UI_Entity_List UI_Entity_List;
struct UI_Entity_List {
  UI_Entity_Node *first;
  UI_Entity_Node *last;
  u64 count;
};

typedef struct Misery_State Misery_State;
struct Misery_State {
  Arena *permanent_arena;
  Arena *frame_arena;

  Font font;
  Vector2 title_bar_size;

  WM_Window window;
  R_Window window_equip;

  u64 time_start;
  u64 us_per_frame;
  b32 should_quit;
};

global Misery_State *state = 0;

internal void
r_char(R_Context *ctx, Font *font, Vector2 p, Vector4 color, u8 c) {
  Font_Glyph_Info *char_info = &font->char_infos[c - font->first_char];
  f32 char_w = char_info->x1 - char_info->x0;
  f32 char_h = char_info->y1 - char_info->y0;
  r_img(ctx,
        range2_make(p.x, p.y, p.x+char_w, p.y+char_h),
        range2_make(char_info->x0, char_info->y0, char_info->x1, char_info->y1),
        state->font.atlas_texture,
        color);
}

internal void
r_text(R_Context *ctx, Font *font, Vector2 p, Vector4 color, String8 text) {
  for (u64 idx = 0; idx < text.size; idx += 1) {
    Font_Glyph_Info *char_info = &font->char_infos[text.str[idx] - font->first_char];
    Range2 char_rect = range2_make(char_info->x0, char_info->y0, char_info->x1, char_info->y1);
    Vector2 char_size = range2_dim(char_rect);
    r_img(ctx,
          range2_min_max(p, vector2_add(p, char_size)),
          char_rect,
          state->font.atlas_texture,
          color);
    p.x += char_size.x;
  }
}

////////////////////////////////
// NOTE: Entry Points

internal void
entry_point(int argc, char **argv) {
  font_init();
  
  Arena *arena = arena_alloc();
  state = push_array(arena, Misery_State, 1);
  state->permanent_arena = arena;
  state->frame_arena = arena_alloc();

  String8 exec_path = get_process_info()->exec_path;
  String8 root_path = str8_chop_last_slash(exec_path);
  assert_always(set_current_path(root_path));

  state->font = font_load_from_path(state->permanent_arena, s("data/liberation-mono.ttf"), 16.0f, 32, 126);

  String8 window_name = s("misery");
  Vector2 window_size = vector2_make(800.f, 600.f);

  state->window = wm_window_open(window_name, window_size, WM_Window_Flag_CUSTOM_BORDER);
  state->title_bar_size = vector2_make(window_size.x, 32.0f);
  wm_window_set_custom_title_thickness(state->window, state->title_bar_size.y);
  wm_window_set_custom_edge_thickness(state->window, 0.0f);

  state->window_equip = r_window_equip(state->window);
  wm_window_first_paint(state->window);

  state->time_start = get_time_us();
  for (;!frame(););
}

internal b32
frame(void) {
  Arena *frame_arena = state->frame_arena;

  WM_Event_List events = wm_get_events(frame_arena, 0);
  for_each_node(WM_Event, event, events.first) {
    switch (event->type) {
      case WM_Event_Type_WINDOW_CLOSE: {
        state->should_quit = 1;
      } break;
      case WM_Event_Type_PRESS: {
        if (!event->is_repeat && event->key == WM_Key_F11) {
          b32 is_maximized = wm_window_is_maximized(state->window);
          wm_window_set_maximized(state->window, !is_maximized);
        }
      } break;
    }
  }

  Range2 window_rect = wm_client_rect_from_window(state->window);
  Vector2 window_size = range2_dim(window_rect);

  state->title_bar_size.x = window_size.x;

  UI_Entity_List ui_entities = {0};
  {
    UI_Entity_Node *n = push_array(frame_arena, UI_Entity_Node, 1);
    n->v.size.x = state->title_bar_size.y;
    n->v.size.y = state->title_bar_size.y;
    n->v.pos.x = state->title_bar_size.x - n->v.size.x*3;
    n->v.pos.y = state->title_bar_size.y - n->v.size.y;
    n->v.color = vector4_make(0.2f, 0.2f, 0.8f, 1.0f);
    sll_queue_push(ui_entities.first, ui_entities.last, n);
    ui_entities.count += 1;
  }
  {
    UI_Entity_Node *n = push_array(frame_arena, UI_Entity_Node, 1);
    n->v.size.x = state->title_bar_size.y;
    n->v.size.y = state->title_bar_size.y;
    n->v.pos.x = state->title_bar_size.x - n->v.size.x*2;
    n->v.pos.y = state->title_bar_size.y - n->v.size.y;
    n->v.color = vector4_make(0.2f, 0.8f, 0.2f, 1.0f);
    sll_queue_push(ui_entities.first, ui_entities.last, n);
    ui_entities.count += 1;
  }
  {
    UI_Entity_Node *n = push_array(frame_arena, UI_Entity_Node, 1);
    n->v.size.x = state->title_bar_size.y;
    n->v.size.y = state->title_bar_size.y;
    n->v.pos.x = state->title_bar_size.x - n->v.size.x;
    n->v.pos.y = state->title_bar_size.y - n->v.size.y;
    n->v.color = vector4_make(0.8f, 0.2f, 0.2f, 1.0f);
    sll_queue_push(ui_entities.first, ui_entities.last, n);
    ui_entities.count += 1;
  }

  R_Rect_Batch_Group_List batch_groups = {0};
  R_Context ctx = r_ctx_make(frame_arena, &batch_groups);
  r_begin(state->window, state->window_equip);

  r_rect(&ctx, range2_min_max(vector2_zero(), state->title_bar_size), vector4_make(0.5f, 0.5f, 0.5f, 1.0f));

  for_each_node(UI_Entity_Node, n, ui_entities.first) {
    r_rect(&ctx, range2_min_max(n->v.pos, vector2_add(n->v.pos, n->v.size)), n->v.color);
  }

  {
    String8 time_info = str8f(frame_arena, "%.2ffps %.2fms", 1.0f/(state->us_per_frame/1000000.0f), state->us_per_frame/1000.0f);
    r_text(&ctx, &state->font, vector2_make(0, 0), vector4_make(1.0f, 0.5f, 0.2f, 1.0f), time_info);
  }

  r_flush(state->window, state->window_equip, &batch_groups);
  r_end(state->window, state->window_equip);

  u64 time_end = get_time_us();
  state->us_per_frame = time_end - state->time_start;
  state->time_start = time_end;

  arena_clear(frame_arena);
  return(state->should_quit);
}
