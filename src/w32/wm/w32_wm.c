////////////////////////////////
// NOTE: Helpers

internal Range2
w32_wm_range2_from_rect(RECT rect) {
  Range2 result = {0};
  result.x0 = (f32)rect.left;
  result.y0 = (f32)rect.top;
  result.x1 = (f32)rect.right;
  result.y1 = (f32)rect.bottom;
  return(result);
}

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

internal W32_WM_Window *
w32_wm_window_alloc(void) {
  W32_WM_Window *window = w32_wm_state->free_window;
  if (window != 0) {
    sll_stack_pop(w32_wm_state->free_window);
  } else {
    window = push_array_no_zero(w32_wm_state->arena, W32_WM_Window, 1);
  }
  memory_zero_struct(window);
  dll_push_back(w32_wm_state->first_window, w32_wm_state->last_window, window);
  window->last_placement.length = sizeof(WINDOWPLACEMENT);
  return(window);
}

internal void
w32_wm_window_release(W32_WM_Window *window) {
  dll_remove(w32_wm_state->first_window, w32_wm_state->last_window, window);
  sll_stack_push(w32_wm_state->free_window, window);
}

internal WM_Key
w32_wm_key_from_vkey(WPARAM vkey) {
  WM_Key result = WM_Key_NULL;
  {
    local b32 is_initialized = 1;
    local WM_Key key_table[256];
    if (!is_initialized){
      is_initialized = 1;
      memory_zero_array(key_table);

      key_table[(unsigned int)'A'] = WM_Key_A;
      key_table[(unsigned int)'B'] = WM_Key_B;
      key_table[(unsigned int)'C'] = WM_Key_C;
      key_table[(unsigned int)'D'] = WM_Key_D;
      key_table[(unsigned int)'E'] = WM_Key_E;
      key_table[(unsigned int)'F'] = WM_Key_F;
      key_table[(unsigned int)'G'] = WM_Key_G;
      key_table[(unsigned int)'H'] = WM_Key_H;
      key_table[(unsigned int)'I'] = WM_Key_I;
      key_table[(unsigned int)'J'] = WM_Key_J;
      key_table[(unsigned int)'K'] = WM_Key_K;
      key_table[(unsigned int)'L'] = WM_Key_L;
      key_table[(unsigned int)'M'] = WM_Key_M;
      key_table[(unsigned int)'N'] = WM_Key_N;
      key_table[(unsigned int)'O'] = WM_Key_O;
      key_table[(unsigned int)'P'] = WM_Key_P;
      key_table[(unsigned int)'Q'] = WM_Key_Q;
      key_table[(unsigned int)'R'] = WM_Key_R;
      key_table[(unsigned int)'S'] = WM_Key_S;
      key_table[(unsigned int)'T'] = WM_Key_T;
      key_table[(unsigned int)'U'] = WM_Key_U;
      key_table[(unsigned int)'V'] = WM_Key_V;
      key_table[(unsigned int)'W'] = WM_Key_W;
      key_table[(unsigned int)'X'] = WM_Key_X;
      key_table[(unsigned int)'Y'] = WM_Key_Y;
      key_table[(unsigned int)'Z'] = WM_Key_Z;

      for (u64 i = '0', j = WM_Key_0; i <= '9'; i += 1, j += 1){
        key_table[i] = (WM_Key)j;
      }
      for (u64 i = VK_NUMPAD0, j = WM_Key_0; i <= VK_NUMPAD9; i += 1, j += 1){
        key_table[i] = (WM_Key)j;
      }
      for (u64 i = VK_F1, j = WM_Key_F1; i <= VK_F24; i += 1, j += 1){
        key_table[i] = (WM_Key)j;
      }

      key_table[VK_SPACE]     = WM_Key_SPACE;
      key_table[VK_OEM_3]     = WM_Key_TICK;
      key_table[VK_OEM_MINUS] = WM_Key_MINUS;
      key_table[VK_OEM_PLUS]  = WM_Key_EQUAL;
      key_table[VK_OEM_4]     = WM_Key_LEFT_BRACKET;
      key_table[VK_OEM_6]     = WM_Key_RIGHT_BRACKET;
      key_table[VK_OEM_1]     = WM_Key_SEMICOLON;
      key_table[VK_OEM_7]     = WM_Key_QUOTE;
      key_table[VK_OEM_COMMA] = WM_Key_COMMA;
      key_table[VK_OEM_PERIOD]= WM_Key_PERIOD;
      key_table[VK_OEM_2]     = WM_Key_SLASH;
      key_table[VK_OEM_5]     = WM_Key_BACKSLASH;

      key_table[VK_TAB]       = WM_Key_TAB;
      key_table[VK_PAUSE]     = WM_Key_PAUSE;
      key_table[VK_ESCAPE]    = WM_Key_ESC;

      key_table[VK_UP]        = WM_Key_UP;
      key_table[VK_LEFT]      = WM_Key_LEFT;
      key_table[VK_DOWN]      = WM_Key_DOWN;
      key_table[VK_RIGHT]     = WM_Key_RIGHT;

      key_table[VK_BACK]      = WM_Key_BACKSPACE;
      key_table[VK_RETURN]    = WM_Key_RETURN;

      key_table[VK_DELETE]    = WM_Key_DELETE;
      key_table[VK_INSERT]    = WM_Key_INSERT;
      key_table[VK_PRIOR]     = WM_Key_PAGE_UP;
      key_table[VK_NEXT]      = WM_Key_PAGE_DOWN;
      key_table[VK_HOME]      = WM_Key_HOME;
      key_table[VK_END]       = WM_Key_END;

      key_table[VK_CAPITAL]   = WM_Key_CAPS_LOCK;
      key_table[VK_NUMLOCK]   = WM_Key_NUM_LOCK;
      key_table[VK_SCROLL]    = WM_Key_SCROLL_LOCK;
      key_table[VK_APPS]      = WM_Key_MENU;

      key_table[VK_CONTROL]   = WM_Key_CTRL;
      key_table[VK_LCONTROL]  = WM_Key_CTRL;
      key_table[VK_RCONTROL]  = WM_Key_CTRL;
      key_table[VK_SHIFT]     = WM_Key_SHIFT;
      key_table[VK_LSHIFT]    = WM_Key_SHIFT;
      key_table[VK_RSHIFT]    = WM_Key_SHIFT;
      key_table[VK_MENU]      = WM_Key_ALT;
      key_table[VK_LMENU]     = WM_Key_ALT;
      key_table[VK_RMENU]     = WM_Key_ALT;

      key_table[VK_DIVIDE]   = WM_Key_NUM_SLASH;
      key_table[VK_MULTIPLY] = WM_Key_NUM_STAR;
      key_table[VK_SUBTRACT] = WM_Key_NUM_MINUS;
      key_table[VK_ADD]      = WM_Key_NUM_PLUS;
      key_table[VK_DECIMAL]  = WM_Key_NUM_PERIOD;

      for (u32 i = 0; i < 10; i += 1){
        key_table[VK_NUMPAD0 + i] = (WM_Key)((u64)WM_Key_NUM_0 + i);
      }

      for (u64 i = 0xDF, j = 0; i < 0xFF; i += 1, j += 1){
        key_table[i] = (WM_Key)((u64)WM_Key_EX0 + j);
      }
    }
    result = key_table[vkey&bitmask8];
  }
  return(result);
}

internal WM_Key
w32_wm_vkey_from_key(WM_Key key) {
  WPARAM result = 0;
  {
    local b32 is_initialized = 0;
    local WPARAM vkey_table[WM_Key_COUNT] = {0};
    if (!is_initialized) {
      is_initialized = 1;
      vkey_table[WM_Key_ESC] = VK_ESCAPE;
      for (WM_Key k = WM_Key_F1; k <= WM_Key_F24; k = (WM_Key)(k+1)) {
        vkey_table[k] = VK_F1+(k-WM_Key_F1);
      }
      vkey_table[WM_Key_TICK] = VK_OEM_3;
      for (WM_Key k = WM_Key_0; k <= WM_Key_9; k = (WM_Key)(k+1)) {
        vkey_table[k] = '0'+(k-WM_Key_0);
      }
      vkey_table[WM_Key_MINUS] = VK_OEM_MINUS;
      vkey_table[WM_Key_EQUAL] = VK_OEM_PLUS;
      vkey_table[WM_Key_BACKSPACE] = VK_BACK;
      vkey_table[WM_Key_TAB] = VK_TAB;
      vkey_table[WM_Key_Q] = 'Q';
      vkey_table[WM_Key_W] = 'W';
      vkey_table[WM_Key_E] = 'E';
      vkey_table[WM_Key_R] = 'R';
      vkey_table[WM_Key_T] = 'T';
      vkey_table[WM_Key_Y] = 'Y';
      vkey_table[WM_Key_U] = 'U';
      vkey_table[WM_Key_I] = 'I';
      vkey_table[WM_Key_O] = 'O';
      vkey_table[WM_Key_P] = 'P';
      vkey_table[WM_Key_LEFT_BRACKET] = VK_OEM_4;
      vkey_table[WM_Key_RIGHT_BRACKET] = VK_OEM_6;
      vkey_table[WM_Key_BACKSLASH] = VK_OEM_5;
      vkey_table[WM_Key_CAPS_LOCK] = VK_CAPITAL;
      vkey_table[WM_Key_A] = 'A';
      vkey_table[WM_Key_S] = 'S';
      vkey_table[WM_Key_D] = 'D';
      vkey_table[WM_Key_F] = 'F';
      vkey_table[WM_Key_G] = 'G';
      vkey_table[WM_Key_H] = 'H';
      vkey_table[WM_Key_J] = 'J';
      vkey_table[WM_Key_K] = 'K';
      vkey_table[WM_Key_L] = 'L';
      vkey_table[WM_Key_SEMICOLON] = VK_OEM_1;
      vkey_table[WM_Key_QUOTE] = VK_OEM_7;
      vkey_table[WM_Key_RETURN] = VK_RETURN;
      vkey_table[WM_Key_SHIFT] = VK_SHIFT;
      vkey_table[WM_Key_Z] = 'Z';
      vkey_table[WM_Key_X] = 'X';
      vkey_table[WM_Key_C] = 'C';
      vkey_table[WM_Key_V] = 'V';
      vkey_table[WM_Key_B] = 'B';
      vkey_table[WM_Key_N] = 'N';
      vkey_table[WM_Key_M] = 'M';
      vkey_table[WM_Key_COMMA] = VK_OEM_COMMA;
      vkey_table[WM_Key_PERIOD] = VK_OEM_PERIOD;
      vkey_table[WM_Key_SLASH] = VK_OEM_2;
      vkey_table[WM_Key_CTRL] = VK_CONTROL;
      vkey_table[WM_Key_ALT] = VK_MENU;
      vkey_table[WM_Key_SPACE] = VK_SPACE;
      vkey_table[WM_Key_MENU] = VK_APPS;
      vkey_table[WM_Key_SCROLL_LOCK] = VK_SCROLL;
      vkey_table[WM_Key_PAUSE] = VK_PAUSE;
      vkey_table[WM_Key_INSERT] = VK_INSERT;
      vkey_table[WM_Key_HOME] = VK_HOME;
      vkey_table[WM_Key_PAGE_UP] = VK_PRIOR;
      vkey_table[WM_Key_DELETE] = VK_DELETE;
      vkey_table[WM_Key_END] = VK_END;
      vkey_table[WM_Key_PAGE_DOWN] = VK_NEXT;
      vkey_table[WM_Key_UP] = VK_UP;
      vkey_table[WM_Key_LEFT] = VK_LEFT;
      vkey_table[WM_Key_DOWN] = VK_DOWN;
      vkey_table[WM_Key_RIGHT] = VK_RIGHT;
      for (WM_Key k = WM_Key_EX0; k <= WM_Key_EX29; k = (WM_Key)(k+1)) {
        vkey_table[k] = 0xDF + (k-WM_Key_EX0);
      }
      vkey_table[WM_Key_NUM_LOCK] = VK_NUMLOCK;
      vkey_table[WM_Key_NUM_SLASH] = VK_DIVIDE;
      vkey_table[WM_Key_NUM_STAR] = VK_MULTIPLY;
      vkey_table[WM_Key_NUM_MINUS] = VK_SUBTRACT;
      vkey_table[WM_Key_NUM_PLUS] = VK_ADD;
      vkey_table[WM_Key_NUM_PERIOD] = VK_DECIMAL;
      vkey_table[WM_Key_NUM_0] = VK_NUMPAD0;
      vkey_table[WM_Key_NUM_1] = VK_NUMPAD1;
      vkey_table[WM_Key_NUM_2] = VK_NUMPAD2;
      vkey_table[WM_Key_NUM_3] = VK_NUMPAD3;
      vkey_table[WM_Key_NUM_4] = VK_NUMPAD4;
      vkey_table[WM_Key_NUM_5] = VK_NUMPAD5;
      vkey_table[WM_Key_NUM_6] = VK_NUMPAD6;
      vkey_table[WM_Key_NUM_7] = VK_NUMPAD7;
      vkey_table[WM_Key_NUM_8] = VK_NUMPAD8;
      vkey_table[WM_Key_NUM_9] = VK_NUMPAD9;
      vkey_table[WM_Key_LEFT_MOUSE_BUTTON] = VK_LBUTTON;
      vkey_table[WM_Key_MIDDLE_MOUSE_BUTTON] = VK_MBUTTON;
      vkey_table[WM_Key_RIGHT_MOUSE_BUTTON] = VK_RBUTTON;
    }
    result = vkey_table[key];
  }
  return(result);
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
          (wParam == VK_SHIFT   || wParam == VK_RSHIFT ||
           wParam == VK_CONTROL || wParam == VK_RCONTROL ||
           wParam == VK_MENU    || wParam == VK_RMENU)) {
        is_right_sided = 1;
      }

      WM_Event *event = w32_wm_push_event(release ? WM_Event_Type_RELEASE : WM_Event_Type_PRESS, window);
      event->key = w32_wm_state->key_from_vkey_table[wParam&bitmask8];
      event->is_repeat = is_repeat;
      event->is_right_sided = is_right_sided;
      event->repeat_count = lParam & bitmask16;
      if (event->key == WM_Key_SHIFT && event->modifiers & WM_Modifier_SHIFT) {event->modifiers &= ~WM_Modifier_SHIFT;}
      if (event->key == WM_Key_CTRL  && event->modifiers & WM_Modifier_CTRL)  {event->modifiers &= ~WM_Modifier_CTRL;}
      if (event->key == WM_Key_ALT   && event->modifiers & WM_Modifier_ALT)   {event->modifiers &= ~WM_Modifier_ALT;}
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
        case WM_LBUTTONUP: case WM_LBUTTONDOWN: {event->key = WM_Key_LEFT_MOUSE_BUTTON;}   break;
        case WM_MBUTTONUP: case WM_MBUTTONDOWN: {event->key = WM_Key_MIDDLE_MOUSE_BUTTON;} break;
        case WM_RBUTTONUP: case WM_RBUTTONDOWN: {event->key = WM_Key_RIGHT_MOUSE_BUTTON;}  break;
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
      WM_Event *event = w32_wm_push_event(WM_Event_Type_SCROLL, window);
      POINT p;
      p.x = (s32)(s16)LOWORD(lParam);
      p.y = (s32)(s16)HIWORD(lParam);
      ScreenToClient(window->hwnd, &p);
      event->position.x = (f32)p.x;
      event->position.y = (f32)p.y;
      event->delta = vector2_make(0.0f, -(f32)wheel_delta/WHEEL_DELTA);
    } break;

    case WM_MOUSEHWHEEL: {
      s16 wheel_delta = HIWORD(wParam);
      WM_Event *event = w32_wm_push_event(WM_Event_Type_SCROLL, window);
      POINT p;
      p.x = (s32)(s16)LOWORD(lParam);
      p.y = (s32)(s16)HIWORD(lParam);
      ScreenToClient(window->hwnd, &p);
      event->position.x = (f32)p.x;
      event->position.y = (f32)p.y;
      event->delta = vector2_make((f32)wheel_delta/WHEEL_DELTA, 0.0f);
    } break;

    case WM_SETCURSOR: {
      Range2 client_rect = wm_window_get_client_rect(window_handle);
      Vector2 mouse = wm_mouse_from_window(window_handle);
      if (range2_contains(client_rect, mouse)) {
        SetCursor(w32_wm_state->cursors[w32_wm_state->last_set_cursor]);
      } else {
        result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
      }
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
        WM_Event *event = w32_wm_push_event(WM_Event_Type_TEXT, window);
        if (lParam & bit29) {
          event->modifiers |= WM_Modifier_ALT;
        }
        event->character = character;
      }
    } break;

    case WM_SIZE:
    case WM_PAINT: {
      PAINTSTRUCT ps = {0};
      BeginPaint(hwnd, &ps);
      frame();
      EndPaint(hwnd, &ps);
    } break;

    case WM_CLOSE: {
      w32_wm_push_event(WM_Event_Type_WINDOW_CLOSE, window);
    } break;
  }

  return(result);
}

////////////////////////////////
// NOTE: @per_os_impl Main Initialization

internal void
wm_init(void) {
  Arena *arena = arena_alloc();
  w32_wm_state = push_array(arena, W32_WM_State, 1);
  w32_wm_state->arena = arena;
  w32_wm_state->hInstance = GetModuleHandle(0);

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

  {
    DEVMODEW devmodew = {0};
    if (EnumDisplaySettingsW(0, ENUM_CURRENT_SETTINGS, &devmodew)) {
      w32_wm_state->system_info.refresh_rate = (f32)devmodew.dmDisplayFrequency;
    }
  }

  {
    struct {
      WM_Cursor cursor;
      LPCSTR id;
    } map[] = {
      {WM_Cursor_ARROW, IDC_ARROW},
      {WM_Cursor_IBEAM, IDC_IBEAM},
      {WM_Cursor_HAND, IDC_HAND},
    };
    for_each_element(idx, map) {
      w32_wm_state->cursors[map[idx].cursor] = LoadCursor(0, map[idx].id);
    }
  }

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
    w32_wm_state->key_from_vkey_table[VK_OEM_4]     = WM_Key_LEFT_BRACKET;
    w32_wm_state->key_from_vkey_table[VK_OEM_6]     = WM_Key_RIGHT_BRACKET;
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
    w32_wm_state->key_from_vkey_table[VK_PRIOR]     = WM_Key_PAGE_UP;
    w32_wm_state->key_from_vkey_table[VK_NEXT]      = WM_Key_PAGE_DOWN;
    w32_wm_state->key_from_vkey_table[VK_HOME]      = WM_Key_HOME;
    w32_wm_state->key_from_vkey_table[VK_END]       = WM_Key_END;

    w32_wm_state->key_from_vkey_table[VK_CAPITAL]   = WM_Key_CAPS_LOCK;
    w32_wm_state->key_from_vkey_table[VK_NUMLOCK]   = WM_Key_NUM_LOCK;
    w32_wm_state->key_from_vkey_table[VK_SCROLL]    = WM_Key_SCROLL_LOCK;
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

    w32_wm_state->key_from_vkey_table[VK_DIVIDE]   = WM_Key_NUM_SLASH;
    w32_wm_state->key_from_vkey_table[VK_MULTIPLY] = WM_Key_NUM_STAR;
    w32_wm_state->key_from_vkey_table[VK_SUBTRACT] = WM_Key_NUM_MINUS;
    w32_wm_state->key_from_vkey_table[VK_ADD]      = WM_Key_NUM_PLUS;
    w32_wm_state->key_from_vkey_table[VK_DECIMAL]  = WM_Key_NUM_PERIOD;

    for (u32 i = 0; i < 10; i += 1) {
      w32_wm_state->key_from_vkey_table[VK_NUMPAD0 + i] = (WM_Key)((u32)WM_Key_NUM_0 + i);
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
wm_window_open(String8 name, Vector2 size) {
  Temp scratch = scratch_begin(0, 0);

  W32_WM_Window *w = w32_wm_window_alloc();
  String16 name16 = str16_from_8(scratch.arena, name);

  DWORD ex_style_flags = WS_EX_APPWINDOW; // | WS_EX_NOREDIRECTIONBITMAP;
  DWORD style_flags = WS_OVERLAPPEDWINDOW;

  {
    RECT rect = {0, 0, (LONG)size.x, (LONG)size.y};
    if (AdjustWindowRectEx(&rect, style_flags, 0, ex_style_flags)) {
      size.x = (f32)(rect.right - rect.left);
      size.y = (f32)(rect.bottom - rect.top);
    }
  }

  s32 monitor_w = GetSystemMetrics(SM_CXSCREEN);
  s32 monitor_h = GetSystemMetrics(SM_CYSCREEN);

  s32 window_w = (s32)size.x;
  s32 window_h = (s32)size.y;

  s32 window_x = (monitor_w - window_w)/2;
  s32 window_y = (monitor_h - window_h)/2;

  w->hwnd = CreateWindowExW(ex_style_flags,
                            L"graphical-window",
                            (WCHAR *)name16.str,
                            style_flags,
                            window_x, window_y,
                            window_w, window_h,
                            0, 0,
                            w32_wm_state->hInstance,
                            0);
  w->hdc = GetDC(w->hwnd);

  WM_Window result = w32_wm_handle_from_window(w);
  scratch_end(scratch);
  return(result);
}

internal void
wm_window_close(WM_Window handle) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    ReleaseDC(window->hwnd, window->hdc);
    DestroyWindow(window->hwnd);
    w32_wm_window_release(window);
  }
}

internal void
wm_window_first_paint(WM_Window handle) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    window->first_paint_done = 1;
    ShowWindow(window->hwnd, SW_SHOW);
    if (window->maximized) {
      ShowWindow(window->hwnd, SW_MAXIMIZE);
    }
  }
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

internal b32
wm_window_is_maximized(WM_Window handle) {
  b32 result = 0;
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    result = !!(IsZoomed(window->hwnd));
  }
  return(result);
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
wm_window_set_name(WM_Window handle, String8 name) {
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    Temp scratch = scratch_begin(0, 0);
    String16 name16 = str16_from_8(scratch.arena, name);
    SetWindowTextW(window->hwnd, (WCHAR *)name16.str);
    scratch_end(scratch);
  }
}

internal Range2
wm_window_get_rect(WM_Window handle) {
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
wm_window_get_client_rect(WM_Window handle) {
  Range2 result = {0};
  W32_WM_Window *window = w32_wm_window_from_handle(handle);
  if (window != 0) {
    RECT rect = {0};
    GetClientRect(window->hwnd, &rect);
    result = w32_wm_range2_from_rect(rect);
  }
  return(result);
}

////////////////////////////////
// NOTE: @per_os_impl Events

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
  WM_Modifiers result = 0;
  if (wm_key_is_down(WM_Key_SHIFT)) {result |= WM_Modifier_SHIFT;}
  if (wm_key_is_down(WM_Key_CTRL))  {result |= WM_Modifier_CTRL;}
  if (wm_key_is_down(WM_Key_ALT))   {result |= WM_Modifier_ALT;}
  return(result);
}

internal b32
wm_key_is_down(WM_Key key) {
  b32 result = 0;
  WPARAM vkey = w32_wm_vkey_from_key(key);
  if (GetKeyState((int)vkey) & 0x8000) {
    result = 1;
  }
  return(result);
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
  w32_wm_state->last_set_cursor = cursor;
  PostMessageW(0, WM_SETCURSOR, 0, 0);
  POINT p = {0};
  GetCursorPos(&p);
  SetCursorPos(p.x, p.y);
}
