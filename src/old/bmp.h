#ifndef BMP_H
#define BMP_H

////////////////////////////////
// NOTE: BMP Types

typedef struct BMP_Info BMP_Info;
struct BMP_Info {
  u32 w, h, bpp;
  u32 off, hsz;
  u32 r_mask, g_mask, b_mask, a_mask;
};

////////////////////////////////
// NOTE: BMP Functions

internal b32      bmp_test(Byte_Stream *s);
internal BMP_Info bmp_parse(Byte_Stream *s);
internal void    *bmp_data_from_file_path(Arena *arena, String8 path, u32 *out_w, u32 *out_h);

#endif // BMP_H
