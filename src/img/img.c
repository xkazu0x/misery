internal IMG__Context
img__ctx_make(String8 stream) {
  IMG__Context result = {0};
  result.stream = stream;
  result.ptr = stream.str;
  result.opl = stream.str + stream.size;
  return(result);
}

internal void
img__ctx_rewind(IMG__Context *ctx) {
  ctx->ptr = ctx->stream.str;
  ctx->opl = ctx->ptr + ctx->stream.size;
}

internal u8
img__ctx_get8(IMG__Context *ctx) {
  u8 z = 0;
  if (ctx->ptr < ctx->opl) {
    z = *ctx->ptr++;
  }
  return(z);
}

internal u16
img__ctx_get16_le(IMG__Context *ctx) {
  u8 lo = img__ctx_get8(ctx);
  u8 hi = img__ctx_get8(ctx);
  u16 z = (hi << 8) | lo;
  return(z);
}

internal u32
img__ctx_get32_le(IMG__Context *ctx) {
  u16 lo = img__ctx_get16_le(ctx);
  u16 hi = img__ctx_get16_le(ctx);
  u32 z = (hi << 16) | lo;
  return(z);
}

internal b32
img__bmp_test(IMG__Context *ctx) {
  if (img__ctx_get8(ctx) != 'B') invalid_path;
  if (img__ctx_get8(ctx) != 'M') invalid_path;
  img__ctx_get32_le(ctx); // discard filesize
  img__ctx_get16_le(ctx); // discard reserved
  img__ctx_get16_le(ctx); // discard reserved
  img__ctx_get32_le(ctx); // discard data offset
  u32 sz = img__ctx_get32_le(ctx);
  b32 z = (sz == 12 || sz == 40 || sz == 56 || sz == 108 || sz == 124);
  img__ctx_rewind(ctx);
  return(z);
}

internal void
img__bmp_parse_header(IMG__Context *ctx, IMG__BMP_Info *info) {
  u32 hsz;
  if (img__ctx_get8(ctx) != 'B') invalid_path;
  if (img__ctx_get8(ctx) != 'M') invalid_path;
  img__ctx_get32_le(ctx); // discard filesize
  img__ctx_get16_le(ctx); // discard reserved
  img__ctx_get16_le(ctx); // discard reserved
  info->off = img__ctx_get32_le(ctx);
  info->hsz = hsz = img__ctx_get32_le(ctx);
  info->rm = info->gm = info->bm = info->am = 0;
  if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124) invalid_path;
  if (hsz == 12) {
    info->w = img__ctx_get16_le(ctx);
    info->h = img__ctx_get16_le(ctx);
  } else {
    info->w = img__ctx_get32_le(ctx);
    info->h = img__ctx_get32_le(ctx);
  }
  if (img__ctx_get16_le(ctx) != 1) invalid_path;
  info->bpp = img__ctx_get16_le(ctx);
  if (hsz != 12) {
    u32 compress = img__ctx_get32_le(ctx);
    if (compress == 1 || compress == 2) invalid_path;
    if (compress == 3 && info->bpp != 16 && info->bpp != 32) invalid_path;
    if (compress >= 4) invalid_path;
    img__ctx_get32_le(ctx); // discard size of image
    img__ctx_get32_le(ctx); // discard hres
    img__ctx_get32_le(ctx); // discard vres
    img__ctx_get32_le(ctx); // discard colors used
    img__ctx_get32_le(ctx); // discard colors important
    if (hsz == 40) {
      if (info->bpp == 16) {
        info->rm = bitmask5 << 11;
        info->gm = bitmask6 << 5;
        info->bm = bitmask5 << 0;
      } else if (info->bpp == 32) {
        info->rm = bitmask8 << 16;
        info->gm = bitmask8 << 8;
        info->bm = bitmask8 << 0;
        info->am = bitmask8 << 24;
      }
    }
    if (hsz == 56 || hsz == 108 || hsz == 124) {
      if (compress == 0) {
        if (info->bpp == 16) {
          info->rm = bitmask5 << 11;
          info->gm = bitmask6 << 5;
          info->bm = bitmask5 << 0;
        } else if (info->bpp == 32) {
          info->rm = bitmask8 << 16;
          info->gm = bitmask8 << 8;
          info->bm = bitmask8 << 0;
          info->am = bitmask8 << 24;
        }
      } else if (compress == 3) {
        info->rm = img__ctx_get32_le(ctx);
        info->gm = img__ctx_get32_le(ctx);
        info->bm = img__ctx_get32_le(ctx);
        info->am = img__ctx_get32_le(ctx);
      }
    }
    if (hsz == 108 || hsz == 124) {
      img__ctx_get32_le(ctx); // discard color space
      for (u32 idx = 0; idx < 12; idx += 1) {
        img__ctx_get32_le(ctx); // discard color space parameters
      }
      if (hsz == 124) {
        img__ctx_get32_le(ctx); // discard rendering intent
        img__ctx_get32_le(ctx); // discard offset of profile data
        img__ctx_get32_le(ctx); // discard size of profile data
        img__ctx_get32_le(ctx); // discard reserved
      }
    }
  }
}

internal IMG_Data
img_data_from_file_path(Arena *arena, String8 path) {
  IMG_Data result = {0};
  Temp scratch = scratch_begin(&arena, 1);
  String8 path_copy = str8_copy(scratch.arena, path);
  String8 file_data = data_from_file_path(scratch.arena, path);
  if (file_data.size > 0) {
    IMG__Context ctx = img__ctx_make(file_data);
    if (!img__bmp_test(&ctx)) invalid_path;
    IMG__BMP_Info info;
    img__bmp_parse_header(&ctx, &info);
    if (info.bpp != 32) invalid_path;
    u32 row_sz = info.w*(info.bpp/8);
    u64 sz = info.h*row_sz;
    result.w = info.w;
    result.h = info.h;
    result.n = (info.bpp/8);
    result.data = (u32 *)push_array(arena, u8, sz);
    u8 *src_row = (u8 *)file_data.str + info.off;
    u8 *dst_row = (u8 *)result.data + (info.h-1)*row_sz;
    for (u32 y = 0; y < info.h; y += 1) {
      memory_copy(dst_row, src_row, row_sz);
      dst_row -= row_sz;
      src_row += row_sz;
    }
  }
  return(result);
};
