#ifndef FONT_PROVIDER_CORE_H
#define FONT_PROVIDER_CORE_H

////////////////////////////////
// NOTE: Types

typedef struct FP_Handle FP_Handle;
struct FP_Handle {
  u64 v[1];
};

typedef struct FP_Metrics FP_Metrics;
struct FP_Metrics {
  f32 ascent;
  f32 descent;
  f32 line_gap;
  f32 capital_height;
};

typedef struct FP_Raster_Result FP_Raster_Result;
struct FP_Raster_Result {
  Vector2 atlas_dim;
  void *atlas_data;
  f32 advance;
};

////////////////////////////////
// NOTE: Handle Type Functions

internal FP_Handle fp_handle_zero(void);
internal b32       fp_handle_match(FP_Handle a, FP_Handle b);

////////////////////////////////
// NOTE: Backend Hooks

// NOTE: layer initialization
internal void fp_init(void);

// NOTE: font
internal FP_Handle fp_font_open(String8 path);
internal FP_Handle fp_font_open_from_static_data_string(String8 *data_ptr);
internal void      fp_font_close(FP_Handle font);
internal void      fp_metrics_from_font(FP_Handle handle);

// NOTE: raster
internal FP_Raster_Result fp_raster(Arena *arena, FP_Handle font, f32 size, String8 string);

#endif // FONT_PROVIDER_CORE_H
