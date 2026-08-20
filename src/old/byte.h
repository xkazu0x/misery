#ifndef BYTE_H
#define BYTE_H

////////////////////////////////
// NOTE: Byte Stream

typedef struct Byte_Stream Byte_Stream;
struct Byte_Stream {
  String8 data;
  u8 *ptr, *opl;
};

////////////////////////////////
// NOTE: Byte Stream Functions

// NOTE: create/reset stream
internal Byte_Stream byte_stream_make(String8 data);
internal void byte_stream_reset(Byte_Stream *s);

// NOTE: read byte from stream
internal u8  byte_8(Byte_Stream *s);
internal u16 byte_get16_le(Byte_Stream *s);
internal u32 byte_get32_le(Byte_Stream *s);

#endif // BYTE_H
