////////////////////////////////
// NOTE: @per_os_impl Main Initialization

internal void
wm_init(void) {
  // NOTE: set up base state
  Arena *arena = arena_alloc();
  lnx_wm_state = push_array(arena, LNX_WM_State, 1);
  lnx_wm_state->arena = arena;
}
