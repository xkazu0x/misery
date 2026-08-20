////////////////////////////////
// NOTE: Entry Points

internal void
main_thread_base_entry_point(int argc, char **argv) {
  // NOTE: initialize all included layers
#if defined(WINDOW_MANAGER_H) && !defined(WINDOW_MANAGER_INIT_MANUAL)
  wm_init();
#endif
#if defined(RENDER_H) && !defined(RENDER_INIT_MANUAL)
  r_init();
#endif

  // NOTE: call into entry point
  entry_point(argc, argv);
}
