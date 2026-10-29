#ifndef IMG_H
#define IMG_H

typedef struct Image_Context Image_Context;
struct Image_Context {
  String8 data;
  u8 *ptr, *opl;
};

typedef struct Image_Bmp_Info Image_Bmp_Info;
struct Image_Bmp_Info {
  u32 w, h, bpp;
  u32 off, hsz;
  u32 rm, gm, bm, am;
};

typedef struct Image Image;
struct Image {
  s32 w, h, n;
  void *data;
};

internal Image_Context img_ctx_make(String8 stream);
internal u8            img_ctx_read8(Image_Context *ctx);
internal u16           img_ctx_read16(Image_Context *ctx);
internal u32           img_ctx_read32(Image_Context *ctx);
internal void          img_ctx_rewind(Image_Context *ctx);

internal b32  img_bmp_check_and_rewind(Image_Context *ctx);
internal void img_bmp_parse_header(Image_Context *ctx, Image_Bmp_Info *info);

internal Image img_from_file_path(Arena *arena, String8 path);

#endif // IMG_H
