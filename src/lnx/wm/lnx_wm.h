#ifndef LNX_WM_H
#define LNX_WM_H

////////////////////////////////
// NOTE: Includes/Libraries

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xcursor/Xcursor.h>
#include <poll.h>
#include <sys/eventfd.h>

////////////////////////////////
// NOTE: Windows

typedef struct LNX_WM_Window LNX_WM_Window;
struct LNX_WM_Window {
  LNX_WM_Window *next;
  LNX_WM_Window *prev;
  Window window;
  XIC xic;
};

////////////////////////////////
// NOTE: State

typedef struct LNX_WM_State LNX_WM_State;
struct LNX_WM_State {
  Arena *arena;
  Display *display;
  XIM xim;
  s32 wakeup_fd;
  Atom wm_delete_window_atom;
  Cursor cursors[WM_Cursor_COUNT];
  WM_Cursor last_set_cursor;
  WM_System_Info system_info;
  LNX_WM_Window *first_window;
  LNX_WM_Window *last_window;
  LNX_WM_Window *first_free_window;
};

////////////////////////////////
// NOTE: Globals

global LNX_WM_State *lnx_wm_state = 0;

////////////////////////////////
// NOTE: Helpers

internal WM_Window      lnx_wm_handle_from_window(LNX_WM_Window *window);
internal LNX_WM_Window *lnx_wm_window_from_handle(WM_Window handle);
internal LNX_WM_Window *lnx_wm_window_from_x11_window(Window window);
internal LNX_WM_Window *lnx_wm_window_alloc(void);
internal void           lnx_wm_window_release(LNX_WM_Window *window);
internal WM_Key         lnx_wm_key_from_keysym(KeySym ks, b32 *out_is_right_sided);
internal KeySym         lnx_wm_keysym_from_key(WM_Key key);

#endif // LNX_WM_H
