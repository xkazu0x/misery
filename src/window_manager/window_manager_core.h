#ifndef WINDOW_MANAGER_CORE_H
#define WINDOW_MANAGER_CORE_H

////////////////////////////////
// NOTE: Graphics System Info

typedef struct WM_System_Info WM_System_Info;
struct WM_System_Info {
  f32 default_refresh_rate;
};

////////////////////////////////
// NOTE: Window Types

typedef u32 WM_Window_Flags;
enum {
  WM_Window_Flag_CUSTOM_BORDER        = (1<<0),
  WM_Window_Flag_USE_DEFAULT_POSITION = (1<<1),
};

typedef struct WM_Window WM_Window;
struct WM_Window {
  u64 v[1];
};

////////////////////////////////
// NOTE: Key Types

typedef enum WM_Key {
  WM_Key_NULL,

  WM_Key_ESC,
  WM_Key_F1,
  WM_Key_F2,
  WM_Key_F3,
  WM_Key_F4,
  WM_Key_F5,
  WM_Key_F6,
  WM_Key_F7,
  WM_Key_F8,
  WM_Key_F9,
  WM_Key_F10,
  WM_Key_F11,
  WM_Key_F12,
  WM_Key_F13,
  WM_Key_F14,
  WM_Key_F15,
  WM_Key_F16,
  WM_Key_F17,
  WM_Key_F18,
  WM_Key_F19,
  WM_Key_F20,
  WM_Key_F21,
  WM_Key_F22,
  WM_Key_F23,
  WM_Key_F24,

  WM_Key_TICK,
  WM_Key_0,
  WM_Key_1,
  WM_Key_2,
  WM_Key_3,
  WM_Key_4,
  WM_Key_5,
  WM_Key_6,
  WM_Key_7,
  WM_Key_8,
  WM_Key_9,
  WM_Key_MINUS,
  WM_Key_EQUAL,
  WM_Key_BACKSPACE,

  WM_Key_TAB,
  WM_Key_Q,
  WM_Key_W,
  WM_Key_E,
  WM_Key_R,
  WM_Key_T,
  WM_Key_Y,
  WM_Key_U,
  WM_Key_I,
  WM_Key_O,
  WM_Key_P,
  WM_Key_LEFTBRACKET,
  WM_Key_RIGHTBRACKET,
  WM_Key_BACKSLASH,

  WM_Key_CAPSLOCK,
  WM_Key_A,
  WM_Key_S,
  WM_Key_D,
  WM_Key_F,
  WM_Key_G,
  WM_Key_H,
  WM_Key_J,
  WM_Key_K,
  WM_Key_L,
  WM_Key_SEMICOLON,
  WM_Key_QUOTE,
  WM_Key_RETURN,

  WM_Key_SHIFT,
  WM_Key_Z,
  WM_Key_X,
  WM_Key_C,
  WM_Key_V,
  WM_Key_B,
  WM_Key_N,
  WM_Key_M,
  WM_Key_COMMA,
  WM_Key_PERIOD,
  WM_Key_SLASH,

  WM_Key_CTRL,
  WM_Key_ALT,
  WM_Key_SPACE,
  WM_Key_MENU,

  WM_Key_SCROLLLOCK,
  WM_Key_PAUSE,
  WM_Key_INSERT,
  WM_Key_HOME,
  WM_Key_PAGEUP,
  WM_Key_DELETE,
  WM_Key_END,
  WM_Key_PAGEDOWN,

  WM_Key_UP,
  WM_Key_LEFT,
  WM_Key_DOWN,
  WM_Key_RIGHT,

  WM_Key_EX0,
  WM_Key_EX1,
  WM_Key_EX2,
  WM_Key_EX3,
  WM_Key_EX4,
  WM_Key_EX5,
  WM_Key_EX6,
  WM_Key_EX7,
  WM_Key_EX8,
  WM_Key_EX9,
  WM_Key_EX10,
  WM_Key_EX11,
  WM_Key_EX12,
  WM_Key_EX13,
  WM_Key_EX14,
  WM_Key_EX15,
  WM_Key_EX16,
  WM_Key_EX17,
  WM_Key_EX18,
  WM_Key_EX19,
  WM_Key_EX20,
  WM_Key_EX21,
  WM_Key_EX22,
  WM_Key_EX23,
  WM_Key_EX24,
  WM_Key_EX25,
  WM_Key_EX26,
  WM_Key_EX27,
  WM_Key_EX28,
  WM_Key_EX29,

  WM_Key_NUMLOCK,
  WM_Key_NUMSLASH,
  WM_Key_NUMSTAR,
  WM_Key_NUMMINUS,
  WM_Key_NUMPLUS,
  WM_Key_NUMPERIOD,
  WM_Key_NUM0,
  WM_Key_NUM1,
  WM_Key_NUM2,
  WM_Key_NUM3,
  WM_Key_NUM4,
  WM_Key_NUM5,
  WM_Key_NUM6,
  WM_Key_NUM7,
  WM_Key_NUM8,
  WM_Key_NUM9,

  WM_Key_MOUSE_LEFT,
  WM_Key_MOUSE_MIDDLE,
  WM_Key_MOUSE_RIGHT,

  WM_Key_COUNT,
} WM_Key;

////////////////////////////////
// NOTE: Event Types

typedef enum WM_Event_Type {
  WM_Event_Type_NULL,
  WM_Event_Type_PRESS,
  WM_Event_Type_RELEASE,
  WM_Event_Type_TEXT_INPUT,
  WM_Event_Type_MOUSE_MOVE,
  WM_Event_Type_MOUSE_WHEEL,
  WM_Event_Type_WINDOW_LOSE_FOCUS,
  WM_Event_Type_WINDOW_CLOSE,
  WM_Event_Type_WAKEUP,
  WM_Event_Type_COUNT,
} WM_Event_Type;

typedef u32 WM_Modifiers;
enum {
  WM_Modifier_CTRL  = (1<<0),
  WM_Modifier_SHIFT = (1<<1),
  WM_Modifier_ALT   = (1<<2),
};

typedef struct WM_Event WM_Event;
struct WM_Event {
  WM_Event *next;
  WM_Event *prev;
  u64 timestamp_us;
  WM_Window window;
  WM_Event_Type type;
  WM_Modifiers modifiers;

  // NOTE: keys
  WM_Key key;
  b32 is_repeat;
  b32 is_right_sided;
  u32 character;
  u32 repeat_count;

  // NOTE: mouse
  Vector2 position;
  Vector2 wheel_delta;
};

typedef struct WM_Event_List WM_Event_List;
struct WM_Event_List {
  WM_Event *first;
  WM_Event *last;
  u64 count;
};

////////////////////////////////
// NOTE: Cursor Types

typedef enum WM_Cursor {
  WM_Cursor_NONE,
  WM_Cursor_POINTER,
  WM_Cursor_COUNT,
} WM_Cursor;

////////////////////////////////
// NOTE: Application-Defined Frame Hook Forward Declaration

internal b32 frame(void);

////////////////////////////////
// NOTE: Handle Type Helpers

internal WM_Window   wm_window_zero(void);
internal b32         wm_window_match(WM_Window a, WM_Window b);

////////////////////////////////
// NOTE: Event Helpers

internal WM_Event *wm_event_list_push(Arena *arena, WM_Event_List *list, WM_Event_Type type);
internal void      wm_eat_event(WM_Event_List *list, WM_Event *event);

////////////////////////////////
// NOTE: @per_os_impl Main Initialization

internal void wm_init(void);

////////////////////////////////
// NOTE: @per_os_impl Graphics System Info

internal WM_System_Info *wm_get_system_info(void);

////////////////////////////////
// NOTE: @per_os_impl Windows

internal WM_Window wm_window_open(String8 name, Vector2 size, WM_Window_Flags flags);
internal void      wm_window_close(WM_Window window);
internal void      wm_window_first_paint(WM_Window window);
internal void      wm_window_focus(WM_Window window);
internal void      wm_window_bring_to_front(WM_Window window);
internal b32       wm_window_is_focused(WM_Window window);
internal b32       wm_window_is_fullscreen(WM_Window window);
internal b32       wm_window_is_maximized(WM_Window window);
internal b32       wm_window_is_minimized(WM_Window window);
internal void      wm_window_set_name(WM_Window window, String8 name);
internal void      wm_window_set_fullscreen(WM_Window window, b32 fullscreen);
internal void      wm_window_set_maximized(WM_Window window, b32 maximized);
internal void      wm_window_set_minimized(WM_Window window, b32 minimized);
internal void      wm_window_set_custom_title_thickness(WM_Window handle, f32 thickness);
internal void      wm_window_set_custom_edge_thickness(WM_Window handle, f32 thickness);
internal Range2    wm_rect_from_window(WM_Window window);
internal Range2    wm_client_rect_from_window(WM_Window window);
internal f32       wm_dpi_from_window(WM_Window window);

////////////////////////////////
// NOTE: @per_os_impl Events

internal void          wm_send_wakeup_event(void);
internal WM_Event_List wm_get_events(Arena *arena, b32 wait);
internal WM_Modifiers  wm_get_modifiers(void);
internal Vector2       wm_mouse_from_window(WM_Window window);

////////////////////////////////
// NOTE: @per_os_impl Cursors

internal void wm_set_cursor(WM_Cursor cursor);

////////////////////////////////
// NOTE: @per_os_impl Native User-Facing Graphical Messages

internal void wm_graphical_message(b32 error, String8 title, String8 message);

#endif // WINDOW_MANAGER_CORE_H
