////////////////////////////////
// NOTE: Image Files

internal Image_File
img_file_from_data(String8 data) {
  Image_File result = {data, data.str, data.str + data.size};
  return(result);
}

internal u8
img_file_read8(Image_File *file) {
  u8 result = memory_read8(file->ptr);
  file->ptr += sizeof(result);
  return(result);
}

internal u16
img_file_read16(Image_File *file) {
  u16 result = memory_read16(file->ptr);
  file->ptr += sizeof(result);
  return(result);
}

internal u32
img_file_read32(Image_File *file) {
  u32 result = memory_read32(file->ptr);
  file->ptr += sizeof(result);
  return(result);
}

internal void
img_file_rewind(Image_File *file) {
  file->ptr = file->data.str;
  file->opl = file->ptr + file->data.size;
}

////////////////////////////////
// NOTE: Bmp Image Helpers

internal b32
bmp_img_file_check(Image_File *file) {
  if (img_file_read8(file) != 'B') invalid_path;
  if (img_file_read8(file) != 'M') invalid_path;
  img_file_read32(file); // discard filesize
  img_file_read16(file); // discard reserved
  img_file_read16(file); // discard reserved
  img_file_read32(file); // discard data offset
  u32 sz = img_file_read32(file);
  b32 result = (sz == 12 || sz == 40 || sz == 56 || sz == 108 || sz == 124);
  return(result);
}

internal b32
bmp_img_file_check_and_rewind(Image_File *file) {
  b32 result = bmp_img_file_check(file);
  img_file_rewind(file);
  return(result);
}

internal void
bmp_img_file_parse_header(Image_File *file, Bmp_Info *info) {
  u32 hsz;
  if (img_file_read8(file) != 'B') invalid_path;
  if (img_file_read8(file) != 'M') invalid_path;
  img_file_read32(file); // discard filesize
  img_file_read16(file); // discard reserved
  img_file_read16(file); // discard reserved
  info->off = img_file_read32(file);
  info->hsz = hsz = img_file_read32(file);
  info->rm = info->gm = info->bm = info->am = 0;
  if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124) invalid_path;
  if (hsz == 12) {
    info->w = img_file_read16(file);
    info->h = img_file_read16(file);
  } else {
    info->w = img_file_read32(file);
    info->h = img_file_read32(file);
  }
  if (img_file_read16(file) != 1) invalid_path;
  info->bpp = img_file_read16(file);
  if (hsz != 12) {
    u32 compress = img_file_read32(file);
    if (compress == 1 || compress == 2) invalid_path;
    if (compress == 3 && info->bpp != 16 && info->bpp != 32) invalid_path;
    if (compress >= 4) invalid_path;
    img_file_read32(file); // discard size of image
    img_file_read32(file); // discard hres
    img_file_read32(file); // discard vres
    img_file_read32(file); // discard colors used
    img_file_read32(file); // discard colors important
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
        info->rm = img_file_read32(file);
        info->gm = img_file_read32(file);
        info->bm = img_file_read32(file);
        info->am = img_file_read32(file);
      }
    }
    if (hsz == 108 || hsz == 124) {
      img_file_read32(file); // discard color space
      for (u32 idx = 0; idx < 12; idx += 1) {
        img_file_read32(file); // discard color space parameters
      }
      if (hsz == 124) {
        img_file_read32(file); // discard rendering intent
        img_file_read32(file); // discard offset of profile data
        img_file_read32(file); // discard size of profile data
        img_file_read32(file); // discard reserved
      }
    }
  }
}

////////////////////////////////
// NOTE: Images

internal Image
img_from_file_path(Arena *arena, String8 path) {
  Image result = {0};
  Temp scratch = scratch_begin(&arena, 1);
  String8 path_copy = str8_copy(scratch.arena, path);
  String8 file_data = data_from_file_path(scratch.arena, path_copy);
  if (file_data.size > 0) {
    Image_File file = img_file_from_data(file_data);
    if (bmp_img_file_check_and_rewind(&file)) {
      Bmp_Info info;
      bmp_img_file_parse_header(&file, &info);
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
  }
  return(result);
};
