////////////////////////////////
// NOTE: Helpers

internal FP_Handle
fp_ft_handle_from_font(FP_FT_Font font) {
  FP_Handle result = {(u64)font.face};
  return(result);
}

internal FP_FT_Font
fp_ft_font_from_handle(FP_Handle handle) {
  FP_FT_Font result = {(FT_Face)handle.v[0]};
  return(result);
}

////////////////////////////////
// NOTE: Backend Hooks

// NOTE: layer initialization

internal void
fp_init(void) {
  Arena *arena = arena_alloc();
  fp_ft_state = push_array(arena, FP_FT_State, 1);
  fp_ft_state->arena = arena;
  FT_Init_FreeType(&fp_ft_state->library);
}

// NOTE: font

internal FP_Handle
fp_font_open(String8 path) {
  Temp scratch = scratch_begin(0, 0);
  path = str8_copy(scratch.arena, path);
  FP_FT_Font font = {0};
  FT_New_Face(fp_ft_state->library, (char *)path.str, 0, &font.face);
  FP_Handle result = fp_ft_handle_from_font(font);
  scratch_end(scratch);
  return(result);
}

internal FP_Handle
fp_font_open_from_static_data_string(String8 *data_ptr) {
  FP_FT_Font font = {0};
  FT_New_Memory_Face(fp_ft_state->library, data_ptr->str, (FT_Long)data_ptr->size, 0, &font.face);
  FP_Handle result = fp_ft_handle_from_font(font);
  return(result);
}

internal void
fp_font_close(FP_Handle handle) {
  FP_FT_Font font = fp_ft_font_from_handle(handle);
  if (font.face != 0) {
    FT_Done_Face(font.face);
  }
}

internal void
fp_metrics_from_font(FP_Handle handle) {
  FP_Metrics result = {0};
  FP_FT_Font font = fp_ft_font_from_handle(handle):
  if (font.face != 0) {
    result.ascent         = (f32)font.face->ascender;
    result.descent        = -(f32)font.face->descender;
    result.line_gap       = (f32)(font.face->height - font.face->ascender + font.face->descender);
    result.capital_height = (f32)(font.face->ascender);
  }
  return(result);
}

// NOTE: raster

internal FP_Raster_Result
fp_raster(Arena *arena, FP_Handle handle, f32 size, String8 string) {
  FP_Raster_Result result = {0};
  FP_FT_Font font = fp_ft_font_from_handle(handle);
  if (font.face != 0) {
    Temp scratch = scratch_begin(0, 0);

    // NOTE: unpack font
    FT_Face face = font.face;
    FT_Set_Pixel_Sizes(face, 0, (FT_UInt)((96.0f/72.0f)*size));
    s64 ascent = face->size->metrics.ascender >> 6;
    s64 height = face->size->metrics.height >> 6;

    // NOTE: unpack string
    String32 string32 = str32_from_8(scratch.arena, string);

    // NOTE: measure
    s32 total_width = 0;
    for (u64 idx = 0; idx < string32.size; idx += 1) {
      FT_Load_Char(face, string32.str[idx], FT_LOAD_RENDER);
      total_width += (face->glyph->advance.x >> 6);
    }

    // NOTE: allocate & fill atlas w/ rasterization
    s32 dim_x = total_width+1;
    s32 dim_y = (s32)height+1;
    u64 atlas_size = dim_x*dim_y*4;
    u8 *atlas_data = push_array(arena, u8, atlas_size);
    s32 baseline = (s32)ascent;
    s32 atlas_write_x = 0;
    for (u64 idx = 0; idx < string32.size; idx += 1) {
      FT_Load_Char(face, string32.str[idx], FT_LOAD_RENDER);
      FT_Bitmap *bmp = &face->glyph->bitmap;
      s32 top = face->glyph->bitmap_top;
      s32 left = face->glyph->bitmap_left;
      for (s32 row = 0; row < (s32)bmp->rows; row += 1) {
        s32 y = baseline - top + row;
        for (s32 col = 0; col < (s32)bmp->width; col += 1) {
          s32 x = atlas_write_x + left + col;
          u64 off = (y*dim_x + x)*4;
          if (off+4 <= atlas_size) {
            atlas_data[off+0] = 255;
            atlas_data[off+1] = 255;
            atlas_data[off+2] = 255;
            atlas_data[off+3] = bmp->buffer[row*bmp->pitch + col];
          }
        }
      }
      atlas_write_x += (face->glyph->advance.x >> 6);
    }

    // NOTE: fill result
    result.atlas_dim = vector2_make((f32)dim_x, (f32)dim_y);
    result.atlas_data = atlas_data;
    result.advance = (f32)total_width;
    scratch_end(scratch);
  }
  return(result);
}
