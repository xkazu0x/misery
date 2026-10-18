#ifndef IMG_H
#define IMG_H

typedef struct IMG__Context IMG__Context;
struct IMG__Context {
  String8 stream;
  u8 *ptr, *opl;
};

typedef struct IMG__BMP_Info IMG__BMP_Info;
struct IMG__BMP_Info {
  u32 w, h, bpp;
  u32 off, hsz;
  u32 rm, gm, bm, am;
};

typedef struct IMG_Info IMG_Info;
struct IMG_Info {
  void *data;
  s32 w, h, n;
};

internal IMG__Context img__ctx_make(String8 stream);
internal void img__ctx_rewind(IMG__Context *ctx);
internal u8 img__ctx_get8(IMG__Context *ctx);
internal u16 img__ctx_get16_le(IMG__Context *ctx);
internal u32 img__ctx_get32_le(IMG__Context *ctx);

internal b32 img__bmp_test(IMG__Context *ctx);
internal void img__bmp_parse_header(IMG__Context *ctx, IMG__BMP_Info *info);

internal IMG_Info img_load(Arena *arena, String8 path);

#endif // IMG_H
