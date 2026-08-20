////////////////////////////////
// NOTE: BMP Functions

internal b32
bmp_test(Byte_Stream *s) {
  if (byte_get8(s) != 'B') invalid_path;
  if (byte_get8(s) != 'M') invalid_path;
  byte_get32_le(s); // discard filesize
  byte_get16_le(s); // discard reserved
  byte_get16_le(s); // discard reserved
  byte_get32_le(s); // discard data offset
  u32 sz = byte_get32_le(s);
  b32 r = (sz == 12 || sz == 40 || sz == 56 || sz == 108 || sz == 124);
  byte_stream_reset(s);
  return(r);
}

internal BMP_Info
bmp_parse(Byte_Stream *s) {
  BMP_Info info = {0};
  u32 hsz;
  if (byte_get8(s) != 'B') invalid_path;
  if (byte_get8(s) != 'M') invalid_path;
  byte_get32_le(s); // discard filesize
  byte_get16_le(s); // discard reserved
  byte_get16_le(s); // discard reserved
  info.off = byte_get32_le(s);
  info.hsz = hsz = byte_get32_le(s);
  info.r_mask = info.g_mask = info.b_mask = info.a_mask = 0;
  if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124) invalid_path;
  if (hsz == 12) {
    info.w = byte_get16_le(s);
    info.h = byte_get16_le(s);
  } else {
    info.w = byte_get32_le(s);
    info.h = byte_get32_le(s);
  }
  if (byte_get16_le(s) != 1) invalid_path;
  info.bpp = byte_get16_le(s);
  if (hsz != 12) {
    u32 compress = byte_get32_le(s);
    if (compress == 1 || compress == 2) invalid_path;
    if (compress == 3 && info.bpp != 16 && info.bpp != 32) invalid_path;
    if (compress >= 4) invalid_path;
    byte_get32_le(s); // discard size of image
    byte_get32_le(s); // discard hres
    byte_get32_le(s); // discard vres
    byte_get32_le(s); // discard colors used
    byte_get32_le(s); // discard colors important
    if (hsz == 40) {
      if (info.bpp == 16) {
        info.r_mask = bitmask5 << 11;
        info.g_mask = bitmask6 << 5;
        info.b_mask = bitmask5 << 0;
      } else if (info.bpp == 32) {
        info.r_mask = bitmask8 << 16;
        info.g_mask = bitmask8 << 8;
        info.b_mask = bitmask8 << 0;
        info.a_mask = bitmask8 << 24;
      }
    }
    if (hsz == 56 || hsz == 108 || hsz == 124) {
      if (compress == 0) {
        if (info.bpp == 16) {
          info.r_mask = bitmask5 << 11;
          info.g_mask = bitmask6 << 5;
          info.b_mask = bitmask5 << 0;
        } else if (info.bpp == 32) {
          info.r_mask = bitmask8 << 16;
          info.g_mask = bitmask8 << 8;
          info.b_mask = bitmask8 << 0;
          info.a_mask = bitmask8 << 24;
        }
      } else if (compress == 3) {
        info.r_mask = byte_get32_le(s);
        info.g_mask = byte_get32_le(s);
        info.b_mask = byte_get32_le(s);
        info.a_mask = byte_get32_le(s);
      }
    }
    if (hsz == 108 || hsz == 124) {
      byte_get32_le(s); // discard color space
      for (u32 i = 0; i < 12; i += 1) {
        byte_get32_le(s); // discard color space parameters
      }
      if (hsz == 124) {
        byte_get32_le(s); // discard rendering intent
        byte_get32_le(s); // discard offset of profile data
        byte_get32_le(s); // discard size of profile data
        byte_get32_le(s); // discard reserved
      }
    }
  }
  return(info);
}

internal void *
bmp_data_from_file_path(Arena *arena, String8 path, u32 *out_w, u32 *out_h) {
  void *data = 0;
  Temp scratch = scratch_begin(&arena, 1);
  String8 file_data = data_from_file_path(scratch.arena, path);
  if (file_data.size > 0) {
    Byte_Stream stream = byte_stream_make(file_data);
    if (!bmp_test(&stream)) invalid_path;
    BMP_Info info = bmp_parse(&stream);
    if (info.bpp != 32) invalid_path;
    u64 sz = info.w*info.h*(info.bpp/8);
    *out_w = info.w;
    *out_h = info.h;
    data = push_array(arena, u8, sz);
    u32 line_size = info.w*info.bpp/8;
    u8 *src_line = file_data.str + info.off;
    u8 *dst_line = (u8 *)data + (info.h - 1)*line_size;
    for (u32 y = 0; y < info.h; y += 1) {
      memory_copy(dst_line, src_line, line_size);
      dst_line -= line_size;
      src_line += line_size;
    }
  }
  return(data);
};
