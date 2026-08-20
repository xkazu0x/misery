////////////////////////////////
// NOTE: Handle Type Functions

// NOTE: windows

internal R_Window
r_window_zero(void) {
  R_Window result = {0};
  return(result);
}

internal b32
r_window_match(R_Window a, R_Window b) {
  b32 result = memory_match_struct(&a, &b);
  return(result);
}

// NOTE: textures

internal R_Texture
r_texture_zero(void) {
  R_Texture result = {0};
  return(result);
}

internal b32
r_texture_match(R_Texture a, R_Texture b) {
  b32 result = memory_match_struct(&a, &b);
  return(result);
}

////////////////////////////////
// NOTE: Context/Batch Type Functions

internal R_Context
r_ctx_make(Arena *arena, R_Rect_Batch_Group_List *groups) {
  R_Context result = {0};
  result.arena = arena;
  result.groups = groups;
  return(result);
}

internal R_Rect *
r_rect_batch_list_push(Arena *arena, R_Rect_Batch_List *list) {
  R_Rect *result = 0;
  {
    R_Rect_Batch_Node *n = list->last;
    if (n == 0 || n->v.count >= R_RECT_BATCH_CHUNK_MAX) {
      n = push_array(arena, R_Rect_Batch_Node, 1);
      n->v.v = push_array_no_zero(arena, R_Rect, R_RECT_BATCH_CHUNK_MAX);
      sll_queue_push(list->first, list->last, n);
      list->node_count += 1;
    }
    result = n->v.v + n->v.count;
    n->v.count += 1;
    list->total_count += 1;
  }
  return(result);
}

////////////////////////////////
// NOTE: Draw Calls

internal R_Rect *
r_rect(R_Context *ctx, Range2 dst, Vector4 color) {
  Arena *arena = ctx->arena;
  R_Rect_Batch_Group_List *groups = ctx->groups;
  R_Rect_Batch_Group_Node *group_n = groups->last;
  if (group_n == 0) {
    group_n = push_array(arena, R_Rect_Batch_Group_Node, 1);
    group_n->params.texture = r_texture_zero();
    sll_queue_push(groups->first, groups->last, group_n);
    groups->count += 1;
  }
  R_Rect *inst = r_rect_batch_list_push(arena, &group_n->batches);
  inst->dst = dst;
  inst->src = range2_make(0.0f, 0.0f, 0.0f, 0.0f);
  inst->color = color;
  inst->white_texture_override = 1.0f;
  return(inst);
}

internal R_Rect *
r_img(R_Context *ctx, Range2 dst, Range2 src, R_Texture texture, Vector4 color) {
  Arena *arena = ctx->arena;
  R_Rect_Batch_Group_List *groups = ctx->groups;
  R_Rect_Batch_Group_Node *group_n = groups->last;
  if (group_n == 0 || !r_texture_match(group_n->params.texture, texture)) {
    group_n = push_array(arena, R_Rect_Batch_Group_Node, 1);
    group_n->params.texture = texture;
    sll_queue_push(groups->first, groups->last, group_n);
    groups->count += 1;
  }
  R_Rect *inst = r_rect_batch_list_push(arena, &group_n->batches);
  inst->dst = dst;
  inst->src = src;
  inst->color = color;
  inst->white_texture_override = 0.0f;
  return(inst);
}
