#ifndef RENDER_CORE_H
#define RENDER_CORE_H

#define R_RECT_BATCH_CHUNK_MAX 256

typedef enum R_Resource_Type {
  R_Resource_Type_STATIC,
  R_Resource_Type_DYNAMIC,
  R_Resource_Type_STREAM,
} R_Resource_Type;

typedef struct R_Window R_Window;
struct R_Window {
  u64 v[1];
};

typedef struct R_Texture R_Texture;
struct R_Texture {
  u64 v[1];
};

typedef struct R_Rect R_Rect;
struct R_Rect {
  Range2 dst;
  Range2 src;
  Vector4 color;
  f32 white_texture_override;
};

typedef struct R_Rect_Batch R_Rect_Batch;
struct R_Rect_Batch {
  R_Rect *v;
  u64 count;
};

typedef struct R_Rect_Batch_Node R_Rect_Batch_Node;
struct R_Rect_Batch_Node {
  R_Rect_Batch_Node *next;
  R_Rect_Batch v;
};

typedef struct R_Rect_Batch_List R_Rect_Batch_List;
struct R_Rect_Batch_List {
  R_Rect_Batch_Node *first;
  R_Rect_Batch_Node *last;
  u64 node_count;
  u64 total_count;
};

typedef struct R_Rect_Batch_Group_Params R_Rect_Batch_Group_Params;
struct R_Rect_Batch_Group_Params {
  R_Texture texture;
};

typedef struct R_Rect_Batch_Group_Node R_Rect_Batch_Group_Node;
struct R_Rect_Batch_Group_Node {
  R_Rect_Batch_Group_Node *next;
  R_Rect_Batch_List batches;
  R_Rect_Batch_Group_Params params;
};

typedef struct R_Rect_Batch_Group_List R_Rect_Batch_Group_List;
struct R_Rect_Batch_Group_List {
  R_Rect_Batch_Group_Node *first;
  R_Rect_Batch_Group_Node *last;
  u64 count;
};

typedef struct R_Context R_Context;
struct R_Context {
  Arena *arena;
  R_Rect_Batch_Group_List *groups;
};

////////////////////////////////
// NOTE: Handle Type Functions

// NOTE: windows
internal R_Window r_window_zero(void);
internal b32      r_window_match(R_Window a, R_Window b);

// NOTE: textures
internal R_Texture r_texture_zero(void);
internal b32       r_texture_match(R_Texture a, R_Texture b);

////////////////////////////////
// NOTE: Context/Batch Type Functions

internal R_Context r_ctx_make(Arena *arena, R_Rect_Batch_Group_List *groups);
internal R_Rect   *r_rect_batch_list_push(Arena *arena, R_Rect_Batch_List *list);

////////////////////////////////
// NOTE: Draw Calls

internal R_Rect *r_rect(R_Context *ctx, Range2 dst, Vector4 color);
internal R_Rect *r_img(R_Context *ctx, Range2 dst, Range2 src, R_Texture texture, Vector4 color);

////////////////////////////////
// NOTE: Backend Hooks

// NOTE: top-level layer initialization
internal void r_init(void);

// NOTE: windows
internal R_Window r_window_equip(WM_Window handle);
internal void     r_window_unequip(WM_Window handle, R_Window equip_handle);

// NOTE: textures
internal R_Texture r_texture_alloc(R_Resource_Type type, Vector2 size, void *data);
internal void      r_texture_release(R_Texture handle);
internal Vector2   r_texture_size(R_Texture handle);

// NOTE: frame markers
internal void r_begin(WM_Window handle, R_Window equip_handle);
internal void r_end(WM_Window handle, R_Window equip_handle);

// NOTE: flush
internal void r_flush(WM_Window handle, R_Window equip_handle, R_Rect_Batch_Group_List *groups);

#endif // RENDER_CORE_H
