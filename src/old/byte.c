////////////////////////////////
// NOTE: Byte Stream Functions

// NOTE: create/reset stream

internal Byte_Stream
byte_stream_make(String8 data) {
  Byte_Stream result = {0};
  result.data = data;
  result.ptr = data.str;
  result.opl = data.str + data.size;
  return(result);
}

internal void
byte_stream_reset(Byte_Stream *s) {
  s->ptr = s->data.str;
  s->opl = s->ptr + s->data.size;
}

// NOTE: read byte from stream

internal u8
byte_get8(Byte_Stream *s) {
  u8 r = 0;
  if (s->ptr < s->opl) {
    r = *s->ptr++;
  }
  return(r);
}

internal u16
byte_get16_le(Byte_Stream *s) {
  u8 lo = byte_get8(s);
  u8 hi = byte_get8(s);
  u16 r = (hi << 8) | lo;
  return(r);
}

internal u32
byte_get32_le(Byte_Stream *s) {
  u16 lo = byte_get16_le(s);
  u16 hi = byte_get16_le(s);
  u32 r = (hi << 16) | lo;
  return(r);
}
