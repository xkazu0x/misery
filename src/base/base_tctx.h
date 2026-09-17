#ifndef BASE_TCTX_H
#define BASE_TCTX_H

////////////////////////////////
// NOTE: Base Per-Thread State Bundle

typedef struct TCTX TCTX;
struct TCTX {
  // NOTE: scratch arenas
  Arena *arenas[2];
};

////////////////////////////////
// NOTE: Thread Context Functions

// NOTE: thread-context allocation & selection
internal TCTX *tctx_alloc(void);
internal void  tctx_release(TCTX *tctx);
internal void  tctx_select(TCTX *tctx);
internal TCTX *tctx_selected(void);

// NOTE: scratch arenas
internal Arena *tctx_get_scratch(Arena **conflicts, u64 count);
#define scratch_begin(conflicts, count) temp_begin(tctx_get_scratch((conflicts), (count)))
#define scratch_end(scratch) temp_end(scratch)

#endif // BASE_TCTX_H
