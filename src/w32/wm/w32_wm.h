#ifndef W32_WM_H
#define W32_WM_H

////////////////////////////////
// NOTE: Includes/Libraries

#pragma comment(lib, "gdi32")

////////////////////////////////
// NOTE: Windows

typedef struct W32_WM_Window W32_WM_Window;
struct W32_WM_Window {
  W32_WM_Window *next;
  W32_WM_Window *prev;
  HWND hwnd;
  HDC hdc;
  WINDOWPLACEMENT last_placement;
  b32 first_paint_done;
  b32 maximized;
};

////////////////////////////////
// NOTE: State

typedef struct W32_WM_State W32_WM_State;
struct W32_WM_State {
  Arena *arena;
  HINSTANCE hInstance;
  HCURSOR cursors[WM_Cursor_COUNT];
  WM_Cursor last_set_cursor;
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

////////////////////////////////
// NOTE: Helpers

internal Range2         w32_wm_range2_from_rect(RECT rect);
internal WM_Window      w32_wm_handle_from_window(W32_WM_Window *window);
internal W32_WM_Window *w32_wm_window_from_handle(WM_Window handle);
internal W32_WM_Window *w32_wm_window_from_hwnd(HWND hwnd);
internal W32_WM_Window *w32_wm_window_alloc(void);
internal void           w32_wm_window_release(W32_WM_Window *window);
internal WM_Key         w32_wm_key_from_vkey(WPARAM vkey);
internal WM_Key         w32_wm_vkey_from_key(WM_Key key);
internal WM_Event      *w32_wm_push_event(WM_Event_Type type, W32_WM_Window *window);
internal LRESULT        w32_wm_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // W32_WM_H
