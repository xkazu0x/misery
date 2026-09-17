#ifndef W32_WM_H
#define W32_WM_H

////////////////////////////////
// NOTE: Includes/Libraries

#include <dwmapi.h>
#include <uxtheme.h>
#include <shellscalingapi.h>
#pragma comment(lib, "gdi32")
#pragma comment(lib, "dwmapi")
#pragma comment(lib, "uxtheme")
#ifndef WM_NCUAHDRAWCAPTION
#define WM_NCUAHDRAWCAPTION (0x00AE)
#endif
#ifndef WM_NCUAHDRAWFRAME
#define WM_NCUAHDRAWFRAME (0x00AF)
#endif

////////////////
// NOTE: Windows

typedef struct W32_WM_Title_Bar_Client_Area W32_WM_Title_Bar_Client_Area;
struct W32_WM_Title_Bar_Client_Area {
  W32_WM_Title_Bar_Client_Area *next;
  Range2 rect;
};

typedef struct W32_WM_Window W32_WM_Window;
struct W32_WM_Window {
  W32_WM_Window *next;
  W32_WM_Window *prev;
  HWND hwnd;
  HDC hdc;
  WINDOWPLACEMENT last_placement;
  f32 dpi;
  b32 first_paint_done;
  b32 maximized;
  b32 custom_border;
  f32 custom_border_title_thickness;
  f32 custom_border_edge_thickness;
};

////////////////////////////////
// NOTE: State

typedef struct W32_WM_State W32_WM_State;
struct W32_WM_State {
  Arena *arena;
  u32 gfx_thread_tid;
  HINSTANCE hInstance;
  HCURSOR hCursor;
  WM_System_Info system_info;
  W32_WM_Window *first_window;
  W32_WM_Window *last_window;
  W32_WM_Window *free_window;
  WM_Key key_from_vkey_table[256];
};

////////////////////////////////
// NOTE: Globals

global W32_WM_State *w32_wm_state = 0;
global WM_Event_List w32_wm_event_list = {0};
global Arena *w32_wm_event_arena = 0;
global b32 w32_wm_resizing = 0;
global b32 w32_wm_new_window_custom_border = 0;

////////////////////////////////
// NOTE: Basic Helpers

internal Range2 w32_wm_range2_from_rect(RECT rect);

////////////////////////////////
// NOTE: Windows

internal WM_Window      w32_wm_handle_from_window(W32_WM_Window *window);
internal W32_WM_Window *w32_wm_window_from_handle(WM_Window handle);
internal W32_WM_Window *w32_wm_window_from_hwnd(HWND hwnd);
internal HWND           w32_wm_hwnd_from_window(W32_WM_Window *window);
internal W32_WM_Window *w32_wm_window_alloc(void);
internal void           w32_wm_window_release(W32_WM_Window *window);
internal WM_Event      *w32_wm_push_event(WM_Event_Type type, W32_WM_Window *window);
internal LRESULT        w32_wm_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // W32_WM_H
