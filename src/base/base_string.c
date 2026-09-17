////////////////////////////////
// NOTE: Character Classification & Conversion Functions

internal b32
char_is_space(u8 c) {
  b32 result = (c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\v' || c == '\f');
  return(result);
}

internal b32
char_is_upper(u8 c) {
  b32 result = ('A' <= c && c <= 'Z');
  return(result);
}

internal b32
char_is_lower(u8 c) {
  b32 result = ('a' <= c && c <= 'z');
  return(result);
}

internal b32
char_is_alpha(u8 c) {
  b32 result = (char_is_upper(c) || char_is_lower(c));
  return(result);
}

internal b32
char_is_slash(u8 c) {
  b32 result = (c == '/' || c == '\\');
  return(result);
}

internal b32
char_is_digit(u8 c) {
  b32 result = ('0' <= c && c <= '9');
  return(result);
}

internal u8
lower_from_char(u8 c) {
  if (char_is_upper(c)) {
    c += ('a' - 'A');
  }
  return(c);
}

internal u8
upper_from_char(u8 c) {
  if (char_is_lower(c)) {
    c -= ('a' - 'A');
  }
  return(c);
}

internal u8
correct_slash_from_char(u8 c) {
  if (char_is_slash(c)) {
    c = '/';
  }
  return(c);
}

////////////////////////////////
// NOTE: C-String Measurement

internal u64
cstr8_len(u8 *cstr) {
  u64 len = 0;
  if (cstr) {
    u8 *ptr = cstr;
    for (;*ptr != 0; ptr += 1);
    len = (ptr - cstr);
  }
  return(len);
}

internal u64
cstr16_len(u16 *cstr) {
  u64 len = 0;
  if (cstr) {
    u16 *ptr = cstr;
    for (;*ptr != 0; ptr += 1);
    len = (ptr - cstr);
  }
  return(len);
}

internal u64
cstr32_len(u32 *cstr) {
  u64 len = 0;
  if (cstr) {
    u32 *ptr = cstr;
    for (;*ptr != 0; ptr += 1);
    len = (ptr - cstr);
  }
  return(len);
}

////////////////////////////////
// NOTE: String Constructors

internal String8
str8_zero(void) {
  String8 result = {0};
  return(result);
}

internal String8
str8_make(u8 *str, u64 size) {
  String8 result = {str, size};
  return(result);
}

internal String8
str8_range(u8 *first, u8 *opl) {
  String8 result = str8_make(first, (opl - first));
  return(result);
}

internal String8
str8_cstr(char *cstr) {
  String8 result = str8_make((u8 *)cstr, cstr8_len((u8 *)cstr));
  return(result);
}

internal String16
str16_zero(void) {
  String16 result = {0};
  return(result);
}

internal String16
str16_make(u16 *str, u64 size) {
  String16 result = {str, size};
  return(result);
}

internal String16
str16_range(u16 *first, u16 *one_past_last) {
  String16 result = str16_make(first, (one_past_last - first));
  return(result);
}

internal String16
str16_cstr(u16 *cstr) {
  String16 result = str16_make(cstr, cstr16_len(cstr));
  return(result);
}

internal String32
str32_zero(void) {
  String32 result = {0};
  return(result);
}

internal String32
str32_make(u32 *str, u64 size) {
  String32 result = {str, size};
  return(result);
}

internal String32
str32_range(u32 *first, u32 *one_past_last) {
  String32 result = str32_make(first, (one_past_last - first));
  return(result);
}

internal String32
str32_cstr(u32 *cstr) {
  String32 result = str32_make(cstr, cstr32_len(cstr));
  return(result);
}

////////////////////////////////
// NOTE: String Stylization

internal String8
upper_from_str8(Arena *arena, String8 s) {
  s = str8_copy(arena, s);
  for (u64 idx = 0; idx < s.size; idx += 1) {
    s.str[idx] = upper_from_char(s.str[idx]);
  }
  return(s);
}

internal String8
lower_from_str8(Arena *arena, String8 s) {
  s = str8_copy(arena, s);
  for (u64 idx = 0; idx < s.size; idx += 1) {
    s.str[idx] = lower_from_char(s.str[idx]);
  }
  return(s);
}

internal String8
backslashed_from_str8(Arena *arena, String8 s) {
  s = str8_copy(arena, s);
  for (u64 idx = 0; idx < s.size; idx += 1) {
    s.str[idx] = char_is_slash(s.str[idx]) ? '\\' : s.str[idx];
  }
  return(s);
}

internal String8
forwardslashed_from_str8(Arena *arena, String8 s) {
  s = str8_copy(arena, s);
  for (u64 idx = 0; idx < s.size; idx += 1) {
    s.str[idx] = char_is_slash(s.str[idx]) ? '/' : s.str[idx];
  }
  return(s);
}

////////////////////////////////
// NOTE: String Matching

internal b32
str8_match(String8 a, String8 b, String_Match_Flags flags) {
  b32 result = 0;
  if (a.size == b.size || (flags & String_Match_Flag_RIGHT_SIDE_SLOPPY)) {
    b32 case_insensitive = (flags & String_Match_Flag_CASE_INSENSITIVE);
    b32 slash_insensitive = (flags & String_Match_Flag_SLASH_INSENSITIVE);
    u64 size = min(a.size, b.size);
    result = 1;
    for (u64 i = 0; i < size; i += 1) {
      u8 at = a.str[i];
      u8 bt = b.str[i];
      if (case_insensitive) {
        at = upper_from_char(at);
        bt = upper_from_char(bt);
      }
      if (slash_insensitive) {
        at = correct_slash_from_char(at);
        bt = correct_slash_from_char(bt);
      }
      if (at != bt) {
        result = 0;
        break;
      }
    }
  }
  return(result);
}

internal b32
str8_char_match(u8 a, u8 b, String_Match_Flags flags) {
  u8 at = a;
  u8 bt = b;
  if (flags & String_Match_Flag_CASE_INSENSITIVE) {
    at = upper_from_char(at);
    bt = upper_from_char(bt);
  }
  if (flags & String_Match_Flag_SLASH_INSENSITIVE) {
    at = correct_slash_from_char(at);
    bt = correct_slash_from_char(bt);
  }
  return(at == bt);
}

internal u64
str8_find_needle(String8 s, String8 needle, String_Match_Flags flags) {
  u8 *p = s.str;
  u64 p_stop_off = max(s.size + 1, needle.size) - needle.size;
  u8 *p_stop = p + p_stop_off;
  if (needle.size > 0) {
    u8 *s_opl = s.str + s.size;
    String8 needle_tail = str8_skip(needle, 1);
    String_Match_Flags adjusted_flags = flags|String_Match_Flag_RIGHT_SIDE_SLOPPY;
    u8 needle_first_char_adjusted = needle.str[0];
    if (adjusted_flags & String_Match_Flag_CASE_INSENSITIVE) {
      needle_first_char_adjusted = upper_from_char(needle_first_char_adjusted);
    }
    if (adjusted_flags & String_Match_Flag_SLASH_INSENSITIVE) {
      needle_first_char_adjusted = correct_slash_from_char(needle_first_char_adjusted);
    }
    for (;p < p_stop; p += 1) {
      u8 haystack_char_adjusted = *p;
      if (adjusted_flags & String_Match_Flag_CASE_INSENSITIVE) {
        haystack_char_adjusted = upper_from_char(haystack_char_adjusted);
      }
      if (adjusted_flags & String_Match_Flag_SLASH_INSENSITIVE) {
        haystack_char_adjusted = correct_slash_from_char(haystack_char_adjusted);
      }
      if (haystack_char_adjusted == needle_first_char_adjusted) {
        if (str8_match(str8_range(p + 1, s_opl), needle_tail, adjusted_flags)) {
          break;
        }
      }
    }
  }
  u64 result = s.size;
  if (p < p_stop) {
    result = (u64)(p - s.str);
  }
  return(result);
}

////////////////////////////////
// NOTE: String Slicing

internal String8
str8_substr(String8 s, u64 min, u64 max) {
  min = clamp_top(min, s.size);
  max = clamp_top(max, s.size);
  s.str += min;
  s.size = (max - min);
  return(s);
}

internal String8
str8_prefix(String8 s, u64 size) {
  s.size = clamp_top(size, s.size);
  return(s);
}

internal String8
str8_skip(String8 s, u64 amt) {
  amt = clamp_top(amt, s.size);
  s.str += amt;
  s.size -= amt;
  return(s);
}

internal String8
str8_postfix(String8 s, u64 size) {
  size = clamp_top(size, s.size);
  s.str = (s.str + s.size) - size;
  s.size = size;
  return(s);
}

internal String8
str8_chop(String8 s, u64 amt) {
  s.size -= clamp_top(amt, s.size);
  return(s);
}

////////////////////////////////
// NOTE: String Formatting & Copying

internal String8
str8_cat(Arena *arena, String8 a, String8 b) {
  String8 result;
  result.size = a.size + b.size;
  result.str = push_array_no_zero(arena, u8, result.size + 1);
  memory_copy(result.str, a.str, a.size);
  memory_copy(result.str + a.size, b.str, b.size);
  result.str[result.size] = 0;
  return(result);
}

internal String8
str8_copy(Arena *arena, String8 s) {
  String8 result;
  result.size = s.size;
  result.str = push_array_no_zero(arena, u8, result.size + 1);
  memory_copy(result.str, s.str, s.size);
  result.str[result.size] = 0;
  return(result);
}

internal String8
str8fv(Arena *arena, char *fmt, va_list args) {
  va_list args2;
  va_copy(args2, args);
  u32 needed_bytes = vsnprintf(0, 0, fmt, args) + 1;
  String8 result;
  result.str = push_array_no_zero(arena, u8, needed_bytes);
  result.size = vsnprintf((char *)result.str, needed_bytes, fmt, args2);
  result.str[result.size] = 0;
  va_end(args2);
  return(result);
}

internal String8
str8f(Arena *arena, char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String8 result = str8fv(arena, fmt, args);
  va_end(args);
  return(result);
}

////////////////////////////////
// NOTE: String List

internal String8_Node *
str8_list_push_node(String8_List *list, String8_Node *n) {
  sll_queue_push(list->first, list->last, n);
  list->count += 1;
  list->size += n->v.size;
  return(n);
}

internal String8_Node *
str8_list_push_node_set_string(String8_List *list, String8_Node *n, String8 s) {
  sll_queue_push(list->first, list->last, n);
  list->count += 1;
  list->size += s.size;
  n->v = s;
  return(n);
}

internal String8_Node *
str8_list_push(Arena *arena, String8_List *list, String8 s) {
  String8_Node *n = push_array_no_zero(arena, String8_Node, 1);
  str8_list_push_node_set_string(list, n, s);
  return(n);
}

internal String8_Node *
str8_list_pushf(Arena *arena, String8_List *list, char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String8 s = str8fv(arena, fmt, args);
  String8_Node *result = str8_list_push(arena, list, s);
  va_end(args);
  return(result);
}

internal String8_Node *
str8_list_push_node_front(String8_List *list, String8_Node *n) {
  sll_queue_push_front(list->first, list->last, n);
  list->count += 1;
  list->size += n->v.size;
  return(n);
}

internal String8_Node *
str8_list_push_node_front_set_string(String8_List *list, String8_Node *n, String8 s) {
  sll_queue_push_front(list->first, list->last, n);
  list->count += 1;
  list->size += s.size;
  n->v = s;
  return(n);
}

internal String8_Node *
str8_list_push_front(Arena *arena, String8_List *list, String8 s) {
  String8_Node *n = push_array_no_zero(arena, String8_Node, 1);
  str8_list_push_node_front_set_string(list, n, s);
  return(n);
}

internal String8_Node *
str8_list_push_frontf(Arena *arena, String8_List *list, char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  String8 s = str8fv(arena, fmt, args);
  String8_Node *result = str8_list_push_front(arena, list, s);
  va_end(args);
  return(result);
}

internal String8_Node *
str8_list_pop_front(String8_List *list) {
  String8_Node *n = 0;
  if (list->count > 0) {
    n = list->first;
    assert(list->size >= n->v.size);
    list->count -= 1;
    list->size -= n->v.size;
    sll_queue_pop(list->first, list->last);
  }
  return(n);
}

internal String8_List
str8_list_copy(Arena *arena, String8_List *list) {
  String8_List result = {0};
  for (String8_Node *n = list->first; n != 0; n = n->next) {
    String8_Node *node = push_array_no_zero(arena, String8_Node, 1);
    String8 string = str8_copy(arena, n->v);
    str8_list_push_node_set_string(&result, node, string);
  }
  return(result);
}

internal void
str8_list_concat_in_place(String8_List *list, String8_List *to_push) {
  if (to_push->count != 0) {
    if (list->last) {
      list->count += to_push->count;
      list->size += to_push->size;
      list->last->next = to_push->first;
      list->last = to_push->last;
    } else {
      *list = *to_push;
    }
    memory_zero_struct(to_push);
  }
}

////////////////////////////////
// NOTE: String Array

internal String8_Array
str8_array_zero(void) {
  String8_Array result = {0};
  return(result);
}

internal String8_Array
str8_array_from_list(Arena *arena, String8_List *list) {
  String8_Array result = {0};
  result.count = list->count;
  result.v = push_array_no_zero(arena, String8, result.count);
  u64 idx = 0;
  for (String8_Node *n = list->first; n != 0; n = n->next, idx += 1) {
    result.v[idx] = n->v;
  }
  return(result);
}

////////////////////////////////
// NOTE: String Splitting & Joining

internal String8_List
str8_split(Arena *arena, String8 s, u8 *split_chars, u64 num_split_chars, String_Split_Flags flags) {
  String8_List list = {0};
  b32 keep_empties = (flags & String_Split_Flag_KEEP_EMPTIES);
  u8 *ptr = s.str;
  u8 *opl = ptr + s.size;
  for (;ptr < opl;) {
    u8 *first = ptr;
    for (;ptr < opl; ptr += 1) {
      u8 c = *ptr;
      b32 is_split = 0;
      for (u64 idx = 0; idx < num_split_chars; idx += 1) {
        if (split_chars[idx] == c) {
          is_split = 1;
          break;
        }
      }
      if (is_split) {
        break;
      }
    }
    String8 string = str8_range(first, ptr);
    if (keep_empties || string.size > 0) {
      str8_list_push(arena, &list, string);
    }
    ptr += 1;
  }
  return(list);
}

internal String8_List
str8_split_by_string_chars(Arena *arena, String8 s, String8 split_chars, String_Split_Flags flags) {
  String8_List result = str8_split(arena, s, split_chars.str, split_chars.size, flags);
  return(result);
}

internal String8
str8_list_join(Arena *arena, String8_List *list, String_Join *opt_params) {
  String_Join join = {0};
  if (opt_params != 0) {
    memory_copy_struct(&join, opt_params);
  }
  u64 sep_count = 0;
  if (list->count > 0) {
    sep_count = list->count - 1;
  }
  String8 result = {0};
  result.size = list->size + join.pre.size + sep_count*join.sep.size + join.post.size;
  u8 *ptr = result.str = push_array_no_zero(arena, u8, result.size + 1);
  memory_copy(ptr, join.pre.str, join.pre.size);
  ptr += join.pre.size;
  for (String8_Node *n = list->first; n != 0; n = n->next) {
    memory_copy(ptr, n->v.str, n->v.size);
    ptr += n->v.size;
    if (n->next != 0) {
      memory_copy(ptr, join.sep.str, join.sep.size);
      ptr += join.sep.size;
    }
  }
  memory_copy(ptr, join.post.str, join.post.size);
  ptr += join.post.size;
  *ptr = 0;
  return(result);
}

////////////////////////////////
// NOTE: String Path Helpers

internal String8
str8_chop_last_slash(String8 s) {
  if (s.size > 0) {
    u8 *ptr = s.str + s.size - 1;
    for (;ptr >= s.str; ptr -= 1) {
      if (*ptr == '/' || *ptr == '\\') {
        break;
      }
    }
    if (ptr >= s.str) {
      s.size = ptr - s.str;
    } else {
      s.size = 0;
    }
  }
  return(s);
}

internal String8
str8_skip_last_slash(String8 s) {
  if (s.size > 0) {
    u8 *ptr = s.str + s.size - 1;
    for (;ptr >= s.str; ptr -= 1) {
      if (*ptr == '/' || *ptr == '\\') {
        break;
      }
    }
    if (ptr >= s.str) {
      ptr += 1;
      s.size = s.str + s.size - ptr;
      s.str = ptr;
    }
  }
  return(s);
}

internal String8
str8_chop_last_dot(String8 s) {
  u64 idx = s.size;
  for (;idx > 0;) {
    idx -= 1;
    if (s.str[idx] == '.') {
      s = str8_prefix(s, idx);
      break;
    }
  }
  return(s);
}

internal String8
str8_skip_last_dot(String8 s) {
  u64 idx = s.size;
  for (;idx > 0;) {
    idx -= 1;
    if (s.str[idx] == '.') {
      s = str8_skip(s, idx + 1);
      break;
    }
  }
  return(s);
}

internal String8_List
str8_split_path(Arena *arena, String8 s) {
  String8_List result = str8_split(arena, s, (u8 *)"/\\", 2, 0);
  return(result);
}

// internal Path_Style   path_style_from_str8(String8 s);
// internal void         str8_path_list_resolve_dots_in_place(String8_List *list, Path_Style style);
// internal String8      str8_path_list_join_by_style(Arena *arena, String8_List *list, Path_Style style);

////////////////////////////////
// NOTE: Relative <-> Absolute

// internal String8 path_absolute_dst_from_relative_dst_src(Arena *arena, String8 dst, String8 src);

////////////////////////////////
// NOTE: UTF-8 & UTF-16 Decoding/Encoding

global const u8 utf8_class[32] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

internal Unicode_Decode
utf8_decode(u8 *str, u64 max) {
  Unicode_Decode result = {1, max_u32};
  u8 byte = str[0];
  u8 byte_class = utf8_class[byte >> 3];
  switch (byte_class) {
    case 1: {
      result.codepoint = byte;
    } break;
    case 2: {
      if (1 < max) {
        u8 count_byte = str[1];
        if (utf8_class[count_byte >> 3] == 0) {
          result.codepoint = ((byte & bitmask5) << 6);
          result.codepoint |= (count_byte & bitmask6);
          result.increment = 2;
        }
      }
    } break;
    case 3: {
      if (2 < max) {
        u8 count_byte[2] = {str[1], str[2]};
        if (utf8_class[count_byte[0] >> 3] == 0 &&
            utf8_class[count_byte[1] >> 3] == 0) {
          result.codepoint = ((byte & bitmask4) << 12);
          result.codepoint |= ((count_byte[0] & bitmask6) << 6);
          result.codepoint |=  (count_byte[1] & bitmask6);
          result.increment = 3;
        }
      }
    } break;
    case 4: {
      if (3 < max) {
        u8 count_byte[3] = {str[1], str[2], str[3]};
        if (utf8_class[count_byte[0] >> 3] == 0 &&
            utf8_class[count_byte[1] >> 3] == 0 &&
            utf8_class[count_byte[2] >> 3] == 0) {
          result.codepoint = ((byte & bitmask3) << 18);
          result.codepoint |= ((count_byte[0] & bitmask6) << 12);
          result.codepoint |= ((count_byte[1] & bitmask6) << 6);
          result.codepoint |=  (count_byte[2] & bitmask6);
          result.increment = 4;
        }
      }
    } break;
  }
  return(result);
}

internal Unicode_Decode
utf16_decode(u16 *str, u64 max) {
  Unicode_Decode result = {1, max_u32};
  result.codepoint = str[0];
  result.increment = 1;
  if (max > 1 &&
      0xD800 <= str[0] && str[0] < 0xDC00 &&
      0xDC00 <= str[1] && str[1] < 0xE000) {
    result.codepoint = ((str[0] - 0xD800) << 10) | ((str[1] - 0xDC00) + 0x10000);
    result.increment = 2;
  }
  return(result);
}

internal u32
utf8_encode(u8 *str, u32 codepoint) {
  u32 increment = 0;
  if (codepoint <= 0x7F) {
    str[0] = (u8)codepoint;
    increment = 1;
  } else if (codepoint <= 0x7FF) {
    str[0] = (u8)((bitmask2 << 6) | ((codepoint >> 6) & bitmask5));
    str[1] = (u8)(bit8 | (codepoint & bitmask6));
    increment = 2;
  } else if (codepoint <= 0xFFFF) {
    str[0] = (u8)((bitmask3 << 5) | ((codepoint >> 12) & bitmask4));
    str[1] = (u8)(bit8 | ((codepoint >> 6) & bitmask6));
    str[2] = (u8)(bit8 |  (codepoint       & bitmask6));
    increment = 3;
  } else if (codepoint <= 0x10FFFF) {
    str[0] = (u8)((bitmask4 << 4) | ((codepoint >> 18) & bitmask3));
    str[1] = (u8)(bit8 | ((codepoint >> 12) & bitmask6));
    str[2] = (u8)(bit8 | ((codepoint >>  6) & bitmask6));
    str[3] = (u8)(bit8 |  (codepoint        & bitmask6));
    increment = 4;
  } else {
    str[0] = '?';
    increment = 1;
  }
  return(increment);
}

internal u32
utf16_encode(u16 *str, u32 codepoint) {
  u32 increment = 1;
  if (codepoint == max_u32) {
    str[0] = (u16)'?';
  } else if (codepoint < 0x10000) {
    str[0] = (u16)codepoint;
  } else {
    u32 u = codepoint - 0x10000;
    str[0] = safe_cast_u16(0xD800 + (u >> 10));
    str[1] = safe_cast_u16(0xDC00 + (u & bitmask10));
    increment = 2;
  }
  return(increment);
}

////////////////////////////////
// NOTE: Unicode String Conversions

internal String8
str8_from_16(Arena *arena, String16 in) {
  String8 result = {0};
  if (in.size) {
    u64 cap = in.size*3;
    u8 *str = push_array_no_zero(arena, u8, cap + 1);
    u16 *ptr = in.str;
    u16 *opl = ptr + in.size;
    u64 size = 0;
    Unicode_Decode consume;
    for (;ptr < opl; ptr += consume.increment) {
      consume = utf16_decode(ptr, opl - ptr);
      size += utf8_encode(str + size, consume.codepoint);
    }
    str[size] = 0;
    arena_pop(arena, (cap - size));
    result = str8_make(str, size);
  }
  return(result);
}

internal String16
str16_from_8(Arena *arena, String8 in) {
  String16 result = {0};
  if (in.size) {
    u64 cap = in.size*2;
    u16 *str = push_array_no_zero(arena, u16, cap + 1);
    u8 *ptr = in.str;
    u8 *opl = ptr + in.size;
    u64 size = 0;
    Unicode_Decode consume;
    for (;ptr < opl; ptr += consume.increment) {
      consume = utf8_decode(ptr, opl - ptr);
      size += utf16_encode(str + size, consume.codepoint);
    }
    str[size] = 0;
    arena_pop(arena, (cap - size)*2);
    result = str16_make(str, size);
  }
  return(result);
}

internal String8
str8_from_32(Arena *arena, String32 in) {
  String8 result = {0};
  if (in.size) {
    u64 cap = in.size*4;
    u8 *str = push_array_no_zero(arena, u8, cap + 1);
    u32 *ptr = in.str;
    u32 *opl = ptr + in.size;
    u64 size = 0;
    for (;ptr < opl; ptr += 1) {
      size += utf8_encode(str + size, *ptr);
    }
    str[size] = 0;
    arena_pop(arena, (cap - size));
    result = str8_make(str, size);
  }
  return(result);
}

internal String32
str32_from_8(Arena *arena, String8 in) {
  String32 result = {0};
  if (in.size) {
    u64 cap = in.size;
    u32 *str = push_array_no_zero(arena, u32, cap + 1);
    u8 *ptr = in.str;
    u8 *opl = ptr + in.size;
    u64 size = 0;
    Unicode_Decode consume;
    for (;ptr < opl; ptr += consume.increment) {
      consume = utf8_decode(ptr, opl - ptr);
      str[size] = consume.codepoint;
      size += 1;
    }
    str[size] = 0;
    arena_pop(arena, (cap - size)*4);
    result = str32_make(str, size);
  }
  return(result);
}

////////////////////////////////
// NOTE: Space Enum <-> String Conversions

internal String8
string_from_operating_system(Operating_System os) {
  String8 result = {0};
  switch (os) {
    case Operating_System_NULL:    {result = str8_lit("Null");} break;
    case Operating_System_WINDOWS: {result = str8_lit("Windows");} break;
    case Operating_System_LINUX:   {result = str8_lit("Linux");} break;
    case Operating_System_MAC:     {result = str8_lit("Mac");} break;
    case Operating_System_COUNT:   {result = str8_lit("Invalid");} break;
  }
  return result;
}

internal String8
string_from_arch(Arch arch) {
  String8 result = {0};
  switch (arch) {
    case Arch_NULL:  {result = str8_lit("Null");} break;
    case Arch_X64:   {result = str8_lit("x64");} break;
    case Arch_X86:   {result = str8_lit("x86");} break;
    case Arch_ARM64: {result = str8_lit("arm64");} break;
    case Arch_ARM32: {result = str8_lit("arm32");} break;
    case Arch_COUNT: {result = str8_lit("Invalid");} break;
  }
  return result;
}

internal String8
string_from_compiler(Compiler compiler) {
  String8 result = {0};
  switch (compiler) {
    case Compiler_NULL:  {result = str8_lit("Null");} break;
    case Compiler_MSVC:  {result = str8_lit("MSVC");} break;
    case Compiler_GCC:   {result = str8_lit("GCC");} break;
    case Compiler_CLANG: {result = str8_lit("Clang");} break;
    case Compiler_COUNT: {result = str8_lit("Invalid");} break;
  }
  return result;
}
