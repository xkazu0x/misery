#ifndef IMAGE_H
#define IMAGE_H

typedef struct Bmp_Info Bmp_Info;
struct Bmp_Info {
  u32 w, h, bpp;
  u32 off, hsz;
  u32 rm, gm, bm, am;
};

typedef struct Image_File Image_File;
struct Image_File {
  String8 data;
  u8 *ptr, *opl;
};

typedef struct Image Image;
struct Image {
  s32 w, h, n;
  void *data;
};

////////////////////////////////
// NOTE: Image Files

internal Image_File img_file_from_data(String8 data);
internal u8         img_file_read8(Image_File *file);
internal u16        img_file_read16(Image_File *file);
internal u32        img_file_read32(Image_File *file);
internal void       img_file_rewind(Image_File *file);

////////////////////////////////
// NOTE: Bmp Image Helpers

internal b32  bmp_img_file_check(Image_File *file);
internal b32  bmp_img_file_check_and_rewind(Image_File *file);
internal void bmp_img_file_parse_header(Image_File *file, Bmp_Info *info);

////////////////////////////////
// NOTE: Images

internal Image img_from_file_path(Arena *arena, String8 path);

#endif // IMAGE_H
