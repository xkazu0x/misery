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
// NOTE: Toolchain/Environment Enum Functions

internal u64
bit_size_from_architecture(Architecture arch) {
  u64 arch_bitsize = 0;
  switch (arch) {
    case Architecture_X64:   arch_bitsize = 64; break;
    case Architecture_X86:   arch_bitsize = 32; break;
    case Architecture_ARM64: arch_bitsize = 64; break;
    case Architecture_ARM32: arch_bitsize = 32; break;
    default: break;
  }
  return(arch_bitsize);
}

internal u64
byte_size_from_arch(Architecture arch) {
  return(bit_size_from_architecture(arch)/8);
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
  assert(time <= max_u32);
  result.year = (u32)time;
  return(result);
}
