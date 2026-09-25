////////////////////////////////
// NOTE: Helpers

internal WM_Window
lnx_wm_handle_from_window(LNX_WM_Window *window) {
  WM_Window result = {0};
  result.v[0] = (u64)window;
  return(result);
}

internal LNX_WM_Window *
lnx_wm_window_from_handle(WM_Window handle) {
  LNX_WM_Window *result = (LNX_WM_Window *)handle.v[0];
  return(result);
}

internal LNX_WM_Window *
lnx_wm_window_from_x11_window(Window window) {
  LNX_WM_Window *result = 0;
  for_each_node(LNX_WM_Window, w, lnx_wm_state->first_window) {
    if (w->window == window) {
      result = w;
      break;
    }
  }
  return(result);
}

internal LNX_WM_Window *
lnx_wm_window_alloc(void) {
  LNX_WM_Window *window = lnx_wm_state->first_free_window;
  if (window != 0) {
    sll_stack_pop(lnx_wm_state->first_free_window);
  } else {
    window = push_array_no_zero(lnx_wm_state->arena, LNX_WM_Window, 1);
  }
  memory_zero_struct(window);
  dll_push_back(lnx_wm_state->first_window, lnx_wm_state->last_window, window);
  return(window);
}

internal void
lnx_wm_window_release(LNX_WM_Window *window) {
  dll_remove(lnx_wm_state->first_window, lnx_wm_state->last_window, window);
  sll_stack_push(lnx_wm_state->first_free_window, window);
}

internal WM_Key
lnx_wm_key_from_keysym(KeySym ks, b32 *out_is_right_sided) {
  WM_Key key = WM_Key_NULL;
  b32 is_right_sided = 0;
  switch (ks){
    default: {
      if(0) {}
      else if (XK_F1 <= ks && ks <= XK_F24) {key = (WM_Key)(WM_Key_F1 + (ks - XK_F1));}
      else if ('0' <= ks && ks <= '9')      {key = WM_Key_0 + (ks - '0');}
    } break;
    case XK_Escape:{key = WM_Key_ESC;};break;
    case XK_BackSpace:{key = WM_Key_BACKSPACE;}break;
    case XK_Delete:{key = WM_Key_DELETE;}break;
    case XK_Return:{key = WM_Key_RETURN;}break;
    case XK_Pause:{key = WM_Key_PAUSE;}break;
    case XK_Tab:{key = WM_Key_TAB;}break;
    case XK_Left:{key = WM_Key_LEFT;}break;
    case XK_Right:{key = WM_Key_RIGHT;}break;
    case XK_Up:{key = WM_Key_UP;}break;
    case XK_Down:{key = WM_Key_DOWN;}break;
    case XK_Home:{key = WM_Key_HOME;}break;
    case XK_End:{key = WM_Key_END;}break;
    case XK_Page_Up:{key = WM_Key_PAGE_UP;}break;
    case XK_Page_Down:{key = WM_Key_PAGE_DOWN;}break;
    case XK_Alt_L:{ key = WM_Key_ALT; }break;
    case XK_Alt_R:{ key = WM_Key_ALT; is_right_sided = 1;}break;
    case XK_Shift_L:{ key = WM_Key_SHIFT; }break;
    case XK_Shift_R:{ key = WM_Key_SHIFT; is_right_sided = 1;}break;
    case XK_Control_L:{ key = WM_Key_CTRL; }break;
    case XK_Control_R:{ key = WM_Key_CTRL; is_right_sided = 1;}break;
    case XK_Caps_Lock:{key = WM_Key_CAPS_LOCK;}break;
    case XK_Menu:{key = WM_Key_MENU;}break;
    case XK_Scroll_Lock:{key = WM_Key_SCROLL_LOCK;}break;
    case XK_Insert:{key = WM_Key_INSERT;}break;
    case XK_Num_Lock:{key = WM_Key_NUM_LOCK;}break;
    case XK_KP_Divide:{key = WM_Key_NUM_SLASH;}break;
    case XK_KP_Multiply:{key = WM_Key_NUM_STAR;}break;
    case XK_KP_Subtract:{key = WM_Key_NUM_MINUS;}break;
    case XK_KP_Add:{key = WM_Key_NUM_PLUS;}break;
    case XK_KP_Decimal:{key = WM_Key_NUM_PERIOD;}break;
    case XK_KP_0:{key = WM_Key_NUM_0;}break;
    case XK_KP_1:{key = WM_Key_NUM_1;}break;
    case XK_KP_2:{key = WM_Key_NUM_2;}break;
    case XK_KP_3:{key = WM_Key_NUM_3;}break;
    case XK_KP_4:{key = WM_Key_NUM_4;}break;
    case XK_KP_5:{key = WM_Key_NUM_5;}break;
    case XK_KP_6:{key = WM_Key_NUM_6;}break;
    case XK_KP_7:{key = WM_Key_NUM_7;}break;
    case XK_KP_8:{key = WM_Key_NUM_8;}break;
    case XK_KP_9:{key = WM_Key_NUM_9;}break;
    case '-':{key = WM_Key_MINUS;}break;
    case '=':{key = WM_Key_EQUAL;}break;
    case '[':{key = WM_Key_LEFT_BRACKET;}break;
    case ']':{key = WM_Key_RIGHT_BRACKET;}break;
    case ';':{key = WM_Key_SEMICOLON;}break;
    case '\'':{key = WM_Key_QUOTE;}break;
    case '.':{key = WM_Key_PERIOD;}break;
    case ',':{key = WM_Key_COMMA;}break;
    case '/':{key = WM_Key_SLASH;}break;
    case '\\':{key = WM_Key_BACKSLASH;}break;
    case '\t':{key = WM_Key_TAB;}break;
    case 'a':case 'A':{key = WM_Key_A;}break;
    case 'b':case 'B':{key = WM_Key_B;}break;
    case 'c':case 'C':{key = WM_Key_C;}break;
    case 'd':case 'D':{key = WM_Key_D;}break;
    case 'e':case 'E':{key = WM_Key_E;}break;
    case 'f':case 'F':{key = WM_Key_F;}break;
    case 'g':case 'G':{key = WM_Key_G;}break;
    case 'h':case 'H':{key = WM_Key_H;}break;
    case 'i':case 'I':{key = WM_Key_I;}break;
    case 'j':case 'J':{key = WM_Key_J;}break;
    case 'k':case 'K':{key = WM_Key_K;}break;
    case 'l':case 'L':{key = WM_Key_L;}break;
    case 'm':case 'M':{key = WM_Key_M;}break;
    case 'n':case 'N':{key = WM_Key_N;}break;
    case 'o':case 'O':{key = WM_Key_O;}break;
    case 'p':case 'P':{key = WM_Key_P;}break;
    case 'q':case 'Q':{key = WM_Key_Q;}break;
    case 'r':case 'R':{key = WM_Key_R;}break;
    case 's':case 'S':{key = WM_Key_S;}break;
    case 't':case 'T':{key = WM_Key_T;}break;
    case 'u':case 'U':{key = WM_Key_U;}break;
    case 'v':case 'V':{key = WM_Key_V;}break;
    case 'w':case 'W':{key = WM_Key_W;}break;
    case 'x':case 'X':{key = WM_Key_X;}break;
    case 'y':case 'Y':{key = WM_Key_Y;}break;
    case 'z':case 'Z':{key = WM_Key_Z;}break;
    case ' ':{key = WM_Key_SPACE;}break;
    case '`':{key = WM_Key_TICK;}break;
  }
  if (out_is_right_sided) {
    out_is_right_sided[0] = is_right_sided;
  }
  return(key);
}

internal KeySym
lnx_wm_keysym_from_key(WM_Key key) {
  KeySym ks = 0;
  switch(key) {
    case WM_Key_ESC:{ks = XK_Escape;}break;
    case WM_Key_F1:{ks = XK_F1;}break;
    case WM_Key_F2:{ks = XK_F2;}break;
    case WM_Key_F3:{ks = XK_F3;}break;
    case WM_Key_F4:{ks = XK_F4;}break;
    case WM_Key_F5:{ks = XK_F5;}break;
    case WM_Key_F6:{ks = XK_F6;}break;
    case WM_Key_F7:{ks = XK_F7;}break;
    case WM_Key_F8:{ks = XK_F8;}break;
    case WM_Key_F9:{ks = XK_F9;}break;
    case WM_Key_F10:{ks = XK_F10;}break;
    case WM_Key_F11:{ks = XK_F11;}break;
    case WM_Key_F12:{ks = XK_F12;}break;
    case WM_Key_F13:{ks = XK_F13;}break;
    case WM_Key_F14:{ks = XK_F14;}break;
    case WM_Key_F15:{ks = XK_F15;}break;
    case WM_Key_F16:{ks = XK_F16;}break;
    case WM_Key_F17:{ks = XK_F17;}break;
    case WM_Key_F18:{ks = XK_F18;}break;
    case WM_Key_F19:{ks = XK_F19;}break;
    case WM_Key_F20:{ks = XK_F20;}break;
    case WM_Key_F21:{ks = XK_F21;}break;
    case WM_Key_F22:{ks = XK_F22;}break;
    case WM_Key_F23:{ks = XK_F23;}break;
    case WM_Key_F24:{ks = XK_F24;}break;
    case WM_Key_TICK:{ks = '`';}break;
    case WM_Key_0:{ks = XK_0;}break;
    case WM_Key_1:{ks = XK_1;}break;
    case WM_Key_2:{ks = XK_2;}break;
    case WM_Key_3:{ks = XK_3;}break;
    case WM_Key_4:{ks = XK_4;}break;
    case WM_Key_5:{ks = XK_5;}break;
    case WM_Key_6:{ks = XK_6;}break;
    case WM_Key_7:{ks = XK_7;}break;
    case WM_Key_8:{ks = XK_8;}break;
    case WM_Key_9:{ks = XK_9;}break;
    case WM_Key_MINUS:{ks = '-';}break;
    case WM_Key_EQUAL:{ks = '=';}break;
    case WM_Key_BACKSPACE:{ks = XK_BackSpace;}break;
    case WM_Key_TAB:{ks = XK_Tab;}break;
    case WM_Key_Q:{ks = XK_Q;}break;
    case WM_Key_W:{ks = XK_W;}break;
    case WM_Key_E:{ks = XK_E;}break;
    case WM_Key_R:{ks = XK_R;}break;
    case WM_Key_T:{ks = XK_T;}break;
    case WM_Key_Y:{ks = XK_Y;}break;
    case WM_Key_U:{ks = XK_U;}break;
    case WM_Key_I:{ks = XK_I;}break;
    case WM_Key_O:{ks = XK_O;}break;
    case WM_Key_P:{ks = XK_P;}break;
    case WM_Key_LEFT_BRACKET:{ks = '[';}break;
    case WM_Key_RIGHT_BRACKET:{ks = ']';}break;
    case WM_Key_BACKSLASH:{ks = '\\';}break;
    case WM_Key_CAPS_LOCK:{ks = XK_Caps_Lock;}break;
    case WM_Key_A:{ks = XK_Alt_L;}break;
    case WM_Key_S:{ks = XK_Shift_L;}break;
    case WM_Key_D:{ks = XK_Delete;}break;
    case WM_Key_F:{ks = XK_F24;}break;
    case WM_Key_G:{ks = XK_G;}break;
    case WM_Key_H:{ks = XK_Home;}break;
    case WM_Key_J:{ks = XK_J;}break;
    case WM_Key_K:{ks = XK_K;}break;
    case WM_Key_L:{ks = XK_L;}break;
    case WM_Key_SEMICOLON:{ks = ';';}break;
    case WM_Key_QUOTE:{ks = '\'';}break;
    case WM_Key_RETURN:{ks = XK_Return;}break;
    case WM_Key_SHIFT:{ks = XK_Shift_L;}break;
    case WM_Key_Z:{ks = XK_Z;}break;
    case WM_Key_X:{ks = XK_X;}break;
    case WM_Key_C:{ks = XK_Caps_Lock;}break;
    case WM_Key_V:{ks = XK_V;}break;
    case WM_Key_B:{ks = XK_B;}break;
    case WM_Key_N:{ks = XK_N;}break;
    case WM_Key_M:{ks = '-';}break;
    case WM_Key_COMMA:{ks = ',';}break;
    case WM_Key_PERIOD:{ks = '.';}break;
    case WM_Key_SLASH:{ks = '/';}break;
    case WM_Key_CTRL:{ks = XK_Control_L;}break;
    case WM_Key_ALT:{ks = XK_Alt_L;}break;
    case WM_Key_SPACE:{ks = ' ';}break;
    case WM_Key_MENU:{ks = XK_Menu;}break;
    case WM_Key_SCROLL_LOCK:{ks = XK_Scroll_Lock;}break;
    case WM_Key_PAUSE:{ks = XK_Pause;}break;
    case WM_Key_INSERT:{ks = XK_Insert;}break;
    case WM_Key_HOME:{ks = XK_Home;}break;
    case WM_Key_PAGE_UP:{ks = XK_Page_Up;}break;
    case WM_Key_DELETE:{ks = XK_Delete;}break;
    case WM_Key_END:{ks = XK_End;}break;
    case WM_Key_PAGE_DOWN:{ks = XK_Page_Down;}break;
    case WM_Key_UP:{ks = XK_Up;}break;
    case WM_Key_LEFT:{ks = XK_Left;}break;
    case WM_Key_DOWN:{ks = XK_Down;}break;
    case WM_Key_RIGHT:{ks = XK_Right;}break;
    case WM_Key_NUM_LOCK:{ks = XK_Num_Lock;}break;
    case WM_Key_NUM_SLASH:{ks = XK_KP_Divide;}break;
    case WM_Key_NUM_STAR:{ks = XK_KP_Multiply;}break;
    case WM_Key_NUM_MINUS:{ks = XK_KP_Subtract;}break;
    case WM_Key_NUM_PLUS:{ks = XK_KP_Add;}break;
    case WM_Key_NUM_PERIOD:{ks = XK_KP_Decimal;}break;
    case WM_Key_NUM_0:{ks = XK_KP_0;}break;
    case WM_Key_NUM_1:{ks = XK_KP_1;}break;
    case WM_Key_NUM_2:{ks = XK_KP_2;}break;
    case WM_Key_NUM_3:{ks = XK_KP_3;}break;
    case WM_Key_NUM_4:{ks = XK_KP_4;}break;
    case WM_Key_NUM_5:{ks = XK_KP_5;}break;
    case WM_Key_NUM_6:{ks = XK_KP_6;}break;
    case WM_Key_NUM_7:{ks = XK_KP_7;}break;
    case WM_Key_NUM_8:{ks = XK_KP_8;}break;
    case WM_Key_NUM_9:{ks = XK_KP_9;}break;
  }
  return(ks);
}

////////////////////////////////
// NOTE: @per_os_impl Main Initialization

internal void
wm_init(void) {
  Arena *arena = arena_alloc();
  lnx_wm_state = push_array(arena, LNX_WM_State, 1);
  lnx_wm_state->arena = arena;
  lnx_wm_state->display = XOpenDisplay(0);

  lnx_wm_state->wm_delete_window_atom = XInternAtom(lnx_wm_state->display, "WM_DELETE_WINDOW", 0);
  lnx_wm_state->xim = XOpenIM(lnx_wm_state->display, 0, 0, 0);
  lnx_wm_state->wakeup_fd = eventfd(0, EFD_CLOEXEC);
  lnx_wm_state->system_info.refresh_rate = 60.0f;

  {
    struct {
      WM_Cursor cursor;
      String8 string;
    } map[] = {
      {WM_Cursor_ARROW, str8_lit_comp("left_ptr")},
      {WM_Cursor_IBEAM, str8_lit_comp("xterm")},
      {WM_Cursor_HAND, str8_lit_comp("hand2")},
    };
    for_each_element(idx, map) {
      lnx_wm_state->cursors[map[idx].cursor] = XcursorLibraryLoadCursor(lnx_wm_state->display, (char *)map[idx].string.str);
    }
  }
}

////////////////////////////////
// NOTE: @per_os_impl Graphics System Info

internal WM_System_Info *
wm_get_system_info(void) {
  return(&lnx_wm_state->system_info);
}

////////////////////////////////
// NOTE: @per_os_impl Windows

internal WM_Window
wm_window_open(String8 name, Vector2 size) {
  LNX_WM_Window *w = lnx_wm_window_alloc();
  s32 screen = DefaultScreen(lnx_wm_state->display);

  s32 monitor_w = DisplayWidth(lnx_wm_state->display, screen);
  s32 monitor_h = DisplayHeight(lnx_wm_state->display, screen);

  s32 window_w = (s32)size.x;
  s32 window_h = (s32)size.y;

  s32 window_x = (monitor_w - window_w)/2;
  s32 window_y = (monitor_w - window_h)/2;

  s32 attrs_mask = CWBackPixel|CWBorderPixel|CWColormap;
  XSetWindowAttributes attrs = {0};
  attrs.background_pixel = 0;
  attrs.border_pixel = 0;
  attrs.colormap = 0;

  w->window = XCreateWindow(lnx_wm_state->display,
                            XDefaultRootWindow(lnx_wm_state->display),
                            window_x, window_y,
                            window_w, window_h,
                            0,
                            CopyFromParent,
                            InputOutput,
                            CopyFromParent,
                            attrs_mask,
                            &attrs);
  XSelectInput(lnx_wm_state->display, w->window,
               ExposureMask|
               PointerMotionMask|
               ButtonPressMask|
               ButtonReleaseMask|
               KeyPressMask|
               KeyReleaseMask|
               StructureNotifyMask);
  XSetWMProtocols(lnx_wm_state->display, w->window, &lnx_wm_state->wm_delete_window_atom, 1);

  w->xic = XCreateIC(lnx_wm_state->xim,
                     XNInputStyle, XIMPreeditNothing|XIMStatusNothing,
                     XNClientWindow, w->window,
                     XNFocusWindow, w->window,
                     NULL);

  Temp scratch = scratch_begin(0, 0);
  String8 name_copy = str8_copy(scratch.arena, name);
  XStoreName(lnx_wm_state->display, w->window, (char *)name_copy.str);
  scratch_end(scratch);

  WM_Window result = lnx_wm_handle_from_window(w);
  return(result);
}

internal void
wm_window_close(WM_Window handle) {
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    XDestroyWindow(lnx_wm_state->display, window->window);
    lnx_wm_window_release(window);
  }
}

internal void
wm_window_first_paint(WM_Window handle) {
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    XMapWindow(lnx_wm_state->display, window->window);
    XFlush(lnx_wm_state->display);
  }
}

internal b32
wm_window_is_fullscreen(WM_Window handle) {
  b32 result = 0;
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    Atom wm_state_atom = XInternAtom(lnx_wm_state->display, "_NET_WM_STATE", 0);
    Atom wm_state_fullscreen_atom = XInternAtom(lnx_wm_state->display, "_NET_WM_STATE_FULLSCREEN", 0);

    unsigned long props_count = 0;
    Atom *props = 0;
    {
      Atom actual_type;
      int actual_format;
      unsigned long bytes_after;
      int result = XGetWindowProperty(lnx_wm_state->display,
                                      window->window,
                                      wm_state_atom,
                                      0,
                                      1024,
                                      0,
                                      AnyPropertyType,
                                      &actual_type,
                                      &actual_format,
                                      &props_count,
                                      &bytes_after,
                                      (unsigned char **)&props);
      (void)result;
    }

    for (unsigned long idx = 0; idx < props_count; idx += 1) {
      if (props[idx] == wm_state_fullscreen_atom) {
        result = 1;
        break;
      }
    }

    XFree(props);
  }
  return(result);
}

internal void
wm_window_set_fullscreen(WM_Window handle, b32 fullscreen) {
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    XEvent event;
    memory_zero_struct(&event);
    event.type = ClientMessage;
    event.xclient.display = lnx_wm_state->display;
    event.xclient.window = window->window;
    event.xclient.message_type = XInternAtom(lnx_wm_state->display, "_NET_WM_STATE", 0);
    event.xclient.format = 32;
    event.xclient.data.l[0] = !!fullscreen;
    event.xclient.data.l[1] = XInternAtom(lnx_wm_state->display, "_NET_WM_STATE_FULLSCREEN", 0);
    event.xclient.data.l[3] = 1;
    XSendEvent(lnx_wm_state->display, DefaultRootWindow(lnx_wm_state->display), 0, SubstructureNotifyMask|SubstructureRedirectMask, &event);
    XFlush(lnx_wm_state->display);
  }
}

internal b32
wm_window_is_maximized(WM_Window handle) {
  b32 result = 0;
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    Atom wm_state_atom = XInternAtom(lnx_wm_state->display, "_NET_WM_STATE", 0);
    Atom wm_state_max_x_atom = XInternAtom(lnx_wm_state->display, "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
    Atom wm_state_max_y_atom = XInternAtom(lnx_wm_state->display, "_NET_WM_STATE_MAXIMIZED_VERT", 0);

    unsigned long props_count = 0;
    Atom *props = 0;
    {
      Atom actual_type;
      int actual_format;
      unsigned long bytes_after;
      int result = XGetWindowProperty(lnx_wm_state->display,
                                      window->window,
                                      wm_state_atom,
                                      0,
                                      1024,
                                      0,
                                      AnyPropertyType,
                                      &actual_type,
                                      &actual_format,
                                      &props_count,
                                      &bytes_after,
                                      (unsigned char **)&props);
      (void)result;
    }

    b32 is_max_x_set = 0;
    b32 is_max_y_set = 0;
    for (unsigned long idx = 0; idx < props_count; idx += 1) {
      if (props[idx] == wm_state_max_x_atom) {is_max_x_set = 1;};
      if (props[idx] == wm_state_max_y_atom) {is_max_y_set = 1;};
    }

    XFree(props);
    result = (is_max_x_set && is_max_y_set);
  }
  return(result);
}

internal void
wm_window_set_maximized(WM_Window handle, b32 maximized) {
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    XEvent event;
    memory_zero_struct(&event);
    event.type = ClientMessage;
    event.xclient.display = lnx_wm_state->display;
    event.xclient.window = window->window;
    event.xclient.message_type = XInternAtom(lnx_wm_state->display, "_NET_WM_STATE", 0);
    event.xclient.format = 32;
    event.xclient.data.l[0] = !!maximized;
    event.xclient.data.l[1] = XInternAtom(lnx_wm_state->display, "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
    event.xclient.data.l[2] = XInternAtom(lnx_wm_state->display, "_NET_WM_STATE_MAXIMIZED_VERT", 0);
    event.xclient.data.l[3] = 1;
    XSendEvent(lnx_wm_state->display, DefaultRootWindow(lnx_wm_state->display), 0, SubstructureNotifyMask|SubstructureRedirectMask, &event);
    XFlush(lnx_wm_state->display);
  }
}

internal b32
wm_window_is_minimized(WM_Window handle) {
  b32 result = 0;
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    Atom wm_state = XInternAtom(lnx_wm_state->display, "WM_STATE", 0);

    unsigned long props_count = 0;
    u8 *prop_data = 0;
    {
      Atom actual_type;
      int actual_format;
      unsigned long bytes_after = 0;
      int result = XGetWindowProperty(lnx_wm_state->display,
                                      window->window,
                                      wm_state,
                                      0,
                                      1024,
                                      0,
                                      AnyPropertyType,
                                      &actual_type,
                                      &actual_format,
                                      &props_count,
                                      &bytes_after, 
                                      (unsigned char **)&prop_data);
    }

    if (prop_data != 0) {
      result = ((*(unsigned long *)prop_data) == IconicState);
    }

    XFree(prop_data);
  }
  return(result);
}

internal void
wm_window_set_minimized(WM_Window handle, b32 minimized) {
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    XIconifyWindow(lnx_wm_state->display, window->window, DefaultScreen(lnx_wm_state->display));
  } else {
    XMapWindow(lnx_wm_state->display, window->window);
  }
  XFlush(lnx_wm_state->display);
}

internal void
wm_window_set_name(WM_Window handle, String8 name) {
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    Temp scratch = scratch_begin(0, 0);
    String8 name_copy = str8_copy(scratch.arena, name);
    XStoreName(lnx_wm_state->display, window->window, (char *)name_copy.str);
    scratch_end(scratch);
  }
}

internal Range2
wm_window_get_rect(WM_Window handle) {
  Range2 result = {0};
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    XWindowAttributes attrs = {0};
    Status status = XGetWindowAttributes(lnx_wm_state->display, window->window, &attrs);
    result = range2_make((f32)attrs.x, (f32)attrs.y, (f32)(attrs.x+attrs.width), (f32)(attrs.y+attrs.height));
  }
  return(result);
}

internal Range2
wm_window_get_client_rect(WM_Window handle) {
  Range2 result = {0};
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    XWindowAttributes attrs = {0};
    Status status = XGetWindowAttributes(lnx_wm_state->display, window->window, &attrs);
    result = range2_make(0, 0, (f32)attrs.width, (f32)attrs.height);
  }
  return(result);
}

////////////////////////////////
// NOTE: @per_os_impl Events

internal WM_Event_List
wm_get_events(Arena *arena, b32 wait) {
  WM_Event_List events = {0};

  for (;;) {
    if (XPending(lnx_wm_state->display) == 0) {
      struct pollfd poll_fds[2] = {
        {.fd = ConnectionNumber(lnx_wm_state->display), .events = POLLIN},
        {.fd = lnx_wm_state->wakeup_fd,                 .events = POLLIN},
      };
      int timeout = wait && events.count == 0 ? -1 : 0;
      int poll_status = poll(poll_fds, array_count(poll_fds), timeout);
      assert(poll_status >= 0);
      if (poll_fds[1].revents & POLLIN) {
        u64 dummy = 0;
        read(lnx_wm_state->wakeup_fd, &dummy, sizeof(dummy));
        wait = 0;
      }
    }
    while (XPending(lnx_wm_state->display)) {
      XEvent event = {0};
      XNextEvent(lnx_wm_state->display, &event);

      b32 set_mouse_cursor = 0;
      switch (event.type) {
        case KeyPress:
        case KeyRelease: {
          LNX_WM_Window *window = lnx_wm_window_from_x11_window(event.xkey.window);

          WM_Modifiers modifiers = 0;
          if (event.xbutton.state & ShiftMask)   {modifiers |= WM_Modifier_SHIFT;}
          if (event.xbutton.state & ControlMask) {modifiers |= WM_Modifier_CTRL;}
          if (event.xbutton.state & Mod1Mask)    {modifiers |= WM_Modifier_ALT;}

          KeySym keysym = 0;
          u8 text[256] = {0};
          u64 text_size = Xutf8LookupString(window->xic, &event.xkey, (char *)text, sizeof(text), &keysym, 0);

          b32 is_right_sided = 0;
          WM_Key key = lnx_wm_key_from_keysym(keysym, &is_right_sided);

          if (event.type == KeyPress && text_size != 0) {
            for (u64 off = 0; off < text_size;) {
              Unicode_Decode decode = utf8_decode(text+off, text_size-off);
              if (decode.codepoint != 0 && decode.codepoint != 127 && decode.codepoint >= 32) {
                WM_Event *e = wm_event_list_push(arena, &events, WM_Event_Type_TEXT);
                e->window = lnx_wm_handle_from_window(window);
                e->character = decode.codepoint;
              }
              if (decode.increment == 0) {
                break;
              }
              off += decode.increment;
            }
          }

          WM_Event *e = wm_event_list_push(arena, &events, event.type == KeyPress ? WM_Event_Type_PRESS : WM_Event_Type_RELEASE);
          e->window = lnx_wm_handle_from_window(window);
          e->modifiers = modifiers;
          e->key = key;
          e->is_right_sided = is_right_sided;
        } break;

        case ButtonPress:
        case ButtonRelease: {
          LNX_WM_Window *window = lnx_wm_window_from_x11_window(event.xbutton.window);

          WM_Modifiers modifiers = 0;
          if (event.xbutton.state & ShiftMask)   {modifiers |= WM_Modifier_SHIFT;}
          if (event.xbutton.state & ControlMask) {modifiers |= WM_Modifier_CTRL;}
          if (event.xbutton.state & Mod1Mask)    {modifiers |= WM_Modifier_ALT;}

          WM_Key key = WM_Key_NULL;
          switch (event.xbutton.button) {
            case Button1: {key = WM_Key_LEFT_MOUSE_BUTTON;} break;
            case Button2: {key = WM_Key_MIDDLE_MOUSE_BUTTON;} break;
            case Button3: {key = WM_Key_RIGHT_MOUSE_BUTTON;} break;
          }

          if (key != WM_Key_NULL) {
            WM_Event *e = wm_event_list_push(arena, &events, event.type == ButtonPress ? WM_Event_Type_PRESS : WM_Event_Type_RELEASE);
            e->window = lnx_wm_handle_from_window(window);
            e->modifiers = modifiers;
            e->key = key;
            e->position = vector2_make((f32)event.xbutton.x, (f32)event.xbutton.y);
          }
          else if(event.xbutton.button == Button4 || event.xbutton.button == Button5) {
            WM_Event *e = wm_event_list_push(arena, &events, WM_Event_Type_SCROLL);
            e->window = lnx_wm_handle_from_window(window);
            e->modifiers = modifiers;
            e->position = vector2_make((f32)event.xbutton.x, (f32)event.xbutton.y);
            e->delta = vector2_make(0, event.xbutton.button == Button4 ? -1.0f : +1.0f);
          }
        } break;

        case MotionNotify: {
          LNX_WM_Window *window = lnx_wm_window_from_x11_window(event.xclient.window);
          WM_Event *e = wm_event_list_push(arena, &events, WM_Event_Type_MOUSE_MOVE);
          e->window = lnx_wm_handle_from_window(window);
          e->position.x = (f32)event.xmotion.x;
          e->position.y = (f32)event.xmotion.y;
          set_mouse_cursor = 1;
        } break;

        // TODO: I think it's needed to set mouse cursor when exposing the window.
        // Apparently, the cursor is cached when quitting the program.
        // When launching it again, it uses the cached cursor for a single frame.

        case ClientMessage: {
          if ((Atom)event.xclient.data.l[0] == lnx_wm_state->wm_delete_window_atom) {
            LNX_WM_Window *window = lnx_wm_window_from_x11_window(event.xclient.window);
            WM_Event *e = wm_event_list_push(arena, &events, WM_Event_Type_WINDOW_CLOSE);
            e->window = lnx_wm_handle_from_window(window);
          }
        } break;
      }
      if (set_mouse_cursor) {
        Window root_window = 0;
        Window child_window = 0;
        int root_rel_x = 0;
        int root_rel_y = 0;
        int child_rel_x = 0;
        int child_rel_y = 0;
        unsigned int mask = 0;
        if (XQueryPointer(lnx_wm_state->display, XDefaultRootWindow(lnx_wm_state->display), &root_window, &child_window, &root_rel_x, &root_rel_y, &child_rel_x, &child_rel_y, &mask)) {
          XDefineCursor(lnx_wm_state->display, root_window, lnx_wm_state->cursors[lnx_wm_state->last_set_cursor]);
          XFlush(lnx_wm_state->display);
        }
      }
    }
    if (events.count > 0 || (wait == 0 && events.count == 0)) {
      break;
    }
  }
  return(events);
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
  char keys[32];
  XQueryKeymap(lnx_wm_state->display, keys);
  KeySym sym = lnx_wm_keysym_from_key(key);
  KeyCode code = XKeysymToKeycode(lnx_wm_state->display, sym);
  b32 result = !!(keys[code/8] & (1u << (code % 8)));
  return(result);
}

internal Vector2
wm_mouse_from_window(WM_Window handle) {
  Vector2 result = {0};
  LNX_WM_Window *window = lnx_wm_window_from_handle(handle);
  if (window != 0) {
    Window root_window = 0;
    Window child_window = 0;
    int root_rel_x = 0;
    int root_rel_y = 0;
    int child_rel_x = 0;
    int child_rel_y = 0;
    unsigned int mask = 0;
    if (XQueryPointer(lnx_wm_state->display, window->window, &root_window, &child_window, &root_rel_x, &root_rel_y, &child_rel_x, &child_rel_y, &mask)) {
      result.x = (f32)child_rel_x;
      result.y = (f32)child_rel_y;
    }
  }
  return(result);
}

////////////////////////////////
// NOTE: @per_os_impl Cursors

internal void
wm_set_cursor(WM_Cursor cursor) {
  lnx_wm_state->last_set_cursor = cursor;
}
