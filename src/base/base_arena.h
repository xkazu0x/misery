#ifndef BASE_ARENA_H
#define BASE_ARENA_H

////////////////////////////////
// NOTE: Arena Types

#define ARENA_HEADER_SIZE 128

typedef u64 Arena_Flags;
enum {
  Arena_Flag_NO_CHAIN = (1<<0),
};

typedef struct Arena_Params Arena_Params;
struct Arena_Params {
  Arena_Flags flags;
  u64 res_size;
  u64 cmt_size;
  void *opt_backing_buffer;
};

typedef struct Arena Arena;
struct Arena {
  Arena *prev; // NOTE: previous arena in chain
  Arena *curr; // NOTE: current arena in chain
  Arena_Flags flags;
  u64 cmt_size;
  u64 res_size;
  u64 base_pos;
  u64 pos;
  u64 cmt;
  u64 res;
};
static_assert(sizeof(Arena) <= ARENA_HEADER_SIZE, arena_header_size_check);

typedef struct Temp Temp;
struct Temp {
  Arena *arena;
  u64 pos;
};

////////////////////////////////
// NOTE: Arena Functions

global const u64 arena_default_res_size = MB(64);
global const u64 arena_default_cmt_size = KB(64);
global const Arena_Flags arena_default_flags = 0;

// NOTE: arena creation/destruction
#define arena_alloc(...) arena_alloc_(&(Arena_Params){.res_size = arena_default_res_size, .cmt_size = arena_default_cmt_size, .flags = arena_default_flags, __VA_ARGS__})
internal Arena *arena_alloc_(Arena_Params *params);
internal void   arena_release(Arena *arena);

// NOTE: arena push/pop core functions
internal void *arena_push(Arena *arena, u64 size, u64 align, b32 zero);
internal u64   arena_pos(Arena *arena);
internal void  arena_pop_to(Arena *arena, u64 pos);

// NOTE: arena clear/pop helpers
internal void arena_clear(Arena *arena);
internal void arena_pop(Arena *arena, u64 amt);

// NOTE: temporary arena scopes
internal Temp temp_begin(Arena *arena);
internal void temp_end(Temp temp);

// NOTE: push helper macros
#define push_array_no_zero_aligned(a,T,c,align) (T *)arena_push((a), sizeof(T)*(c), (align), (0))
#define push_array_aligned(a,T,c,align)         (T *)arena_push((a), sizeof(T)*(c), (align), (1))
#define push_array_no_zero(a,T,c)               (T *)push_array_no_zero_aligned(a, T, c, max(8, align_of(T)))
#define push_array(a,T,c)                       (T *)push_array_aligned(a, T, c, max(8, align_of(T)))

#endif // BASE_ARENA_H
