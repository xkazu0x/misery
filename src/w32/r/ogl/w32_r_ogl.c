internal void_proc *
r_ogl_load_proc(char *name) {
  void_proc *ptr = (void_proc*)wglGetProcAddress(name);
  if (ptr == (void_proc*)1 || ptr == (void_proc*)2 || ptr == (void_proc*)3 || ptr == (void_proc*)-1) {
    ptr = 0;
  }
  return(ptr);
}

internal void
r_ogl_init(void) {
  WNDCLASSEXW wndclass = {sizeof(wndclass)};
  wndclass.lpfnWndProc = DefWindowProcW;
  wndclass.hInstance = GetModuleHandle(0);
  wndclass.lpszClassName = L"bootstrap-window";
  ATOM wndatom = RegisterClassExW(&wndclass);
  HWND hwnd = CreateWindowExW(0, L"bootstrap-window", L"", 0,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              0, 0, wndclass.hInstance, 0);

  HDC hdc = GetDC(hwnd);

  PIXELFORMATDESCRIPTOR pfd = {sizeof(pfd)};
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pf = ChoosePixelFormat(hdc, &pfd);
  DescribePixelFormat(hdc, pf, sizeof(pfd), &pfd);
  SetPixelFormat(hdc, pf, &pfd);

  HGLRC ctx = wglCreateContext(hdc);
  wglMakeCurrent(hdc, ctx);

  wglChoosePixelFormatARB    = (FNWGLCHOOSEPIXELFORMATARBPROC*)   r_ogl_load_proc("wglChoosePixelFormatARB");
  wglCreateContextAttribsARB = (FNWGLCREATECONTEXTATTRIBSARBPROC*)r_ogl_load_proc("wglCreateContextAttribsARB");
  wglSwapIntervalEXT         = (FNWGLSWAPINTERVALEXTPROC*)        r_ogl_load_proc("wglSwapIntervalEXT");

  int pf_attribs_i[] = {
    WGL_DRAW_TO_WINDOW_ARB, 1,
    WGL_SUPPORT_OPENGL_ARB, 1,
    WGL_DOUBLE_BUFFER_ARB, 1,
    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB, 32,
    WGL_DEPTH_BITS_ARB, 24,
    WGL_STENCIL_BITS_ARB, 8,
    0
  };
  UINT num_formats = 0;
  wglChoosePixelFormatARB(hdc, pf_attribs_i, 0, 1, &pf, &num_formats);

  HGLRC real_ctx = 0;
  if (pf) {
    b32 debug_mode = 0;
#if BUILD_DEBUG
    debug_mode = 1;
#endif
    int ctx_attribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
      WGL_CONTEXT_MINOR_VERSION_ARB, 3,
      WGL_CONTEXT_FLAGS_ARB, !!debug_mode*WGL_CONTEXT_DEBUG_BIT_ARB,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
    };
    real_ctx = wglCreateContextAttribsARB(hdc, ctx, ctx_attribs);
    r_ogl_w32_hglrc = real_ctx;
  }

  wglMakeCurrent(hdc, 0);
  wglDeleteContext(ctx);
  wglMakeCurrent(hdc, real_ctx);
  wglSwapIntervalEXT(1);
}

internal void
r_ogl_window_equip(WM_Window window) {
  W32_WM_Window *w = w32_wm_window_from_handle(window);
  wglMakeCurrent(w->hdc, r_ogl_w32_hglrc);

  int pixel_format = 0;
  UINT num_formats = 0;
  int pf_attribs_i[] = {
    WGL_DRAW_TO_WINDOW_ARB, 1,
    WGL_SUPPORT_OPENGL_ARB, 1,
    WGL_DOUBLE_BUFFER_ARB, 1,
    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB, 32,
    WGL_DEPTH_BITS_ARB, 24,
    WGL_STENCIL_BITS_ARB, 8,
    0
  };

  wglChoosePixelFormatARB(w->hdc,
                          pf_attribs_i,
                          0,
                          1,
                          &pixel_format,
                          &num_formats);

  PIXELFORMATDESCRIPTOR pfd = {sizeof(pfd)};
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;
  SetPixelFormat(w->hdc, pixel_format, &pfd);
}

internal void
r_ogl_os_window_unequip(WM_Window window) {
}

internal void
r_ogl_window_select(WM_Window window) {
  W32_WM_Window *w = w32_wm_window_from_handle(window);
  if (w != 0) {
    wglMakeCurrent(w->hdc, r_ogl_w32_hglrc);
  }
}

internal void
r_ogl_window_swap(WM_Window window) {
  W32_WM_Window *w = w32_wm_window_from_handle(window);
  if (w != 0) {
    SwapBuffers(w->hdc);
  }
}
