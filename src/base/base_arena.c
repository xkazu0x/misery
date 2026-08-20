////////////////////////////////
// NOTE: Arena Functions

// NOTE: arena creation/destruction

internal Arena *
arena_alloc_(Arena_Params *params) {
  u64 res_size = params->res_size;
  u64 cmt_size = params->cmt_size;

  // NOTE: reserve/commit initial block
  void *base = params->opt_backing_buffer;
  if (base == 0) {
    // NOTE: round up reserve/commit sizes
    res_size = align_up_pow2(res_size, get_system_info()->page_size);
    cmt_size = align_up_pow2(cmt_size, get_system_info()->page_size);
    base = memory_reserve(res_size);
    memory_commit(base, cmt_size);
    asan_poison_memory_region(base, cmt_size);
  } else {
    asan_poison_memory_region(base, params->res_size);
  }

  // NOTE: extract arena header & fill
  asan_unpoison_memory_region(base, ARENA_HEADER_SIZE);
  Arena *arena    = (Arena *)base;
  arena->curr     = arena;
  arena->flags    = params->flags;
  arena->cmt_size = params->cmt_size;
  arena->res_size = params->res_size;
  arena->base_pos = 0;
  arena->pos      = ARENA_HEADER_SIZE;
  arena->cmt      = cmt_size;
  arena->res      = res_size;
#if ARENA_FREE_LIST
  arena->free_last = 0;
#endif

  return(arena);
}

internal void
arena_release(Arena *arena) {
  for (Arena *n = arena->curr, *prev = 0; n != 0; n = prev) {
    prev = n->prev;
    asan_unpoison_memory_region(n, n->cmt);
    memory_release(n, n->res);
  }
}

// NOTE: arena push/pop core functions

internal void *
arena_push(Arena *arena, u64 size, u64 align, b32 zero) {
  Arena *curr = arena->curr;
  u64 pos_pre = align_up_pow2(curr->pos, align);
  u64 pos_pst = pos_pre + size;

  // NOTE: compute the size to zero
  u64 size_to_zero = 0;
  if (zero) {
    size_to_zero = min(curr->cmt, pos_pst) - pos_pre;
  }

  // NOTE: chain, if needed
  if (curr->res < pos_pst && !(arena->flags & Arena_Flag_NO_CHAIN)) {
    u64 res_size = curr->res_size;
    u64 cmt_size = curr->cmt_size;
    if (size + ARENA_HEADER_SIZE > res_size) {
      res_size = align_up_pow2(size + ARENA_HEADER_SIZE, align);
      cmt_size = align_up_pow2(size + ARENA_HEADER_SIZE, align);
    }
    Arena *new_block = arena_alloc(.res_size = res_size, .cmt_size = cmt_size, .flags = curr->flags);

    new_block->base_pos = curr->base_pos + curr->res;
    sll_stack_push_n(arena->curr, new_block, prev);

    curr = new_block;
    pos_pre = align_up_pow2(curr->pos, align);
    pos_pst = pos_pre + size;

    size_to_zero = 0;
  }

  // NOTE: commit new pages, if needed
  if (curr->cmt < pos_pst) {
    u64 cmt_pst_aligned = pos_pst + curr->cmt_size-1;
    cmt_pst_aligned -= cmt_pst_aligned%curr->cmt_size;
    u64 cmt_pst_clamped = clamp_top(cmt_pst_aligned, curr->res);
    u64 cmt_size = cmt_pst_clamped - curr->cmt;
    u8 *cmt_ptr = (u8 *)curr + curr->cmt;
    memory_commit(cmt_ptr, cmt_size);
    asan_poison_memory_region(cmt_ptr, cmt_size);
    curr->cmt = cmt_pst_clamped;
  }

  // NOTE: push onto current block
  void *result = 0;
  if (curr->cmt >= pos_pst) {
    result = (u8 *)curr + pos_pre;
    curr->pos = pos_pst;
    asan_unpoison_memory_region(result, size);
    memory_zero(result, size_to_zero);
  }

  return(result);
}

internal u64
arena_pos(Arena *arena) {
  Arena *curr = arena->curr;
  u64 result = curr->base_pos + curr->pos;
  return(result);
}

internal void
arena_pop_to(Arena *arena, u64 pos) {
  u64 big_pos = clamp_bot(ARENA_HEADER_SIZE, pos);
  Arena *curr = arena->curr;
  for (Arena *prev = 0; curr->base_pos >= big_pos; curr = prev) {
    prev = curr->prev;
    asan_unpoison_memory_region(curr, curr->cmt);
    memory_release(curr, curr->res);
  }
  arena->curr = curr;
  u64 new_pos = big_pos - curr->base_pos;
  assert_always(new_pos <= curr->pos);
  asan_poison_memory_region((u8 *)curr + new_pos, (curr->pos + new_pos));
  curr->pos = new_pos;
}

// NOTE: arena clear/pop helpers

internal void
arena_clear(Arena *arena) {
  arena_pop_to(arena, 0);
}

internal void
arena_pop(Arena *arena, u64 amt) {
  u64 pos_old = arena_pos(arena);
  u64 pos_new = pos_old;
  if (amt < pos_old) {
    pos_new = pos_old - amt;
  }
  arena_pop_to(arena, pos_new);
}

// NOTE: temporary arena scopes

internal Temp
temp_begin(Arena *arena) {
  u64 pos = arena_pos(arena);
  Temp result = {arena, pos};
  return(result);
}

internal void
temp_end(Temp temp) {
  arena_pop_to(temp.arena, temp.pos);
}
