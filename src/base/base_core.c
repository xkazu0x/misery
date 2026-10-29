////////////////////////////////
// NOTE: Safe Casts

internal u16
safe_cast_u16(u32 x) {
  assert_always(x <= max_u16);
  u16 result = (u16)x;
  return(result);
}

internal u32
safe_cast_u32(u64 x) {
  assert_always(x <= max_u32);
  u32 result = (u32)x;
  return(result);
}

internal s32
safe_cast_s32(s64 x) {
  assert_always(x <= max_s32);
  s32 result = (s32)x;
  return(result);
}

////////////////////////////////
// NOTE: Bit Patterns

#if COMPILER_MSVC || (COMPILER_CLANG && OS_WINDOWS)

internal u64
ctz32(u32 mask) {
  unsigned long idx;
  _BitScanForward(&idx, mask);
  return(idx);
}

internal u64
ctz64(u64 mask) {
  unsigned long idx;
  _BitScanForward64(&idx, mask);
  return(idx);
}

internal u64
clz32(u32 mask) {
  unsigned long idx;
  _BitScanReverse(&idx, mask);
  return(31 - idx);
}

internal u64
clz64(u64 mask) {
  unsigned long idx;
  _BitScanReverse64(&idx, mask);
  return(63 - idx);
}

#elif COMPILER_CLANG || COMPILER_GCC

internal u64
ctz32(u32 mask) {
  return(__builtin_ctz(mask));
}

internal u64
ctz64(u64 mask) {
  return(__builtin_ctzll(mask));
}

internal u64
clz32(u32 mask) {
  return(__builtin_clz(mask));
}

internal u64
clz64(u64 mask) {
  return(__builtin_clzll(mask));
}

#else
# error Bit instrinsic functions not defined for this compiler.
#endif

////////////////////////////////
// NOTE: Memory Functions

internal void
memory_write16(void *ptr, u16 v) {
  memory_copy(ptr, &v, sizeof(v));
}

internal void
memory_write32(void *ptr, u32 v) {
  memory_copy(ptr, &v, sizeof(v));
}

internal void
memory_write64(void *ptr, u64 v) {
  memory_copy(ptr, &v, sizeof(v));
}

internal u8
memory_read8(void *ptr) {
  u8 result = 0;
  memory_copy(&result, ptr, sizeof(result));
  return(result);
}

internal u16
memory_read16(void *ptr) {
  u16 result;
  memory_copy(&result, ptr, sizeof(result));
  return(result);
}

internal u32
memory_read32(void *ptr) {
  u32 result;
  memory_copy(&result, ptr, sizeof(result));
  return(result);
}

internal u64
memory_read64(void *ptr) {
  u64 result;
  memory_copy(&result, ptr, sizeof(result));
  return(result);
}

////////////////////////////////
// NOTE: Toolchain/Environment Enum Functions

internal u64
bit_size_from_arch(Arch arch) {
  u64 result = 0;
  switch (arch) {
    case Arch_X64:   result = 64; break;
    case Arch_X86:   result = 32; break;
    case Arch_ARM64: result = 64; break;
    case Arch_ARM32: result = 32; break;
    default: break;
  }
  return(result);
}

internal u64
byte_size_from_arch(Arch arch) {
  u64 result = bit_size_from_arch(arch)/8;
  return(result);
}

////////////////////////////////
// NOTE: Time Functions

internal Dense_Time
dense_time_from_date_time(Date_Time date_time){
  Dense_Time result = 0;
  result += date_time.year;
  result *= 12;
  result += date_time.mon;
  result *= 31;
  result += date_time.day;
  result *= 24;
  result += date_time.hour;
  result *= 60;
  result += date_time.min;
  result *= 61;
  result += date_time.sec;
  result *= 1000;
  result += date_time.msec;
  return(result);
}

internal Date_Time
date_time_from_dense_time(Dense_Time time){
  Date_Time result = {0};
  result.msec = time%1000;
  time /= 1000;
  result.sec  = time%61;
  time /= 61;
  result.min  = time%60;
  time /= 60;
  result.hour = time%24;
  time /= 24;
  result.day  = time%31;
  time /= 31;
  result.mon  = time%12;
  time /= 12;
  result.year = safe_cast_u32(time);
  return(result);
}
