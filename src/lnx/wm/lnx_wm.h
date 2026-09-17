#ifndef LNX_WM_H
#define LNX_WM_H

////////////////////////////////
// NOTE: Includes/Libraries

////////////////////////////////
// NOTE: Windows

typedef struct LNX_WM_Window LNX_WM_Window;
struct LNX_WM_Window {
  LNX_WM_Window *next;
  LNX_WM_Window *prev;
};

////////////////////////////////
// NOTE: State

typedef struct LNX_WM_State LNX_WM_State;
struct LNX_WM_State {
  Arena *arena;
};

////////////////////////////////
// NOTE: Globals

global LNX_WM_State *lnx_wm_state = 0;

#endif // LNX_WM_H
