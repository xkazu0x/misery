#ifndef BASE_STRING_H
#define BASE_STRING_H

////////////////////////////////
// NOTE: String Types

typedef struct String8 String8;
struct String8 {
  u8 *str;
  u64 size;
};

typedef struct String16 String16;
struct String16 {
  u16 *str;
  u64 size;
};

typedef struct String32 String32;
struct String32 {
  u32 *str;
  u64 size;
};

////////////////////////////////
// NOTE: String List & Array Types

typedef struct String8_Node String8_Node;
struct String8_Node {
  String8_Node *next;
  String8 v;
};

typedef struct String8_List String8_List;
struct String8_List {
  String8_Node *first;
  String8_Node *last;
  u64 count;
  u64 size;
};

typedef struct String8_Array String8_Array;
struct String8_Array {
  String8 *v;
  u64 count;
};

////////////////////////////////
// NOTE: String Matching, Splitting & Joining Types

typedef u32 String_Match_Flags;
enum {
  String_Match_Flag_CASE_INSENSITIVE  = (1<<0),
  String_Match_Flag_SLASH_INSENSITIVE = (1<<1),
  String_Match_Flag_RIGHT_SIDE_SLOPPY = (1<<2),
};

typedef u32 String_Split_Flags;
enum {
  String_Split_Flag_KEEP_EMPTIES = (1<<0),
};

// typedef enum Path_Style {
//   PATH_STYLE_NULL,
//   PATH_STYLE_RELATIVE,
//   PATH_STYLE_WINDOWS_ABSOLUTE,
//   PATH_STYLE_UNIX_ABSOLUTE,
// #if OS_WINDOWS
//   PATH_STYLE_SYSTEM_ABSOLUTE = PATH_STYLE_WINDOWS_ABSOLUTE,
// #elif OS_LINUX
//   PATH_STYLE_SYSTEM_ABSOLUTE = PATH_STYLE_UNIX_ABSOLUTE,
// #else
//   #error Absolute path style is undefined for this OS.
//   #endif
// } Path_Style;

typedef struct String_Join String_Join;
struct String_Join {
  String8 pre;
  String8 sep;
  String8 post;
};

////////////////////////////////
// NOTE: Unicode Types

typedef struct Unicode_Decode Unicode_Decode;
struct Unicode_Decode {
  u32 increment;
  u32 codepoint;
};

////////////////////////////////
// NOTE: Character Classification & Conversion Functions

internal b32 char_is_space(u8 c);
internal b32 char_is_upper(u8 c);
internal b32 char_is_lower(u8 c);
internal b32 char_is_alpha(u8 c);
internal b32 char_is_slash(u8 c);
internal b32 char_is_digit(u8 c);
internal u8 lower_from_char(u8 c);
internal u8 upper_from_char(u8 c);
internal u8 correct_slash_from_char(u8 c);

////////////////////////////////
// NOTE: C-String Measurement

internal u64 cstr8_len(u8 *cstr);
internal u64 cstr16_len(u16 *cstr);
internal u64 cstr32_len(u32 *cstr);

////////////////////////////////
// NOTE: String Constructors

#define s(s) str8_lit(s)

#define str8_lit(s) str8_make((u8 *)(s), sizeof(s) - 1)
#define str8_lit_comp(s) {(u8 *)(s), sizeof(s) - 1}
#define str8_lit_cstr(s) str8_make((u8 *)(s), sizeof(s))
#define str8_fmt(s) (int)((s).size), ((s).str)

internal String8 str8_zero(void);
internal String8 str8_make(u8 *str, u64 size);
internal String8 str8_range(u8 *first, u8 *opl);
internal String8 str8_cstr(char *cstr);

internal String16 str16_zero(void);
internal String16 str16_make(u16 *data, u64 size);
internal String16 str16_range(u16 *first, u16 *opl);
internal String16 str16_cstr(u16 *cstr);

internal String32 str32_zero(void);
internal String32 str32_make(u32 *data, u64 size);
internal String32 str32_range(u32 *first, u32 *opl);
internal String32 str32_cstr(u32 *cstr);

////////////////////////////////
// NOTE: String Stylization

internal String8 upper_from_str8(Arena *arena, String8 s);
internal String8 lower_from_str8(Arena *arena, String8 s);
internal String8 backslashed_from_str8(Arena *arena, String8 s);
internal String8 forwardslashed_from_str8(Arena *arena, String8 s);

////////////////////////////////
// NOTE: String Matching

internal b32 str8_match(String8 a, String8 b, String_Match_Flags flags);
internal b32 str8_char_match(u8 a, u8 b, String_Match_Flags flags);
internal u64 str8_find_needle(String8 s, String8 needle, String_Match_Flags flags);

#define str8_match_lit(a_lit, b)   str8_match(str8_lit(a_lit), (b))
#define str8_match_cstr(a_cstr, b) str8_match(str8_cstr(a_cstr), (b))
#define str8_matchi(a, b) str8_match(a, b, String_Match_Flag_CASE_INSENSITIVE)
#define str8_starts_with(s, start)  str8_match(str8_prefix((s), (start).size), (start), 0)
#define str8_starts_withi(s, start) str8_match(str8_prefix((s), (start).size), (start), String_Match_Flag_CASE_INSENSITIVE)
#define str8_ends_with(s, end)  str8_match(str8_postfix((s), (end).size), (end), 0)
#define str8_ends_withi(s, end) str8_match(str8_postfix((s), (end).size), (end), String_Match_Flag_CASE_INSENSITIVE)
#define str8_char_matchi(a, b) str8_char_match(a, b, String_Match_Flag_CASE_INSENSITIVE)


////////////////////////////////
// NOTE: String Slicing

internal String8 str8_substr(String8 s, u64 min, u64 max);
internal String8 str8_prefix(String8 s, u64 size);
internal String8 str8_skip(String8 s, u64 amt);
internal String8 str8_postfix(String8 s, u64 size);
internal String8 str8_chop(String8 s, u64 amt);

////////////////////////////////
// NOTE: String Formatting & Copying

internal String8 str8_cat(Arena *arena, String8 a, String8 b);
internal String8 str8_copy(Arena *arena, String8 s);
internal String8 str8fv(Arena *arena, char *fmt, va_list args);
internal String8 str8f(Arena *arena, char *fmt, ...);

////////////////////////////////
// NOTE: String List

internal String8_Node *str8_list_push_node(String8_List *list, String8_Node *n);
internal String8_Node *str8_list_push_node_set_string(String8_List *list, String8_Node *n, String8 s);
internal String8_Node *str8_list_push(Arena *arena, String8_List *list, String8 s);
internal String8_Node *str8_list_pushf(Arena *arena, String8_List *list, char *fmt, ...);
internal String8_Node *str8_list_push_node_front(String8_List *list, String8_Node *n);
internal String8_Node *str8_list_push_node_front_set_string(String8_List *list, String8_Node *n, String8 s);
internal String8_Node *str8_list_push_front(Arena *arena, String8_List *list, String8 s);
internal String8_Node *str8_list_push_frontf(Arena *arena, String8_List *list, char *fmt, ...);
internal String8_Node *str8_list_pop_front(String8_List *list);
internal String8_List  str8_list_copy(Arena *arena, String8_List *list);
internal void          str8_list_concat_in_place(String8_List *list, String8_List *to_push);

////////////////////////////////
// NOTE: String Array

internal String8_Array str8_array_zero(void);
internal String8_Array str8_array_from_list(Arena *arena, String8_List *list);

////////////////////////////////
// NOTE: String Splitting & Joining

internal String8_List str8_split(Arena *arena, String8 s, u8 *split_chars, u64 num_split_chars, String_Split_Flags flags);
internal String8_List str8_split_by_string_chars(Arena *arena, String8 s, String8 split_chars, String_Split_Flags flags);
internal String8      str8_list_join(Arena *arena, String8_List *list, String_Join *opt_params);

////////////////////////////////
// NOTE: String Path Helpers

internal String8 str8_chop_last_slash(String8 s);
internal String8 str8_skip_last_slash(String8 s);
internal String8 str8_chop_last_dot(String8 s);
internal String8 str8_skip_last_dot(String8 s);

internal String8_List str8_split_path(Arena *arena, String8 s);
// internal Path_Style   path_style_from_str8(String8 s);
// internal void         str8_path_list_resolve_dots_in_place(String8_List *list, Path_Style style);
// internal String8      str8_path_list_join_by_style(Arena *arena, String8_List *list, Path_Style style);

////////////////////////////////
// NOTE: Relative <-> Absolute

// internal String8 path_absolute_dst_from_relative_dst_src(Arena *arena, String8 dst, String8 src);

////////////////////////////////
// NOTE: UTF-8 & UTF-16 Decoding/Encoding

internal Unicode_Decode utf8_decode(u8 *str, u64 max);
internal Unicode_Decode utf16_decode(u16 *str, u64 max);
internal u32 utf8_encode(u8 *str, u32 codepoint);
internal u32 utf16_encode(u16 *str, u32 codepoint);

////////////////////////////////
// NOTE: Unicode String Conversions

internal String8 str8_from_16(Arena *arena, String16 in);
internal String16 str16_from_8(Arena *arena, String8 in);
internal String8 str8_from_32(Arena *arena, String32 in);
internal String32 str32_from_8(Arena *arena, String8 in);

////////////////////////////////
// NOTE: Space Enum <-> String Conversions

internal String8 string_from_operating_system(Operating_System os);
internal String8 string_from_arch(Arch arch);
internal String8 string_from_compiler(Compiler compiler);

#endif // BASE_STRING_H
