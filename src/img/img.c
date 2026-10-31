internal IMG__Ctx
img__ctx_make(String8 data) {
  IMG__Ctx result = {data, data.str, data.str + data.size};
  return(result);
}

internal u8
img__ctx_read8(IMG__Ctx *ctx) {
  u8 result = memory_read8(ctx->ptr);
  ctx->ptr += sizeof(result);
  return(result);
}

internal u16
img__ctx_read16(IMG__Ctx *ctx) {
  u16 result = memory_read16(ctx->ptr);
  ctx->ptr += sizeof(result);
  return(result);
}

internal u32
img__ctx_read32(IMG__Ctx *ctx) {
  u32 result = memory_read32(ctx->ptr);
  ctx->ptr += sizeof(result);
  return(result);
}

internal void
img__ctx_rewind(IMG__Ctx *ctx) {
  ctx->ptr = ctx->data.str;
  ctx->opl = ctx->ptr + ctx->data.size;
}

internal b32
img__bmp_check_and_rewind(IMG__Ctx *ctx) {
  if (img__ctx_read8(ctx) != 'B') invalid_path;
  if (img__ctx_read8(ctx) != 'M') invalid_path;
  img__ctx_read32(ctx); // discard filesize
  img__ctx_read16(ctx); // discard reserved
  img__ctx_read16(ctx); // discard reserved
  img__ctx_read32(ctx); // discard data offset
  u32 sz = img__ctx_read32(ctx);
  b32 z = (sz == 12 || sz == 40 || sz == 56 || sz == 108 || sz == 124);
  img__ctx_rewind(ctx);
  return(z);
}

internal void
img__bmp_parse_header(IMG__Ctx *ctx, IMG__BMP_Info *info) {
  u32 hsz;
  if (img__ctx_read8(ctx) != 'B') invalid_path;
  if (img__ctx_read8(ctx) != 'M') invalid_path;
  img__ctx_read32(ctx); // discard filesize
  img__ctx_read16(ctx); // discard reserved
  img__ctx_read16(ctx); // discard reserved
  info->off = img__ctx_read32(ctx);
  info->hsz = hsz = img__ctx_read32(ctx);
  info->rm = info->gm = info->bm = info->am = 0;
  if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124) invalid_path;
  if (hsz == 12) {
    info->w = img__ctx_read16(ctx);
    info->h = img__ctx_read16(ctx);
  } else {
    info->w = img__ctx_read32(ctx);
    info->h = img__ctx_read32(ctx);
  }
  if (img__ctx_read16(ctx) != 1) invalid_path;
  info->bpp = img__ctx_read16(ctx);
  if (hsz != 12) {
    u32 compress = img__ctx_read32(ctx);
    if (compress == 1 || compress == 2) invalid_path;
    if (compress == 3 && info->bpp != 16 && info->bpp != 32) invalid_path;
    if (compress >= 4) invalid_path;
    img__ctx_read32(ctx); // discard size of image
    img__ctx_read32(ctx); // discard hres
    img__ctx_read32(ctx); // discard vres
    img__ctx_read32(ctx); // discard colors used
    img__ctx_read32(ctx); // discard colors important
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
        info->rm = img__ctx_read32(ctx);
        info->gm = img__ctx_read32(ctx);
        info->bm = img__ctx_read32(ctx);
        info->am = img__ctx_read32(ctx);
      }
    }
    if (hsz == 108 || hsz == 124) {
      img__ctx_read32(ctx); // discard color space
      for (u32 idx = 0; idx < 12; idx += 1) {
        img__ctx_read32(ctx); // discard color space parameters
      }
      if (hsz == 124) {
        img__ctx_read32(ctx); // discard rendering intent
        img__ctx_read32(ctx); // discard offset of profile data
        img__ctx_read32(ctx); // discard size of profile data
        img__ctx_read32(ctx); // discard reserved
      }
    }
  }
}

internal IMG_Data
img_data_from_file_path(Arena *arena, String8 path) {
  IMG_Data result = {0};
  Temp scratch = scratch_begin(&arena, 1);
  String8 path_copy = str8_copy(scratch.arena, path);
  String8 file_data = data_from_file_path(scratch.arena, path_copy);
  if (file_data.size > 0) {
    IMG__Ctx ctx = img__ctx_make(file_data);
    if (!img__bmp_check_and_rewind(&ctx)) invalid_path;
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
