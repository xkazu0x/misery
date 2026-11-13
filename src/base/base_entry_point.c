////////////////////////////////
// NOTE: Entry Points

internal void
main_thread_base_entry_point(int argc, char **argv) {
  // NOTE: init all included layers
#if defined(WINDOW_MANAGER_H) && !defined(WINDOW_MANAGER_INIT_MANUAL)
  wm_init();
#endif

  // NOTE: call into entry point
  entry_point(argc, argv);
}
