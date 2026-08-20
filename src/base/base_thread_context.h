#ifndef BASE_THREAD_CONTEXT_H
#define BASE_THREAD_CONTEXT_H

typedef struct Thread_Context Thread_Context;
struct Thread_Context {
  // NOTE: scratch arenas
  Arena *arenas[2];
};

////////////////////////////////
// NOTE: Thread Context Functions

// NOTE: thread-context allocation & selection
internal Thread_Context *tctx_alloc(void);
internal void            tctx_release(Thread_Context *tctx);
internal void            tctx_select(Thread_Context *tctx);
internal Thread_Context *tctx_selected(void);

// NOTE: scratch arenas
internal Arena *tctx_get_scratch(Arena **conflicts, u64 count);
#define scratch_begin(conflicts, count) temp_begin(tctx_get_scratch((conflicts), (count)))
#define scratch_end(scratch) temp_end(scratch)

#endif // BASE_THREAD_CONTEXT_H
