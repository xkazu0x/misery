#ifndef IMG_H
#define IMG_H

typedef struct IMG__Ctx IMG__Ctx;
struct IMG__Ctx {
  String8 data;
  u8 *ptr, *opl;
};

typedef struct IMG__BMP_Info IMG__BMP_Info;
struct IMG__BMP_Info {
  u32 w, h, bpp;
  u32 off, hsz;
  u32 rm, gm, bm, am;
};

typedef struct IMG_Data IMG_Data;
struct IMG_Data {
  s32 w, h, n;
  void *data;
};

internal IMG__Ctx img__ctx_make(String8 stream);
internal u8       img__ctx_read8(IMG__Ctx *ctx);
internal u16      img__ctx_read16(IMG__Ctx *ctx);
internal u32      img__ctx_read32(IMG__Ctx *ctx);
internal void     img__ctx_rewind(IMG__Ctx *ctx);

internal b32  img__bmp_check_and_rewind(IMG__Ctx *ctx);
internal void img__bmp_parse_header(IMG__Ctx *ctx, IMG__BMP_Info *info);

internal IMG_Data img_data_from_file_path(Arena *arena, String8 path);

#endif // IMG_H
