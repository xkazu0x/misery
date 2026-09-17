////////////////////////////////
// NOTE: Globals

thread_static TCTX *tctx_thread_local;

////////////////////////////////
// NOTE: Thread Context Functions

// NOTE: thread-context allocation & selection

internal TCTX *
tctx_alloc(void) {
  Arena *arena0 = arena_alloc();
  Arena *arena1 = arena_alloc();
  TCTX *tctx = push_array(arena0, TCTX, 1);
  tctx->arenas[0] = arena0;
  tctx->arenas[1] = arena1;
  return(tctx);
}

internal void
tctx_release(TCTX *tctx) {
  arena_release(tctx->arenas[1]);
  arena_release(tctx->arenas[0]);
}

internal void
tctx_select(TCTX *tctx) {
  tctx_thread_local = tctx;
}

internal TCTX *
tctx_selected(void) {
  return(tctx_thread_local);
}

// NOTE: scratch arenas

internal Arena *
tctx_get_scratch(Arena **conflicts, u64 count) {
  TCTX *tctx = tctx_selected();
  Arena *result = 0;
  Arena **arena_ptr = tctx->arenas;
  for (u64 i = 0; i < array_count(tctx->arenas); i += 1, arena_ptr += 1) {
    Arena **conflict_ptr = conflicts;
    b32 has_conflict = 0;
    for (u64 j = 0; j < count; j += 1, conflict_ptr += 1) {
      if (*arena_ptr == *conflict_ptr) {
        has_conflict = 1;
        break;
      }
    }
    if(!has_conflict) {
      result = *arena_ptr;
      break;
    }
  }
  return(result);
}
