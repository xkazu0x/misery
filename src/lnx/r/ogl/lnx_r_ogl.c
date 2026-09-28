////////////////////////////////
// NOTE: @per_os_impl Hooks

internal void_proc *
r_ogl_load_proc(char *name) {
  void_proc *result = (void_proc *)glXGetProcAddressARB((u8 *)name);
  return(result);
}

internal void
r_ogl_init(void) {
  int glx_version_major = 0;
  int glx_version_minor = 0;
  if (!glXQueryVersion(lnx_wm_state->display, &glx_version_major, &glx_version_minor) || (glx_version_major == 1 && glx_version_minor < 3) || glx_version_major < 1) {
    Temp scratch = scratch_begin(0, 0);
    String8 message = str8f(scratch.arena, "Unsupported GLX version (%i.%i, need at least 1.3)", glx_version_major, glx_version_minor);
    fprintf(stderr, "[FATAL] %.*s\n", str8_varg(message));
    abort_self(1);
    scratch_end(scratch);
  }

  local int fb_cfg_attrs[] = {
    GLX_X_RENDERABLE,   1,
    GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
    GLX_RENDER_TYPE,    GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
    GLX_RED_SIZE,       8,
    GLX_GREEN_SIZE,     8,
    GLX_BLUE_SIZE,      8,
    GLX_ALPHA_SIZE,     8,
    GLX_DEPTH_SIZE,     24,
    GLX_STENCIL_SIZE,   8,
    GLX_DOUBLEBUFFER,   1,
    None
  };

  int fb_cfg_count = 0;
  GLXFBConfig *fb_cfgs = glXChooseFBConfig(lnx_wm_state->display, DefaultScreen(lnx_wm_state->display), fb_cfg_attrs, &fb_cfg_count);
  if (fb_cfgs == 0) {
    fprintf(stderr, "[FATAL] %.*s\n", str8_varg(str8_lit("Count not find a suitable framebuffer configuration.")));
    abort_self(1);
  }

  GLXFBConfig fb_cfg = fb_cfgs[0];
  XFree(fb_cfgs);

  // NOTE: extract visual/colormap from chosen fbconfig, publish to os layer
  {
    XVisualInfo *vi = glXGetVisualFromFBConfig(lnx_wm_state->display, fb_cfg);
    if (vi == 0) {
      fprintf(stderr, "[FATAL] %.*s\n", str8_varg(str8_lit("Count not get visual from GLX framebuffer config.")));
      abort_self(1);
    }
    lnx_wm_state->window_visual = vi->visual;
    lnx_wm_state->window_depth = vi->depth;
    lnx_wm_state->window_colormap = XCreateColormap(lnx_wm_state->display, XRootWindow(lnx_wm_state->display, vi->screen), vi->visual, AllocNone);
    XFree(vi);
  }

  // NOTE: create context
  {
    b32 debug_mode = 0;
#if BUILD_DEBUG
    debug_mode = 1;
#endif

    glXCreateContextAttribsARB_Proc_Type glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARB_Proc_Type)glXGetProcAddressARB((u8 *)"glXCreateContextAttribsARB");

    int ctx_attrs[] = {
      GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
      GLX_CONTEXT_MINOR_VERSION_ARB, 3,
      GLX_CONTEXT_FLAGS_ARB,         !!debug_mode*GLX_CONTEXT_DEBUG_BIT_ARB,
      GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
      None
    };

    r_ogl_lnx_ctx = glXCreateContextAttribsARB(lnx_wm_state->display, fb_cfg, 0, 1, ctx_attrs);
  }

  glXMakeCurrent(lnx_wm_state->display, 0, r_ogl_lnx_ctx);
}

internal void
r_ogl_window_equip(WM_Window window) {
}

internal void
r_ogl_window_unequip(WM_Window window) {
}

internal void
r_ogl_window_select(WM_Window window) {
  LNX_WM_Window *w = lnx_wm_window_from_handle(window);
  if (w != 0) {
    glXMakeCurrent(lnx_wm_state->display, w->window, r_ogl_lnx_ctx);
    // NOTE: ensure default framebuffer writes target the back buffer; on some drivers
    // GL_DRAW_BUFFER stays GL_NONE if a context was first made current without a drawable.
    glDrawBuffer(GL_BACK);
  }
}

internal void
r_ogl_window_swap(WM_Window window) {
  LNX_WM_Window *w = lnx_wm_window_from_handle(window);
  if (w != 0) {
    glXSwapBuffers(lnx_wm_state->display, w->window);
  }
}
