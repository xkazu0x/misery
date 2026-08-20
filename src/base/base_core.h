#ifndef BASE_CORE_H
#define BASE_CORE_H

////////////////////////////////
// NOTE: Foreign Includes

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#if COMPILER_MSVC || (COMPILER_CLANG && OS_WINDOWS)
#include <intrin.h>
#endif

#include <stdlib.h>

#undef min
#undef max

////////////////////////////////
// NOTE: Codebase Keywords

#define internal static
#define global   static
#define local    static

#if COMPILER_MSVC
# define thread_static __declspec(thread)
#elif COMPILER_CLANG || COMPILER_GCC
# define thread_static __thread
#else
# error thread_static not defined for this compiler.
#endif

#if COMPILER_MSVC
# define force_inline __forceinline
#elif COMPILER_CLANG || COMPILER_GCC
# define force_inline __attribute__((always_inline))
#else
# error force_inline not defined for this compiler.
#endif

#if COMPILER_MSVC
# define no_inline __declspec(noinline)
#elif COMPILER_CLANG || COMPILER_GCC
# define no_inline __attribute__((noinline))
#else
# error no_inline not defined for this compiler.
#endif

////////////////////////////////
// NOTE: Linkage Keyword Macros

#if OS_WINDOWS
# define shared_function __declspec(dllexport)
#else
# define shared_function
#endif

////////////////////////////////
// NOTE: Units

#define KB(n) (((u64)(n))<<10)
#define MB(n) (((u64)(n))<<20)
#define GB(n) (((u64)(n))<<30)
#define TB(n) (((u64)(n))<<40)
#define thousand(n) ((n)*1000)
#define million(n)  ((n)*1000000)
#define billion(n)  ((n)*1000000000)

////////////////////////////////
// NOTE: Clamps, Mins, Maxes

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define clamp_top(a,x) min(a, x);
#define clamp_bot(x,b) max(x, b);
#define clamp(x,a,b) (((x)<(a))?(a):((x)>(b))?(b):(x))

////////////////////////////////
// NOTE: Type -> Alignment

#if COMPILER_MSVC
# define align_of(T) __alignof(T)
#elif COMPILER_CLANG
# define align_of(T) __alignof(T)
#elif COMPILER_GCC
# define align_of(T) __alignof__(T)
#else
# error aling_of not defined for this compiler.
#endif

#if COMPILER_MSVC
# define align_type(x) __declspec(align(x))
#elif COMPILER_CLANG || COMPILER_GCC
# define align_type(x) __attribute__((aligned(x)))
#else
# error align_type not defined for this compiler.
#endif

////////////////////////////////
// NOTE: Member Offsets

#define member_of(T,m)                (((T *)0)->m)
#define offset_of(T,m)                offsetof(T,m)
#define member_from_offset(T,ptr,off) (T)((((u8 *)ptr)+(off)))
#define member_from_ptr(T,ptr,m)      (void *)((((u8 *)ptr)+offset_of(T,m)))
#define cast_from_member(T,m,ptr)     (T *)((((u8 *)ptr)-offset_of(T,m)))

////////////////////////////////
// NOTE: Foor-Loop Construct Macros

#define for_each_index(it,c)                for (u64 it = 0; it < (c); it += 1)
#define for_each_element(it,a)              for (u64 it = 0; it < array_count(a); it += 1)
#define for_each_enum_value(T,it)           for (T it = (T)0; it < T##_COUNT; it = (T)(it+1))
#define for_each_non_zero_enum_value(T,it)  for (T it = (T)1; it < T##_COUNT; it = (T)(it+1))
#define for_each_node(T,it,first)           for (T *it = first; it != 0; it = it->next)

////////////////////////////////
// NOTE: Memory Operation Macros

#define memory_copy(d,s,z)        memmove((d),(s),(z))
#define memory_set(d,b,z)         memset((d),(b),(z))
#define memory_compare(a,b,z)     memcmp((a),(b),(z))

#define memory_copy_struct(d,s)   memory_copy((d),(s),sizeof(*(d)))
#define memory_copy_array(d,s)    memory_copy((d),(s),sizeof(d))
#define memory_copy_typed(d,s,c)  memory_copy((d),(s),sizeof(*(d))*(c))
#define memory_copy_str8(d,s)     memory_copy((d),(s).str,(s).size)

#define memory_zero(s,z)          memset((s),0,(z))
#define memory_zero_struct(s)     memory_zero((s),sizeof(*(s)))
#define memory_zero_array(a)      memory_zero((a),sizeof(a))
#define memory_zero_typed(m,c)    memory_zero((m),sizeof(*(m))*(c))

#define memory_match(a,b,z)      (memory_compare((a),(b),(z)) == 0)
#define memory_match_struct(a,b)  memory_match((a),(b),sizeof(*(a)))
#define memory_match_array(a,b)   memory_match((a),(b),sizeof(a))

////////////////////////////////
// NOTE: Asserts

#if COMPILER_MSVC
# define trap() __debugbreak()
#elif COMPILER_CLANG || COMPILER_GCC
# define trap() __builtin_trap()
#else
# error Unknown trap intrinsic for this compiler.
#endif

#define assert_always(x) do{if(!(x)){trap();}}while(0)
#if BUILD_DEBUG
# define assert(x) assert_always(x)
#else
# define assert(x) (void)(x)
#endif
#define invalid_path    assert(!"Invalid Path!")
#define not_implemented assert(!"Not Implemented!")
#define static_assert(C,ID) global u8 glue(ID, __LINE__)[(C)?1:-1]

////////////////////////////////
// NOTE: Linked List Building Macros

// NOTE: linked list macro helpers
#define check_nil(nil,p)  ((p) == 0 || (p) == nil)
#define set_nil(nil,p)    ((p) = nil)

// NOTE: doubly-linked lists
#define dll_insert_npz(nil,f,l,p,n,next,prev)\
(check_nil(nil,f))?\
((f)=(l)=(n),set_nil(nil,(n)->next),set_nil(nil,(n)->prev)):\
(check_nil(nil,p))?\
((n)->next=(f),(f)->prev=(n),(f)=(n),set_nil(nil,(n)->prev)):\
((p)==(l))?\
((l)->next=(n),(n)->prev=(l),(l)=(n),set_nil(nil,(n)->next)):\
(((!check_nil(nil,p) && check_nil(nil,(p)->next)) ? (0) : ((p)->next->prev=(n))), ((n)->next=(p)->next),((p)->next=(n)),((n)->prev=(p)))
#define dll_push_back_npz(nil,f,l,n,next,prev) dll_insert_npz(nil,f,l,l,n,next,prev)
#define dll_push_front_npz(nil,f,l,n,next,prev) dll_insert_npz(nil,l,f,f,n,next,prev)
#define dll_remove_npz(nil,f,l,n,next,prev)\
(((n)==(f) ? (f)=(n)->next : (0)),\
((n)==(l) ? (l)=(l)->prev : (0)),\
(check_nil(nil,(n)->prev) ? (0):\
((n)->prev->next=(n)->next)),\
(check_nil(nil,(n)->next) ? (0):\
((n)->next->prev=(n)->prev)))

// NOTE: singly-linked, doubly-headed lists (queues)
#define sll_queue_push_nz(nil,f,l,n,next)\
(check_nil(nil,(f))?\
((f)=(l)=(n),set_nil(nil,(n)->next)):\
((l)->next=(n),(l)=(n),set_nil(nil,(n)->next)))
#define sll_queue_push_front_nz(nil,f,l,n,next)\
(check_nil(nil,f)?\
((f)=(l)=(n),set_nil(nil,(n)->next)):\
((n)->next=(f),(f)=(n)))
#define sll_queue_pop_nz(nil,f,l,next)\
((f)==(l)?\
(set_nil(nil,f),set_nil(nil,l)):\
((f)=(f)->next))

// NOTE: singly-linked, singly-headed lists (stacks)
#define sll_stack_push_n(f,n,next) ((n)->next=(f),(f)=(n))
#define sll_stack_pop_n(f,next) ((f)=(f)->next)

// NOTE: doubly-linked lists helpers
#define dll_insert_np(f,l,p,n,next,prev)    dll_insert_npz(0,f,l,p,n,next,prev)
#define dll_push_back_np(f,l,n,next,prev)   dll_push_back_npz(0,f,l,n,next,prev)
#define dll_push_front_np(f,l,n,next,prev)  dll_push_front_npz(0,f,l,n,next,prev)
#define dll_remove_np(f,l,n,next,prev)      dll_remove_npz(0,f,l,n,next,prev)
#define dll_insert(f,l,p,n)                 dll_insert_npz(0,f,l,p,n,next,prev)
#define dll_push_back(f,l,n)                dll_push_back_npz(0,f,l,n,next,prev)
#define dll_push_front(f,l,n)               dll_push_front_npz(0,f,l,n,next,prev)
#define dll_remove(f,l,n)                   dll_remove_npz(0,f,l,n,next,prev)

// NOTE: singly-linked, doubly-headed lists helpers
#define sll_queue_push_n(f,l,n,next)        sll_queue_push_nz(0,f,l,n,next)
#define sll_queue_push_front_n(f,l,n,next)  sll_queue_push_front_nz(0,f,l,n,next)
#define sll_queue_pop_n(f,l,next)           sll_queue_pop_nz(0,f,l,next)
#define sll_queue_push(f,l,n)               sll_queue_push_nz(0,f,l,n,next)
#define sll_queue_push_front(f,l,n)         sll_queue_push_front_nz(0,f,l,n,next)
#define sll_queue_pop(f,l)                  sll_queue_pop_nz(0,f,l,next)

// NOTE: singly-linked, singly-headed lists helpers
#define sll_stack_push(f,n)                 sll_stack_push_n(f,n,next);
#define sll_stack_pop(f)                    sll_stack_pop_n(f,next);

////////////////////////////////
// NOTE: Address Sanitizer Markup

#if COMPILER_MSVC
# if defined(__SANITIZE_ADDRESS__)
#  define ASAN_ENABLED 1
#  define ASAN_NO_ADDR __declspec(no_sanitize_address)
# endif
#elif COMPILER_CLANG
# if defined(__has_feature)
#  if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#   define ASAN_ENABLED 1
#  endif
# endif
# define ASAN_NO_ADDR   __attribute__((no_sanitize("address")))
# define UBSAN_NO_ALIGN __attribute__((no_sanitize("alignment")))
#endif

#ifndef  ASAN_NO_ADDR
# define ASAN_NO_ADDR
#endif
#ifndef  UBSAN_NO_ALIGN
# define UBSAN_NO_ALIGN
#endif

#if ASAN_ENABLED
void __asan_poison_memory_region(void const volatile *addr, size_t size);
void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
# define asan_poison_memory_region(addr, size)   __asan_poison_memory_region((addr), (size))
# define asan_unpoison_memory_region(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
# define asan_poison_memory_region(addr, size)   ((void)(addr), (void)(size))
# define asan_unpoison_memory_region(addr, size) ((void)(addr), (void)(size))
#endif

////////////////////////////////
// NOTE: Misc. Helper Macros

#define stringfy_(s) #s
#define stringfy(s) stringfy_(s)

#define glue_(a, b) a##b
#define glue(a, b) glue_(a, b)

#define array_count(a) (sizeof(a)/sizeof(*(a)))
#define swap_t(T,a,b) do{T __t = (a); (a) = (b); (b) = __t;}while(0)
#define sign_t(T,x) ((T)((x) > 0) - (T)((x) < 0))
#define abs_t(T,x) (sign_t(T,x)*(x))

#if ARCH_64BIT
#define int_from_ptr(ptr) ((u64)(ptr))
#elif ARCH_32BIT
#define int_from_ptr(ptr) ((u32)(ptr))
#else
#error Missing pointer-to-integer cast for this architecture.
#endif
#define ptr_from_int(i) (void *)(i)

#define compose_64bit(hi, lo) ((((u64)hi) << 32) | ((u64)lo))
#define compose_32bit(hi, lo) ((((u32)hi) << 16) | ((u32)lo))
#define align_up_pow2(x, b)   (((x) + (b) - 1)&(~((b) - 1)))
#define align_down_pow2(x, b) ((x)&(~((b) - 1)))

#define extract1(word,idx)  (((word) >> (idx)) & 1)
#define extract8(word,pos)  (((word) >> ((pos)*8)) & max_u8)
#define extract16(word,pos) (((word) >> ((pos)*16)) & max_u16)
#define extract32(word,pos) (((word) >> ((pos)*32)) & max_u32)

////////////////////////////////
// NOTE: Base Types

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;
typedef s8        b8;
typedef s16       b16;
typedef s32       b32;
typedef s64       b64;
typedef float     f32;
typedef double    f64;
typedef void void_proc(void);

////////////////////////////////
// NOTE: Toolchain/Environment Enums

typedef enum Operating_System {
  Operating_System_NULL,
  Operating_System_WINDOWS,
  Operating_System_LINUX,
  Operating_System_MAC,
  Operating_System_COUNT,
} Operating_System;
#if OS_WINDOWS
# define Operating_System_CURRENT Operating_System_WINDOWS
#elif OS_LINUX
# define Operating_System_CURRENT Operating_System_LINUX
#elif OS_MAC
# define Operating_System_CURRENT Operating_System_MAC
#else
# define Operating_System_CURRENT Operating_System_NULL
#endif

typedef enum Architecture {
  Architecture_NULL,
  Architecture_X64,
  Architecture_X86,
  Architecture_ARM64,
  Architecture_ARM32,
  Architecture_COUNT,
} Architecture;
#if ARCH_X64
# define Arch_CURRENT Architecture_X64
#elif ARCH_X86
# define Arch_CURRENT Architecture_X86
#elif ARCH_ARM64
# define Arch_CURRENT Architecture_ARM64
#elif ARCH_ARM32
# define Arch_CURRENT Architecture_ARM32
#else
# define Arch_CURRENT Architecture_NULL
#endif

typedef enum Compiler {
  Compiler_NULL,
  Compiler_MSVC,
  Compiler_GCC,
  Compiler_CLANG,
  Compiler_COUNT,
} Compiler;
#if COMPILER_MSVC
# define Compiler_CURRENT Compiler_MSVC
#elif COMPILER_GCC
# define Compiler_CURRENT Compiler_GCC
#elif COMPILER_CLANG
# define Compiler_CURRENT Compiler_CLANG
#else
# define Compiler_CURRENT Compiler_NULL
#endif

////////////////////////////////
// NOTE: Basic Constants

global u32 sign32     = 0x80000000;
global u32 exponent32 = 0x7F800000;
global u32 mantissa32 = 0x007FFFFF;

global f32   big_golden32 = 1.61803398875f;
global f32 small_golden32 = 0.61803398875f;

global f32 pi32 = 3.1415926535897f;

global f64 machine_epsilon64 = 4.94065645841247e-324;

global u64 max_u64 = 0xffffffffffffffffull;
global u32 max_u32 = 0xffffffff;
global u16 max_u16 = 0xffff;
global u8  max_u8  = 0xff;

global s64 max_s64 = (s64)0x7fffffffffffffffll;
global s32 max_s32 = (s32)0x7fffffff;
global s16 max_s16 = (s16)0x7fff;
global s8  max_s8  =  (s8)0x7f;

global s64 min_s64 = (s64)0x8000000000000000ll;
global s32 min_s32 = (s32)0x80000000;
global s16 min_s16 = (s16)0x8000;
global s8  min_s8  =  (s8)0x80;

global const u32 bitmask1  = 0x00000001u;
global const u32 bitmask2  = 0x00000003u;
global const u32 bitmask3  = 0x00000007u;
global const u32 bitmask4  = 0x0000000fu;
global const u32 bitmask5  = 0x0000001fu;
global const u32 bitmask6  = 0x0000003fu;
global const u32 bitmask7  = 0x0000007fu;
global const u32 bitmask8  = 0x000000ffu;
global const u32 bitmask9  = 0x000001ffu;
global const u32 bitmask10 = 0x000003ffu;
global const u32 bitmask11 = 0x000007ffu;
global const u32 bitmask12 = 0x00000fffu;
global const u32 bitmask13 = 0x00001fffu;
global const u32 bitmask14 = 0x00003fffu;
global const u32 bitmask15 = 0x00007fffu;
global const u32 bitmask16 = 0x0000ffffu;
global const u32 bitmask17 = 0x0001ffffu;
global const u32 bitmask18 = 0x0003ffffu;
global const u32 bitmask19 = 0x0007ffffu;
global const u32 bitmask20 = 0x000fffffu;
global const u32 bitmask21 = 0x001fffffu;
global const u32 bitmask22 = 0x003fffffu;
global const u32 bitmask23 = 0x007fffffu;
global const u32 bitmask24 = 0x00ffffffu;
global const u32 bitmask25 = 0x01ffffffu;
global const u32 bitmask26 = 0x03ffffffu;
global const u32 bitmask27 = 0x07ffffffu;
global const u32 bitmask28 = 0x0fffffffu;
global const u32 bitmask29 = 0x1fffffffu;
global const u32 bitmask30 = 0x3fffffffu;
global const u32 bitmask31 = 0x7fffffffu;
global const u32 bitmask32 = 0xffffffffu;

global const u64 bitmask33 = 0x00000001ffffffffull;
global const u64 bitmask34 = 0x00000003ffffffffull;
global const u64 bitmask35 = 0x00000007ffffffffull;
global const u64 bitmask36 = 0x0000000fffffffffull;
global const u64 bitmask37 = 0x0000001fffffffffull;
global const u64 bitmask38 = 0x0000003fffffffffull;
global const u64 bitmask39 = 0x0000007fffffffffull;
global const u64 bitmask40 = 0x000000ffffffffffull;
global const u64 bitmask41 = 0x000001ffffffffffull;
global const u64 bitmask42 = 0x000003ffffffffffull;
global const u64 bitmask43 = 0x000007ffffffffffull;
global const u64 bitmask44 = 0x00000fffffffffffull;
global const u64 bitmask45 = 0x00001fffffffffffull;
global const u64 bitmask46 = 0x00003fffffffffffull;
global const u64 bitmask47 = 0x00007fffffffffffull;
global const u64 bitmask48 = 0x0000ffffffffffffull;
global const u64 bitmask49 = 0x0001ffffffffffffull;
global const u64 bitmask50 = 0x0003ffffffffffffull;
global const u64 bitmask51 = 0x0007ffffffffffffull;
global const u64 bitmask52 = 0x000fffffffffffffull;
global const u64 bitmask53 = 0x001fffffffffffffull;
global const u64 bitmask54 = 0x003fffffffffffffull;
global const u64 bitmask55 = 0x007fffffffffffffull;
global const u64 bitmask56 = 0x00ffffffffffffffull;
global const u64 bitmask57 = 0x01ffffffffffffffull;
global const u64 bitmask58 = 0x03ffffffffffffffull;
global const u64 bitmask59 = 0x07ffffffffffffffull;
global const u64 bitmask60 = 0x0fffffffffffffffull;
global const u64 bitmask61 = 0x1fffffffffffffffull;
global const u64 bitmask62 = 0x3fffffffffffffffull;
global const u64 bitmask63 = 0x7fffffffffffffffull;
global const u64 bitmask64 = 0xffffffffffffffffull;

global const u32 bit1  = (1u<<0);
global const u32 bit2  = (1u<<1);
global const u32 bit3  = (1u<<2);
global const u32 bit4  = (1u<<3);
global const u32 bit5  = (1u<<4);
global const u32 bit6  = (1u<<5);
global const u32 bit7  = (1u<<6);
global const u32 bit8  = (1u<<7);
global const u32 bit9  = (1u<<8);
global const u32 bit10 = (1u<<9);
global const u32 bit11 = (1u<<10);
global const u32 bit12 = (1u<<11);
global const u32 bit13 = (1u<<12);
global const u32 bit14 = (1u<<13);
global const u32 bit15 = (1u<<14);
global const u32 bit16 = (1u<<15);
global const u32 bit17 = (1u<<16);
global const u32 bit18 = (1u<<17);
global const u32 bit19 = (1u<<18);
global const u32 bit20 = (1u<<19);
global const u32 bit21 = (1u<<20);
global const u32 bit22 = (1u<<21);
global const u32 bit23 = (1u<<22);
global const u32 bit24 = (1u<<23);
global const u32 bit25 = (1u<<24);
global const u32 bit26 = (1u<<25);
global const u32 bit27 = (1u<<26);
global const u32 bit28 = (1u<<27);
global const u32 bit29 = (1u<<28);
global const u32 bit30 = (1u<<29);
global const u32 bit31 = (1u<<30);
global const u32 bit32 = (1u<<31);

global const u64 bit33 = (1ull<<32);
global const u64 bit34 = (1ull<<33);
global const u64 bit35 = (1ull<<34);
global const u64 bit36 = (1ull<<35);
global const u64 bit37 = (1ull<<36);
global const u64 bit38 = (1ull<<37);
global const u64 bit39 = (1ull<<38);
global const u64 bit40 = (1ull<<39);
global const u64 bit41 = (1ull<<40);
global const u64 bit42 = (1ull<<41);
global const u64 bit43 = (1ull<<42);
global const u64 bit44 = (1ull<<43);
global const u64 bit45 = (1ull<<44);
global const u64 bit46 = (1ull<<45);
global const u64 bit47 = (1ull<<46);
global const u64 bit48 = (1ull<<47);
global const u64 bit49 = (1ull<<48);
global const u64 bit50 = (1ull<<49);
global const u64 bit51 = (1ull<<50);
global const u64 bit52 = (1ull<<51);
global const u64 bit53 = (1ull<<52);
global const u64 bit54 = (1ull<<53);
global const u64 bit55 = (1ull<<54);
global const u64 bit56 = (1ull<<55);
global const u64 bit57 = (1ull<<56);
global const u64 bit58 = (1ull<<57);
global const u64 bit59 = (1ull<<58);
global const u64 bit60 = (1ull<<59);
global const u64 bit61 = (1ull<<60);
global const u64 bit62 = (1ull<<61);
global const u64 bit63 = (1ull<<62);
global const u64 bit64 = (1ull<<63);

////////////////////////////////
// NOTE: Time Types

typedef struct Date_Time Date_Time;
struct Date_Time {
  u16 usec;  // [0,999]
  u16 msec;  // [0,999]
  u16 sec;   // [0,60]
  u16 min;   // [0,59]
  u16 hour;  // [0,24]
  u16 day;
  u32 mon;
  u32 year;  // 1 = 1 CE, 0 = 1 BC
};

typedef u64 Dense_Time;

////////////////////////////////
// NOTE: Safe Casts

internal u16 safe_cast_u16(u32 x);
internal u32 safe_cast_u32(u64 x);
internal s32 safe_cast_s32(s64 x);

////////////////////////////////
// NOTE: Bit Patterns

internal u64 ctz32(u32 mask);
internal u64 ctz64(u64 mask);
internal u64 clz32(u32 mask);
internal u64 clz64(u64 mask);

////////////////////////////////
// NOTE: Toolchain/Environment Enum Functions

internal u64 bit_size_from_arch(Architecture arch);
internal u64 byte_size_from_arch(Architecture arch);

////////////////////////////////
// NOTE: Time Functions

internal Dense_Time dense_time_from_date_time(Date_Time date_time);
internal Date_Time  date_time_from_dense_time(Dense_Time dense_time);

////////////////////////////////
// NOTE: @per_os_impl Time

internal u64  get_time_us(void);
internal void sleep_ms(u32 ms);

#endif // BASE_CORE_H
