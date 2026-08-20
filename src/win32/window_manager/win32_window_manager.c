////////////////////////////////
// NOTE: Modern Windows SDK Functions
// (We must dynamically link to them, since they can be missing in older SDKs)

typedef BOOL w32_SetProcessDpiAwarenessContext_Type(void* value);
typedef UINT w32_GetDpiForWindow_Type(HWND hwnd);
typedef HRESULT w32_GetDpiForMonitor_Type(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);
typedef int w32_GetSystemMetricsForDpi_Type(int nIndex, UINT dpi);
#define w32_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((void *)-4)
global w32_GetDpiForWindow_Type *w32_GetDpiForWindow_func = 0;
global w32_GetDpiForMonitor_Type *w32_GetDpiForMonitor_func = 0;
global w32_GetSystemMetricsForDpi_Type *w32_GetSystemMetricsForDpi_func = 0;

////////////////////////////////
// NOTE: Basic Helpers

internal Range2
w32_wm_range2_from_rect(RECT rect) {
  Range2 result = {0};
  result.x0 = (f32)rect.left;
  result.y0 = (f32)rect.top;
  result.x1 = (f32)rect.right;
  result.y1 = (f32)rect.bottom;
  return(result);
}

////////////////////////////////
// NOTE: Windows

internal WM_Window
w32_wm_handle_from_window(W32_WM_Window *window) {
  WM_Window result = {0};
  result.v[0] = (u64)window;
  return(result);
}

internal W32_WM_Window *
w32_wm_window_from_handle(WM_Window handle) {
  W32_WM_Window *result = (W32_WM_Window *)handle.v[0];
  return(result);
}

internal W32_WM_Window *
w32_wm_window_from_hwnd(HWND hwnd) {
  W32_WM_Window *result = 0;
  for (W32_WM_Window *w = w32_wm_state->first_window; w; w = w->next) {
    if (w->hwnd == hwnd) {
      result = w;
      break;
    }
  }
  return(result);
}

internal HWND
w32_wm_hwnd_from_window(W32_WM_Window *window) {
  return(window->hwnd);
}

internal W32_WM_Window *
w32_wm_window_alloc(void) {
  W32_WM_Window *result = w32_wm_state->free_window;
  if (result != 0) {
    sll_stack_pop(w32_wm_state->free_window);
  } else {
    result = push_array_no_zero(w32_wm_state->arena, W32_WM_Window, 1);
  }
  memory_zero_struct(result);
  dll_push_back(w32_wm_state->first_window, w32_wm_state->last_window, result);
  result->last_placement.length = sizeof(WINDOWPLACEMENT);
  return(result);
}

internal void
w32_wm_window_release(W32_WM_Window *window) {
  ReleaseDC(window->hwnd, window->hdc);
  DestroyWindow(window->hwnd);
  dll_remove(w32_wm_state->first_window, w32_wm_state->last_window, window);
  sll_stack_push(w32_wm_state->free_window, window);
}

internal WM_Event *
w32_wm_push_event(WM_Event_Type type, W32_WM_Window *window) {
  WM_Event *event = wm_event_list_push(w32_wm_event_arena, &w32_wm_event_list, type);
  event->window = w32_wm_handle_from_window(window);
  event->modifiers = wm_get_modifiers();
  return(event);
}

internal LRESULT CALLBACK
w32_wm_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;

  W32_WM_Window *window = w32_wm_window_from_hwnd(hwnd);
  WM_Window window_handle = w32_wm_handle_from_window(window);
  b32 release = 0;

  switch (uMsg) {
    default: {
      result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
    } break;

    case WM_ENTERSIZEMOVE: {
      w32_wm_resizing = 1;
    } break;

    case WM_EXITSIZEMOVE: {
      w32_wm_resizing = 0;
    } break;

    case WM_SIZE:
    case WM_PAINT: {
      PAINTSTRUCT ps = {0};
      BeginPaint(hwnd, &ps);
      frame();
      EndPaint(hwnd, &ps);
      DwmFlush();
    } break;

    case WM_CLOSE: {
      w32_wm_push_event(WM_Event_Type_WINDOW_CLOSE, window);
      wm_send_wakeup_event();
    } break;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP: {
      release = 1;
    } // NOTE: fallthrough
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN: {
      WM_Event *event = w32_wm_push_event(release ? WM_Event_Type_RELEASE : WM_Event_Type_PRESS, window);
      switch (uMsg) {
        case WM_LBUTTONUP: case WM_LBUTTONDOWN: {event->key = WM_Key_MOUSE_LEFT;}   break;
        case WM_MBUTTONUP: case WM_MBUTTONDOWN: {event->key = WM_Key_MOUSE_MIDDLE;} break;
        case WM_RBUTTONUP: case WM_RBUTTONDOWN: {event->key = WM_Key_MOUSE_RIGHT;}  break;
      }
      event->position.x = (f32)(s16)LOWORD(lParam);
      event->position.y = (f32)(s16)HIWORD(lParam);
      if (release) {
        ReleaseCapture();
      } else {
        SetCapture(hwnd);
      }
    } break;

    case WM_MOUSEMOVE: {
      WM_Event *event = w32_wm_push_event(WM_Event_Type_MOUSE_MOVE, window);
      event->position.x = (f32)(s16)LOWORD(lParam);
      event->position.y = (f32)(s16)HIWORD(lParam);
    } break;

    case WM_MOUSEWHEEL: {
      s16 wheel_delta = HIWORD(wParam);
      WM_Event *event = w32_wm_push_event(WM_Event_Type_MOUSE_WHEEL, window);
      POINT p;
      p.x = (s32)(s16)LOWORD(lParam);
      p.y = (s32)(s16)HIWORD(lParam);
      ScreenToClient(window->hwnd, &p);
      event->position.x = (f32)p.x;
      event->position.y = (f32)p.y;
      event->wheel_delta = vector2_make(0.0f, -(f32)wheel_delta/WHEEL_DELTA);
    } break;

    case WM_MOUSEHWHEEL: {
      s16 wheel_delta = HIWORD(wParam);
      WM_Event *event = w32_wm_push_event(WM_Event_Type_MOUSE_WHEEL, window);
      POINT p;
      p.x = (s32)(s16)LOWORD(lParam);
      p.y = (s32)(s16)HIWORD(lParam);
      ScreenToClient(window->hwnd, &p);
      event->position.x = (f32)p.x;
      event->position.y = (f32)p.y;
      event->wheel_delta = vector2_make((f32)wheel_delta/WHEEL_DELTA, 0.0f);
    } break;

    case WM_SYSKEYUP:
    case WM_SYSKEYDOWN: {
      if (wParam != VK_MENU && (wParam < VK_F1 || VK_F24 < wParam || wParam == VK_F4)) {
        result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
      }
    } // NOTE: fallthrough
    case WM_KEYUP:
    case WM_KEYDOWN: {
      b32 was_down = (lParam & bit31);
      b32 is_down = !(lParam & bit32);

      b32 is_repeat = 0;
      if (!is_down) {
        release = 1;
      } else if(was_down) {
        is_repeat = 1;
      }

      b32 is_right_sided = 0;
      if ((lParam & bit25) &&
          (wParam == VK_CONTROL || wParam == VK_RCONTROL ||
           wParam == VK_SHIFT || wParam == VK_RSHIFT ||
           wParam == VK_MENU || wParam == VK_RMENU)) {
        is_right_sided = 1;
      }

      WM_Event *event = w32_wm_push_event(release ? WM_Event_Type_RELEASE : WM_Event_Type_PRESS, window);
      event->key = w32_wm_state->key_from_vkey_table[wParam&bitmask8];
      event->is_repeat = is_repeat;
      event->is_right_sided = is_right_sided;
      event->repeat_count = lParam & bitmask16;
      if (event->key == WM_Key_CTRL  && event->modifiers & WM_Modifier_CTRL)  {event->modifiers &= ~WM_Modifier_CTRL;}
      if (event->key == WM_Key_SHIFT && event->modifiers & WM_Modifier_SHIFT) {event->modifiers &= ~WM_Modifier_SHIFT;}
      if (event->key == WM_Key_ALT   && event->modifiers & WM_Modifier_ALT)   {event->modifiers &= ~WM_Modifier_ALT;}
    } break;

    case WM_SYSCHAR: {
      WORD vk_code = LOWORD(wParam);
      if (vk_code == VK_SPACE) {
        result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
      } else {
        result = 0;
      }
    } break;

    case WM_CHAR: {
      u32 character = (u32)wParam;
      if (character >= 32 && character != 127) {
        WM_Event *event = w32_wm_push_event(WM_Event_Type_TEXT_INPUT, window);
        if (lParam & bit29) {
          event->modifiers |= WM_Modifier_ALT;
        }
        event->character = character;
      }
    } break;

    case WM_KILLFOCUS: {
      w32_wm_push_event(WM_Event_Type_WINDOW_LOSE_FOCUS, window);
      ReleaseCapture();
    } break;

    case WM_SETCURSOR: {
      Range2 window_rect = wm_client_rect_from_window(window_handle);
      Vector2 mouse = wm_mouse_from_window(window_handle);
      b32 on_border = 0;
      DWORD window_style = window ? GetWindowLong(window->hwnd, GWL_STYLE) : 0;
      b32 is_fullscreen = !(window_style & WS_OVERLAPPEDWINDOW);
      if (window != 0 && window->custom_border && !is_fullscreen) {
        b32 on_border_x = ((mouse.x <= window->custom_border_edge_thickness) || ((window_rect.x1-window->custom_border_edge_thickness) <= mouse.x));
        b32 on_border_y = ((mouse.y <= window->custom_border_edge_thickness) || ((window_rect.y1-window->custom_border_edge_thickness) <= mouse.y));
        on_border = on_border_x || on_border_y;
      }
      if (!w32_wm_resizing && !on_border && range2_contains(window_rect, mouse)) {
        SetCursor(w32_wm_state->hCursor);
      } else {
        result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
      }
    } break;

    // case WM_SETICON:
    // case WM_SETTEXT: {
    //   if (w32_wm_new_window_custom_border || (window != 0 && window->custom_border && !window->custom_border_composition_enabled)) {
    //     // NOTE: https://blogs.msdn.microsoft.com/wpfsdk/2008/09/08/custom-window-chrome-in-wpf/
    //     LONG_PTR old_style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    //     SetWindowLongPtrW(hwnd, GWL_STYLE, old_style & ~WS_VISIBLE);
    //     result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
    //     SetWindowLongPtrW(hwnd, GWL_STYLE, old_style);
    //   } else {
    //     result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
    //   }
    // } break;

    case WM_DPICHANGED: {
      f32 new_dpi = (f32)(wParam & 0xffff);
      RECT suggested_new_rect = *(RECT *)lParam;
      window->dpi = new_dpi;
      SetWindowPos(window->hwnd, 0,
                   suggested_new_rect.left,
                   suggested_new_rect.top,
                   suggested_new_rect.right - suggested_new_rect.left,
                   suggested_new_rect.bottom - suggested_new_rect.top,
                   0);
    } break;

    // NOTE: [custom border] client/window size calculation
    case WM_NCCALCSIZE: {
      if (w32_wm_new_window_custom_border || (window != 0 && window->custom_border)) {
        f32 dpi = w32_GetDpiForWindow_func ? (f32)w32_GetDpiForWindow_func(hwnd) : 96.0f;
        s32 frame_x = w32_GetSystemMetricsForDpi_func ? w32_GetSystemMetricsForDpi_func(SM_CXFRAME, (UINT)dpi) : GetSystemMetrics(SM_CXFRAME);
        s32 frame_y = w32_GetSystemMetricsForDpi_func ? w32_GetSystemMetricsForDpi_func(SM_CYFRAME, (UINT)dpi) : GetSystemMetrics(SM_CYFRAME);
        s32 padding = w32_GetSystemMetricsForDpi_func ? w32_GetSystemMetricsForDpi_func(SM_CXPADDEDBORDER, (UINT)dpi) : GetSystemMetrics(SM_CXPADDEDBORDER);
        DWORD window_style = GetWindowLong(hwnd, GWL_STYLE);
        b32 is_fullscreen = !(window_style & WS_OVERLAPPEDWINDOW);
        if (!is_fullscreen) {
          RECT* rect = wParam == 0 ? (RECT*)lParam : ((NCCALCSIZE_PARAMS*)lParam)->rgrc;
          rect->right  -= frame_x + padding;
          rect->left   += frame_x + padding;
          rect->bottom -= frame_y + padding;
          if (IsMaximized(hwnd)) {
            rect->top += frame_y + padding;
            // NOTE: if we do not do this hidden taskbar can not be unhidden on mouse hover
            // Unfortunately it can create an ugly bottom border when maximized...
            rect->bottom -= 1; 
          }
        }
      } else {
        result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
      }
    } break;

    // NOTE: [custom border] client/window hit testing (mapping mouse -> action)
    case WM_NCHITTEST: {
      DWORD window_style = window ? GetWindowLong(hwnd, GWL_STYLE) : 0;
      b32 is_fullscreen = !(window_style & WS_OVERLAPPEDWINDOW);
      if (window == 0 || window->custom_border == 0 || is_fullscreen) {
        result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
      } else {
        b32 is_default_handled = 0;

        // NOTE: let the default procedure handle resizing areas
        result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
        switch (result) {
          case HTNOWHERE:
          case HTRIGHT:
          case HTLEFT:
          case HTTOPLEFT:
          case HTTOPRIGHT:
          case HTBOTTOMRIGHT:
          case HTBOTTOM:
          case HTBOTTOMLEFT: {
            is_default_handled = 1;
          } break;
        }

        if (!is_default_handled) {
          POINT client_pos = {0};
          client_pos.x = GET_X_LPARAM(lParam);
          client_pos.y = GET_Y_LPARAM(lParam);
          ScreenToClient(hwnd, &client_pos);

          f32 dpi = w32_GetDpiForWindow_func ? (f32)w32_GetDpiForWindow_func(hwnd) : 96.0f;
          s32 frame_y = w32_GetSystemMetricsForDpi_func ? w32_GetSystemMetricsForDpi_func(SM_CYFRAME, (UINT)dpi) : GetSystemMetrics(SM_CYFRAME);

          b32 is_over_top_resize = client_pos.y >= 0 && client_pos.y < frame_y;
          b32 is_over_title_bar  = client_pos.y >= 0 && client_pos.y < window->custom_border_title_thickness;

          if (IsMaximized(hwnd)) {
            if (is_over_title_bar) {
              result = HTCAPTION;
            } else {
              result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
            }
          } else {
            if (is_over_top_resize) {
              result = HTTOP;
            } else if (is_over_title_bar) {
              result = HTCAPTION;
            } else {
              result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
            }
          }
        }
      }
    } break;
  }

  return(result);
}

////////////////////////////////
// NOTE: @per_os_impl Main Initialization

internal void
wm_init(void) {
  // NOTE: set up base state
  Arena *arena = arena_alloc();
  w32_wm_state = push_array(arena, W32_WM_State, 1);
  w32_wm_state->arena = arena;
  w32_wm_state->gfx_thread_tid = (u32)GetCurrentThreadId();
  w32_wm_state->hInstance = GetModuleHandle(0);

  // NOTE: set dpi awareness
  w32_SetProcessDpiAwarenessContext_Type *w32_SetProcessDpiAwarenessContext_func = 0;
  HMODULE module = LoadLibraryA("user32.dll");
  if (module != 0) {
    w32_SetProcessDpiAwarenessContext_func = (w32_SetProcessDpiAwarenessContext_Type *)GetProcAddress(module, "SetProcessDpiAwarenessContext");
    w32_GetDpiForWindow_func = (w32_GetDpiForWindow_Type *)GetProcAddress(module, "GetDpiForWindow");
    w32_GetDpiForMonitor_func = (w32_GetDpiForMonitor_Type *)GetProcAddress(module, "GetDpiForMonitor");
    w32_GetSystemMetricsForDpi_func = (w32_GetSystemMetricsForDpi_Type *)GetProcAddress(module, "GetSystemMetricsForDpi");
    FreeLibrary(module);
  }
  if (w32_SetProcessDpiAwarenessContext_func != 0) {
    w32_SetProcessDpiAwarenessContext_func(w32_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  } else {
    HMODULE shcore = LoadLibraryA("shcore.dll");
    if (shcore) {
      typedef HRESULT WINAPI w32_SetProcessDpiAwareness_Type(int);
      w32_SetProcessDpiAwareness_Type *w32_SetProcessDpiAwareness_func = (w32_SetProcessDpiAwareness_Type *)GetProcAddress(shcore, "SetProcessDpiAwareness");
      if (w32_SetProcessDpiAwareness_func) {
        w32_SetProcessDpiAwareness_func(2);
      }
      FreeLibrary(shcore);
    }
    SetProcessDPIAware();
  }

  // NOTE: register graphical-window class
  {
    WNDCLASSEXW wndclass = {sizeof(wndclass)};
    wndclass.lpfnWndProc = w32_wm_window_proc;
    wndclass.hInstance = w32_wm_state->hInstance;
    wndclass.lpszClassName = L"graphical-window";
    wndclass.hCursor = LoadCursor(0, IDC_ARROW);
    wndclass.hIcon = LoadIcon(0, IDI_APPLICATION); // LoadIcon(w32_wm_state->hInstance, MAKEINTRESOURCE(1))
    wndclass.hbrBackground = GetStockObject(BLACK_BRUSH);
    wndclass.style = CS_VREDRAW|CS_HREDRAW;
    ATOM wndatom = RegisterClassExW(&wndclass);
    (void)wndatom;
  }

  // NOTE: grab graphics system info
  {
    DEVMODEW devmodew = {0};
    if (EnumDisplaySettingsW(0, ENUM_CURRENT_SETTINGS, &devmodew)) {
      w32_wm_state->system_info.default_refresh_rate = (f32)devmodew.dmDisplayFrequency;
    }
  }

  // NOTE: set initial cursor
  wm_set_cursor(WM_Cursor_POINTER);

  // NOTE: fill VKEY -> WM_Key table
  {
    w32_wm_state->key_from_vkey_table[(unsigned int)'A'] = WM_Key_A;
    w32_wm_state->key_from_vkey_table[(unsigned int)'B'] = WM_Key_B;
    w32_wm_state->key_from_vkey_table[(unsigned int)'C'] = WM_Key_C;
    w32_wm_state->key_from_vkey_table[(unsigned int)'D'] = WM_Key_D;
    w32_wm_state->key_from_vkey_table[(unsigned int)'E'] = WM_Key_E;
    w32_wm_state->key_from_vkey_table[(unsigned int)'F'] = WM_Key_F;
    w32_wm_state->key_from_vkey_table[(unsigned int)'G'] = WM_Key_G;
    w32_wm_state->key_from_vkey_table[(unsigned int)'H'] = WM_Key_H;
    w32_wm_state->key_from_vkey_table[(unsigned int)'I'] = WM_Key_I;
    w32_wm_state->key_from_vkey_table[(unsigned int)'J'] = WM_Key_J;
    w32_wm_state->key_from_vkey_table[(unsigned int)'K'] = WM_Key_K;
    w32_wm_state->key_from_vkey_table[(unsigned int)'L'] = WM_Key_L;
    w32_wm_state->key_from_vkey_table[(unsigned int)'M'] = WM_Key_M;
    w32_wm_state->key_from_vkey_table[(unsigned int)'N'] = WM_Key_N;
    w32_wm_state->key_from_vkey_table[(unsigned int)'O'] = WM_Key_O;
    w32_wm_state->key_from_vkey_table[(unsigned int)'P'] = WM_Key_P;
    w32_wm_state->key_from_vkey_table[(unsigned int)'Q'] = WM_Key_Q;
    w32_wm_state->key_from_vkey_table[(unsigned int)'R'] = WM_Key_R;
    w32_wm_state->key_from_vkey_table[(unsigned int)'S'] = WM_Key_S;
    w32_wm_state->key_from_vkey_table[(unsigned int)'T'] = WM_Key_T;
    w32_wm_state->key_from_vkey_table[(unsigned int)'U'] = WM_Key_U;
    w32_wm_state->key_from_vkey_table[(unsigned int)'V'] = WM_Key_V;
    w32_wm_state->key_from_vkey_table[(unsigned int)'W'] = WM_Key_W;
    w32_wm_state->key_from_vkey_table[(unsigned int)'X'] = WM_Key_X;
    w32_wm_state->key_from_vkey_table[(unsigned int)'Y'] = WM_Key_Y;
    w32_wm_state->key_from_vkey_table[(unsigned int)'Z'] = WM_Key_Z;

    for(u32 i = '0', j = WM_Key_0; i <= '9'; i += 1, j += 1) {
      w32_wm_state->key_from_vkey_table[i] = (WM_Key)j;
    }
    for(u32 i = VK_NUMPAD0, j = WM_Key_0; i <= VK_NUMPAD9; i += 1, j += 1) {
      w32_wm_state->key_from_vkey_table[i] = (WM_Key)j;
    }
    for(u32 i = VK_F1, j = WM_Key_F1; i <= VK_F24; i += 1, j += 1) {
      w32_wm_state->key_from_vkey_table[i] = (WM_Key)j;
    }

    w32_wm_state->key_from_vkey_table[VK_SPACE]     = WM_Key_SPACE;
    w32_wm_state->key_from_vkey_table[VK_OEM_3]     = WM_Key_TICK;
    w32_wm_state->key_from_vkey_table[VK_OEM_MINUS] = WM_Key_MINUS;
    w32_wm_state->key_from_vkey_table[VK_OEM_PLUS]  = WM_Key_EQUAL;
    w32_wm_state->key_from_vkey_table[VK_OEM_4]     = WM_Key_LEFTBRACKET;
    w32_wm_state->key_from_vkey_table[VK_OEM_6]     = WM_Key_RIGHTBRACKET;
    w32_wm_state->key_from_vkey_table[VK_OEM_1]     = WM_Key_SEMICOLON;
    w32_wm_state->key_from_vkey_table[VK_OEM_7]     = WM_Key_QUOTE;
    w32_wm_state->key_from_vkey_table[VK_OEM_COMMA] = WM_Key_COMMA;
    w32_wm_state->key_from_vkey_table[VK_OEM_PERIOD]= WM_Key_PERIOD;
    w32_wm_state->key_from_vkey_table[VK_OEM_2]     = WM_Key_SLASH;
    w32_wm_state->key_from_vkey_table[VK_OEM_5]     = WM_Key_BACKSLASH;

    w32_wm_state->key_from_vkey_table[VK_TAB]       = WM_Key_TAB;
    w32_wm_state->key_from_vkey_table[VK_PAUSE]     = WM_Key_PAUSE;
    w32_wm_state->key_from_vkey_table[VK_ESCAPE]    = WM_Key_ESC;

    w32_wm_state->key_from_vkey_table[VK_UP]        = WM_Key_UP;
    w32_wm_state->key_from_vkey_table[VK_LEFT]      = WM_Key_LEFT;
    w32_wm_state->key_from_vkey_table[VK_DOWN]      = WM_Key_DOWN;
    w32_wm_state->key_from_vkey_table[VK_RIGHT]     = WM_Key_RIGHT;

    w32_wm_state->key_from_vkey_table[VK_BACK]      = WM_Key_BACKSPACE;
    w32_wm_state->key_from_vkey_table[VK_RETURN]    = WM_Key_RETURN;

    w32_wm_state->key_from_vkey_table[VK_DELETE]    = WM_Key_DELETE;
    w32_wm_state->key_from_vkey_table[VK_INSERT]    = WM_Key_INSERT;
    w32_wm_state->key_from_vkey_table[VK_PRIOR]     = WM_Key_PAGEUP;
    w32_wm_state->key_from_vkey_table[VK_NEXT]      = WM_Key_PAGEDOWN;
    w32_wm_state->key_from_vkey_table[VK_HOME]      = WM_Key_HOME;
    w32_wm_state->key_from_vkey_table[VK_END]       = WM_Key_END;

    w32_wm_state->key_from_vkey_table[VK_CAPITAL]   = WM_Key_CAPSLOCK;
    w32_wm_state->key_from_vkey_table[VK_NUMLOCK]   = WM_Key_NUMLOCK;
    w32_wm_state->key_from_vkey_table[VK_SCROLL]    = WM_Key_SCROLLLOCK;
    w32_wm_state->key_from_vkey_table[VK_APPS]      = WM_Key_MENU;

    w32_wm_state->key_from_vkey_table[VK_SHIFT]     = WM_Key_SHIFT;
    w32_wm_state->key_from_vkey_table[VK_LSHIFT]    = WM_Key_SHIFT;
    w32_wm_state->key_from_vkey_table[VK_RSHIFT]    = WM_Key_SHIFT;
    w32_wm_state->key_from_vkey_table[VK_CONTROL]   = WM_Key_CTRL;
    w32_wm_state->key_from_vkey_table[VK_LCONTROL]  = WM_Key_CTRL;
    w32_wm_state->key_from_vkey_table[VK_RCONTROL]  = WM_Key_CTRL;
    w32_wm_state->key_from_vkey_table[VK_MENU]      = WM_Key_ALT;
    w32_wm_state->key_from_vkey_table[VK_LMENU]     = WM_Key_ALT;
    w32_wm_state->key_from_vkey_table[VK_RMENU]     = WM_Key_ALT;

    w32_wm_state->key_from_vkey_table[VK_DIVIDE]   = WM_Key_NUMSLASH;
    w32_wm_state->key_from_vkey_table[VK_MULTIPLY] = WM_Key_NUMSTAR;
    w32_wm_state->key_from_vkey_table[VK_SUBTRACT] = WM_Key_NUMMINUS;
    w32_wm_state->key_from_vkey_table[VK_ADD]      = WM_Key_NUMPLUS;
    w32_wm_state->key_from_vkey_table[VK_DECIMAL]  = WM_Key_NUMPERIOD;

    for (u32 i = 0; i < 10; i += 1) {
      w32_wm_state->key_from_vkey_table[VK_NUMPAD0 + i] = (WM_Key)((u32)WM_Key_NUM0 + i);
    }

    for (u32 i = 0xDF, j = 0; i < 0xFF; i += 1, j += 1) {
      w32_wm_state->key_from_vkey_table[i] = (WM_Key)((u32)WM_Key_EX0 + j);
    }
  }
}

////////////////////////////////
// NOTE: @per_os_impl Graphics System Info

internal WM_System_Info *
wm_get_system_info(void) {
  return(&w32_wm_state->system_info);
}

////////////////////////////////
// NOTE: @per_os_impl Windows

internal WM_Window
wm_window_open(String8 name, Vector2 size, WM_Window_Flags flags) {
  b32 custom_border = !!(flags & WM_Window_Flag_CUSTOM_BORDER);
  b32 use_default_position = !!(flags & WM_Window_Flag_USE_DEFAULT_POSITION);

  // NOTE: make hwnd
  HWND hwnd = 0;
  {
    Temp scratch = scratch_begin(0, 0);
    String16 name16 = str16_from_8(scratch.arena, name);
    w32_wm_new_window_custom_border = custom_border;

    DWORD ex_style_flags = WS_EX_APPWINDOW; // | WS_EX_NOREDIRECTIONBITMAP;
    DWORD style_flags = WS_OVERLAPPEDWINDOW;

    if (custom_border == 0) {
      RECT rect = {0, 0, (LONG)size.x, (LONG)size.y};
      if (AdjustWindowRectEx(&rect, style_flags, 0, ex_style_flags)) {
        size.x = (f32)(rect.right - rect.left);
        size.y = (f32)(rect.bottom - rect.top);
      }
    }

    s32 pos_x = CW_USEDEFAULT;
    s32 pos_y = CW_USEDEFAULT;
    if (!use_default_position) {
      s32 monitor_w = GetSystemMetrics(SM_CXSCREEN);
      s32 monitor_h = GetSystemMetrics(SM_CYSCREEN);
      pos_x = (monitor_w - (s32)size.x)/2;
      pos_y = (monitor_h - (s32)size.y)/2;
    }

    hwnd = CreateWindowExW(ex_style_flags,
                           L"graphical-window",
                           (WCHAR *)name16.str,
                           style_flags,
                           pos_x, pos_y,
                           (s32)size.x, (s32)size.y,
                           0, 0,
                           w32_wm_state->hInstance,
                           0);

    w32_wm_new_window_custom_border = 0;
    scratch_end(scratch);
  }

  // NOTE: make/fill window
  W32_WM_Window *window = w32_wm_window_alloc();
  {
    window->hwnd = hwnd;
    window->hdc = GetDC(hwnd);
    if (w32_GetDpiForWindow_func != 0) {
      window->dpi = (f32)w32_GetDpiForWindow_func(hwnd);
    } else {
      window->dpi = 96.0f;
    }
    window->custom_border = custom_border;
  }

  WM_Window result = w32_wm_handle_from_window(window);
  return(result);
}

internal void
wm_window_close(WM_Window handle) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    w32_wm_window_release(window);
  }
}

internal void
wm_window_first_paint(WM_Window handle) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  window->first_paint_done = 1;
  ShowWindow(window->hwnd, SW_SHOW);
  if (window->maximized) {
    ShowWindow(window->hwnd, SW_MAXIMIZE);
  }
}

internal void
wm_window_focus(WM_Window handle) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  SetForegroundWindow(window->hwnd);
  SetFocus(window->hwnd);
}

internal void
wm_window_bring_to_front(WM_Window handle) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    BringWindowToTop(window->hwnd);
  }
}

internal b32
wm_window_is_focused(WM_Window handle) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  HWND active_hwnd = GetForegroundWindow();
  return(active_hwnd == window->hwnd);
}

internal b32
wm_window_is_fullscreen(WM_Window handle) {
  b32 result = 0;
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    DWORD window_style = GetWindowLong(window->hwnd, GWL_STYLE);
    result = !(window_style & WS_OVERLAPPEDWINDOW);
  }
  return(result);
}

internal b32
wm_window_is_maximized(WM_Window handle) {
  b32 result = 0;
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    result = !!(IsZoomed(window->hwnd));
  }
  return(result);
}

internal b32
wm_window_is_minimized(WM_Window handle) {
  b32 result = 0;
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    result = !!(IsIconic(window->hwnd));
  }
  return(result);
}

internal void
wm_window_set_name(WM_Window handle, String8 name) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    Temp scratch = scratch_begin(0, 0);
    String16 name16 = str16_from_8(scratch.arena, name);
    SetWindowTextW(window->hwnd, (WCHAR *)name16.str);
    scratch_end(scratch);
  }
}

internal void
wm_window_set_fullscreen(WM_Window handle, b32 fullscreen) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    DWORD window_style = GetWindowLong(window->hwnd, GWL_STYLE);
    b32 is_fullscreen_already = wm_window_is_fullscreen(handle);
    if (fullscreen) {
      if (!is_fullscreen_already) {
        GetWindowPlacement(window->hwnd, &window->last_placement);
      }
      MONITORINFO monitor_info = {sizeof(monitor_info)};
      if(GetMonitorInfo(MonitorFromWindow(window->hwnd, MONITOR_DEFAULTTOPRIMARY), &monitor_info)) {
        SetWindowLong(window->hwnd, GWL_STYLE, window_style & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(window->hwnd, HWND_TOP,
                     monitor_info.rcMonitor.left,
                     monitor_info.rcMonitor.top,
                     monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                     monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
      }
    } else {
      SetWindowLong(window->hwnd, GWL_STYLE, window_style | WS_OVERLAPPEDWINDOW);
      SetWindowPlacement(window->hwnd, &window->last_placement);
      SetWindowPos(window->hwnd, 0, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                   SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
  }
}

internal void
wm_window_set_maximized(WM_Window handle, b32 maximized) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    if (window->first_paint_done) {
      switch (maximized) {
        default:
        case 0: {ShowWindow(window->hwnd, SW_RESTORE);} break;
        case 1: {ShowWindow(window->hwnd, SW_MAXIMIZE);} break;
      }
    } else {
      window->maximized = maximized;
    }
  }
}

internal void
wm_window_set_minimized(WM_Window handle, b32 minimized) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0 && minimized != wm_window_is_minimized(handle)) {
    switch(minimized) {
      default:
      case 0: {ShowWindow(window->hwnd, SW_RESTORE);} break;
      case 1: {ShowWindow(window->hwnd, SW_MINIMIZE);} break;
    }
  }
}

internal void
wm_window_set_custom_title_thickness(WM_Window handle, f32 thickness) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    window->custom_border_title_thickness = thickness;
  }
}

internal void
wm_window_set_custom_edge_thickness(WM_Window handle, f32 thickness) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    window->custom_border_edge_thickness = thickness;
  }
}

internal Range2
wm_rect_from_window(WM_Window handle) {
  Range2 result = {0};
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    RECT rect = {0};
    GetWindowRect(window->hwnd, &rect);
    result = w32_wm_range2_from_rect(rect);
  }
  return(result);
}

internal Range2
wm_client_rect_from_window(WM_Window handle) {
  Range2 result = {0};
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    RECT rect = {0};
    GetClientRect(window->hwnd, &rect);
    result = w32_wm_range2_from_rect(rect);
  }
  return(result);
}

internal f32
wm_dpi_from_window(WM_Window handle) {
  f32 result = 96.0f;
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    result = window->dpi;
  }
  return(result);
}

////////////////////////////////
// NOTE: @per_os_impl Events

internal void
wm_send_wakeup_event(void) {
  PostThreadMessageW(w32_wm_state->gfx_thread_tid, 0x401, 0, 0);
}

internal WM_Event_List
wm_get_events(Arena *arena, b32 wait) {
  w32_wm_event_arena = arena;
  memory_zero_struct(&w32_wm_event_list);
  MSG msg = {0};
  if (!wait || GetMessageW(&msg, 0, 0, 0)) {
    b32 first_wait = wait;
    for (;first_wait || PeekMessageW(&msg, 0, 0, 0, PM_REMOVE); first_wait = 0) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
      if (msg.message == WM_QUIT) {
        w32_wm_push_event(WM_Event_Type_WINDOW_CLOSE, 0);
      }
    }
  }
  return(w32_wm_event_list);
}

internal WM_Modifiers
wm_get_modifiers(void) {
  WM_Modifiers modifiers = 0;
  if (GetKeyState(VK_CONTROL) & 0x8000) {
    modifiers |= WM_Modifier_CTRL;
  }
  if (GetKeyState(VK_SHIFT) & 0x8000) {
    modifiers |= WM_Modifier_SHIFT;
  }
  if (GetKeyState(VK_MENU) & 0x8000) {
    modifiers |= WM_Modifier_ALT;
  }
  return(modifiers);
}

internal Vector2
wm_mouse_from_window(WM_Window handle) {
  Vector2 result = {0};
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(window->hwnd, &p);
    result.x = (f32)p.x;
    result.y = (f32)p.y;
  }
  return(result);
}

////////////////////////////////
// NOTE: @per_os_impl Cursors

internal void
wm_set_cursor(WM_Cursor cursor) {
  b32 valid_cursor = 1;
  HCURSOR hcursor = 0;
  switch (cursor) {
    default: {valid_cursor = 0;} break;
    case WM_Cursor_NONE: {
      hcursor = 0;
    } break;
    case WM_Cursor_POINTER: {
      hcursor = LoadCursor(0, IDC_ARROW);
    } break;
  }
  if (valid_cursor && !w32_wm_resizing) {
    if (hcursor != w32_wm_state->hCursor) {
      PostMessageW(0, WM_SETCURSOR, 0, 0);
      POINT p = {0};
      GetCursorPos(&p);
      SetCursorPos(p.x, p.y);
    }
    w32_wm_state->hCursor = hcursor;
  }
}

////////////////////////////////
// NOTE: @per_os_impl Native User-Facing Graphical Messages

internal void
wm_graphical_message(b32 error, String8 title, String8 message) {
  Temp scratch = scratch_begin(0, 0);
  String16 title16 = str16_from_8(scratch.arena, title);
  String16 message16 = str16_from_8(scratch.arena, message);
  MessageBoxW(0, (WCHAR *)message16.str, (WCHAR *)title16.str, MB_OK|(!!error*MB_ICONERROR));
  scratch_end(scratch);
}
