////////////////////////////////
// NOTE: Handle Type Functions

internal File
file_zero(void) {
  File result = {0};
  return(result);
}

internal b32
file_match(File a, File b) {
  b32 result = memory_match_struct(&a, &b);
  return(result);
}

////////////////////////////////
// NOTE: File System Helpers

internal String8
string_from_file_min_max(Arena *arena, File file, u64 min, u64 max) {
  u64 pre_pos = arena_pos(arena);
  String8 result = {0};
  result.size = (max - min);
  result.str = push_array_no_zero(arena, u8, result.size);
  u64 actual_read_size = file_read(file, min, max, result.str);
  if (actual_read_size < result.size) {
    arena_pop_to(arena, pre_pos + actual_read_size);
    result.size = actual_read_size;
  }
  return(result);
}

internal String8
data_from_file_path(Arena *arena, String8 path) {
  File file = file_open(path, File_Access_Flag_READ|File_Access_Flag_SHARE_READ);
  File_Properties props = file_properties(file);
  String8 data = string_from_file_min_max(arena, file, 0, props.size);
  file_close(file);
  return(data);
}
