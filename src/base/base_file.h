#ifndef BASE_FILE_H
#define BASE_FILE_H

typedef u32 File_Property_Flags;
enum {
  File_Property_Flag_IS_HIDDEN    = (1<<0),
  File_Property_Flag_IS_DIRECTORY = (1<<1),
};

typedef u32 File_Access_Flags;
enum {
  File_Access_Flag_READ        = (1<<0),
  File_Access_Flag_WRITE       = (1<<1),
  File_Access_Flag_EXECUTE     = (1<<2),
  File_Access_Flag_APPEND      = (1<<3),
  File_Access_Flag_SHARE_READ  = (1<<4),
  File_Access_Flag_SHARE_WRITE = (1<<5),
  File_Access_Flag_INHERITED   = (1<<6),
};

typedef u32 File_Iter_Flags;
enum {
  File_Iter_Flag_SKIP_DIRECTORY = (1<<0),
  File_Iter_Flag_SKIP_FILE   = (1<<1),
  File_Iter_Flag_SKIP_HIDDEN  = (1<<2),
  File_Iter_Flag_DONE         = (1<<3),
};

typedef struct File_Properties File_Properties;
struct File_Properties {
  u64 size;
  Dense_Time modified;
  Dense_Time created;
  File_Property_Flags flags;
};

typedef struct File_Info File_Info;
struct File_Info {
  String8 name;
  File_Properties props;
};

typedef struct File File;
struct File {
  u64 v[1];
};

typedef struct File_Iter File_Iter;
struct File_Iter {
  u64 v[1];
};

////////////////////////////////
// NOTE: Handle Type Functions

internal File file_zero(void);
internal b32  file_match(File a, File b);

////////////////////////////////
// NOTE: File System Helpers

internal String8 data_from_file_path(Arena *arena, String8 path);
internal String8 string_from_file_min_max(Arena *arena, File file, u64 min, u64 max);

////////////////////////////////
// NOTE: @per_os_impl File System

// NOTE: files
internal File            file_open(String8 path, File_Access_Flags flags);
internal void            file_close(File file);
internal u64             file_read(File file, u64 min, u64 max, void *out_data);
internal u64             file_write(File file, u64 min, u64 max, void *data);
internal File_Properties properties_from_file(File file);
internal b32             delete_file_path(String8 path);
internal b32             copy_file_path(String8 dst, String8 src);
internal b32             move_file_path(String8 dst, String8 src);
internal String8         full_path_from_path(Arena *arena, String8 path);

// NOTE: directory iteration
internal File_Iter file_iter_begin(Arena *arena, String8 path, File_Iter_Flags flags);
internal b32       file_iter_next(Arena *arena, File_Iter iter, File_Info *info_out);
internal void      file_iter_end(File_Iter iter);

#endif // BASE_FILE_H
