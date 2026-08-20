////////////////////////////////
// NOTE: File Info Conversion Helpers

internal File_Property_Flags
w32_file_property_flags_from_dwFileAttributes(DWORD dwFileAttributes) {
  File_Property_Flags result = 0;
  if (dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
    result |= File_Property_Flag_IS_HIDDEN;
  }
  if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    result |= File_Property_Flag_IS_DIRECTORY;
  }
  return(result);
}

////////////////////////////////
// NOTE: Time Conversion Helpers

internal void
w32_date_time_from_system_time(Date_Time *out, SYSTEMTIME *in) {
  out->year = in->wYear;
  out->mon  = in->wMonth - 1;
  out->day  = in->wDay;
  out->hour = in->wHour;
  out->min  = in->wMinute;
  out->sec  = in->wSecond;
  out->msec = in->wMilliseconds;
}

internal void
w32_system_time_from_date_time(SYSTEMTIME *out, Date_Time *in) {
  out->wYear         = (WORD)(in->year);
  out->wMonth        = (WORD)(in->mon + 1);
  out->wDay          = in->day;
  out->wHour         = in->hour;
  out->wMinute       = in->min;
  out->wSecond       = in->sec;
  out->wMilliseconds = in->msec;
}

internal void
w32_dense_time_from_file_time(Dense_Time *out, FILETIME *in) {
  SYSTEMTIME systime = {0};
  FileTimeToSystemTime(in, &systime);
  Date_Time date_time = {0};
  w32_date_time_from_system_time(&date_time, &systime);
  *out = dense_time_from_date_time(date_time);
}

internal u32
w32_sleep_ms_from_endt_us(u64 endt_us) {
  u32 sleep_ms = 0;
  if (endt_us == max_u64) {
    sleep_ms = INFINITE;
  } else {
    u64 begint_us = get_time_us();
    if (begint_us < endt_us) {
      u64 sleep_us = endt_us - begint_us;
      sleep_ms = (u32)((sleep_us + 999)/1000);
    }
  }
  return(sleep_ms);
}

//////////////////////////
// NOTE: @per_os_impl Time

internal u64
get_time_us(void) {
  u64 result = 0;
  LARGE_INTEGER large_integer;
  if (QueryPerformanceCounter(&large_integer)) {
    result = (large_integer.QuadPart*million(1))/w32_state.microsecond_resolution;
  }
  return(result);
}

internal void
sleep_ms(u32 ms) {
  Sleep(ms);
}

////////////////////////////////
// NOTE: @per_os_impl Memory Allocation

internal void *
memory_reserve(u64 size) {
  void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
  return(result);
}

internal b32
memory_commit(void *ptr, u64 size) {
  b32 result = (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0);
  return(result);
}

internal void
memory_decommit(void *ptr, u64 size) {
  VirtualFree(ptr, size, MEM_DECOMMIT);
}

internal void
memory_release(void *ptr, u64 size) {
  // NOTE: size not used - not necessary on Windows, but necessary for other OSes.
  VirtualFree(ptr, 0, MEM_RELEASE);
}

////////////////////////////////
// NOTE: @per_os_impl System Info

internal System_Info *
get_system_info(void) {
  return(&w32_state.system_info);
}

////////////////////////////////
// NOTE: @per_os_impl File System

// NOTE: files

internal File
file_open(String8 path, File_Access_Flags flags) {
  File result = {0};
  Temp scratch = scratch_begin(0, 0);
  String16 path16 = str16_from_8(scratch.arena, path);
  DWORD access_flags = 0;
  DWORD share_mode = 0;
  DWORD creation_disposition = OPEN_EXISTING;
  SECURITY_ATTRIBUTES security_attributes = {sizeof(security_attributes), 0, 0};
  if (flags & File_Access_Flag_READ)        {access_flags |= GENERIC_READ;}
  if (flags & File_Access_Flag_WRITE)       {access_flags |= GENERIC_WRITE;}
  if (flags & File_Access_Flag_EXECUTE)     {access_flags |= GENERIC_EXECUTE;}
  if (flags & File_Access_Flag_SHARE_READ)  {share_mode |= FILE_SHARE_READ;}
  if (flags & File_Access_Flag_SHARE_WRITE) {share_mode |= FILE_SHARE_WRITE|FILE_SHARE_DELETE;}
  if (flags & File_Access_Flag_WRITE)       {creation_disposition = CREATE_ALWAYS;}
  if (flags & File_Access_Flag_APPEND)      {creation_disposition = OPEN_ALWAYS; access_flags |= FILE_APPEND_DATA;}
  if (flags & File_Access_Flag_INHERITED)   {security_attributes.bInheritHandle = 1;}
  HANDLE file = CreateFileW((WCHAR *)path16.str, access_flags, share_mode, &security_attributes, creation_disposition, FILE_ATTRIBUTE_NORMAL, 0);
  if (file != INVALID_HANDLE_VALUE) {
    result.v[0] = (u64)file;
  } else {
    DWORD err = GetLastError();
    (void)err;
  }
  scratch_end(scratch);
  return(result);
}

internal void
file_close(File file) {
  if (!file_match(file, file_zero())) {
    HANDLE handle = (HANDLE)file.v[0];
    BOOL result = CloseHandle(handle);
    (void)result;
  }
}

internal u64
file_read(File file, u64 min, u64 max, void *out_data) {
  u64 total_read_size = 0;
  if (!file_match(file, file_zero())) {
    HANDLE handle = (HANDLE)file.v[0];
    u8 *ptr = out_data;
    u64 off = min;
    while (off != max) {
      u64 amt64 = max - off;
      u32 amt32 = (u32)min(MB(32), amt64);
      DWORD read_size = 0;
      OVERLAPPED overlapped = {.Offset = (u32)off, .OffsetHigh = (u32)(off >> 32)};
      if (!ReadFile(handle, ptr, amt32, &read_size, &overlapped)) {
        break;
      }
      ptr += read_size;
      off += read_size;
    }
    total_read_size = off - min;
  }
  return(total_read_size);
}

internal u64
file_write(File file, u64 min, u64 max, void *data) {
  if (file_match(file, file_zero())) {return(0);};
  HANDLE handle = (HANDLE)file.v[0];
  u64 src_off = 0;
  u64 dst_off = min;
  u64 total_write_size = max - min;
  for (;;) {
    void *bytes_src = (u8 *)data + src_off;
    u64 bytes_left = total_write_size - src_off;
    DWORD write_size = (DWORD)min(MB(1), bytes_left);
    DWORD bytes_written = 0;
    OVERLAPPED overlapped = {0};
    overlapped.Offset = (dst_off&0x00000000ffffffffull);
    overlapped.OffsetHigh = (dst_off&0xffffffff00000000ull) >> 32;
    BOOL success = WriteFile(handle, bytes_src, write_size, &bytes_written, &overlapped);
    if (success == 0) {
      break;
    }
    src_off += bytes_written;
    dst_off += bytes_written;
    if (bytes_left == 0) {
      break;
    }
  }
  return(src_off);
}

internal File_Properties
properties_from_file(File file) {
  File_Properties result = {0};
  if (!file_match(file, file_zero())) {
    HANDLE handle = (HANDLE)file.v[0];
    BY_HANDLE_FILE_INFORMATION info;
    if (GetFileInformationByHandle(handle, &info)) {
      result.size = compose_64bit(info.nFileSizeHigh, info.nFileSizeLow);
      w32_dense_time_from_file_time(&result.modified, &info.ftLastWriteTime);
      w32_dense_time_from_file_time(&result.created, &info.ftCreationTime);
      result.flags = w32_file_property_flags_from_dwFileAttributes(info.dwFileAttributes);
    }
  }
  return(result);
}

internal b32
delete_file_path(String8 path) {
  Temp scratch = scratch_begin(0, 0);
  String16 path16 = str16_from_8(scratch.arena, path);
  b32 result = DeleteFileW((WCHAR *)path16.str);
  scratch_end(scratch);
  return(result);
}

internal b32
copy_file_path(String8 dst, String8 src) {
  Temp scratch = scratch_begin(0, 0);
  String16 dst16 = str16_from_8(scratch.arena, dst);
  String16 src16 = str16_from_8(scratch.arena, src);
  b32 result = CopyFileW((WCHAR *)src16.str, (WCHAR *)dst16.str, 0);
  scratch_end(scratch);
  return(result);
}

internal b32
move_file_path(String8 dst, String8 src) {
  Temp scratch = scratch_begin(0, 0);
  String16 dst16 = str16_from_8(scratch.arena, dst);
  String16 src16 = str16_from_8(scratch.arena, src);
  b32 result = MoveFileW((WCHAR *)src16.str, (WCHAR *)dst16.str);
  scratch_end(scratch);
  return(result);
}

internal String8
full_path_from_path(Arena *arena, String8 path) {
  Temp scratch = scratch_begin(&arena, 1);
  DWORD size = max(MAX_PATH, (DWORD)path.size*2) + 1;
  String16 path16 = str16_from_8(scratch.arena, path);
  u16 *str = push_array_no_zero(scratch.arena, u16, size);
  DWORD size16 = GetFullPathNameW((WCHAR *)path16.str, size, str, 0);
  if (size16 > size) {
    arena_pop(scratch.arena, size);
    size = size16 + 1;
    str = push_array_no_zero(scratch.arena, u16, size);
    size16 = GetFullPathNameW((WCHAR *)path16.str, size, str, 0);
  }
  String8 result = str8_from_16(arena, str16_make((u16 *)str, size16));
  scratch_end(scratch);
  return(result);
}

// NOTE: directory iteration

internal File_Iter
file_iter_begin(Arena *arena, String8 path, File_Iter_Flags flags) {
  Temp scratch = scratch_begin(&arena, 1);
  String8 path_with_wildcard = str8_cat(scratch.arena, path, str8_lit("\\*"));
  String16 path16 = str16_from_8(scratch.arena, path_with_wildcard);
  W32_File_Iter *w32_iter = push_array(arena, W32_File_Iter, 1);
  w32_iter->flags = flags;
  if (path.size == 0) {
    w32_iter->is_volume_iter = 1;
    WCHAR str[512] = {0};
    DWORD size = GetLogicalDriveStringsW(sizeof(str), str);
    String8_List drive_strings = {0};
    for (u64 off = 0; off < (u64)size;) {
      String16 next_drive_string16 = str16_cstr((u16 *)str+off);
      off += next_drive_string16.size+1;
      String8 next_drive_string = str8_from_16(arena, next_drive_string16);
      next_drive_string = str8_chop_last_slash(next_drive_string);
      str8_list_push(scratch.arena, &drive_strings, next_drive_string);
    }
    w32_iter->drive_strings = str8_array_from_list(arena, &drive_strings);
    w32_iter->drive_strings_iter_idx = 0;
  } else {
    w32_iter->handle = FindFirstFileExW((WCHAR *)path16.str, FindExInfoBasic, &w32_iter->find_data, FindExSearchNameMatch, 0, FIND_FIRST_EX_LARGE_FETCH);
  }
  scratch_end(scratch);
  File_Iter result = {(u64)w32_iter};
  return(result);
}

internal b32
file_iter_next(Arena *arena, File_Iter iter, File_Info *info_out) {
  b32 result = 0;
  W32_File_Iter *w32_iter = (W32_File_Iter *)iter.v[0];
  File_Iter_Flags flags = w32_iter->flags;
  switch (w32_iter->is_volume_iter) {
    // NOTE: file iteration
    default:
    case 0: {
      if (!(flags & File_Iter_Flag_DONE) && w32_iter->handle != INVALID_HANDLE_VALUE) {
        do {
          // NOTE: check is usable
          b32 is_usable = 1;

          WCHAR *file_name = w32_iter->find_data.cFileName;
          DWORD attributes = w32_iter->find_data.dwFileAttributes;
          if (file_name[0] == '.') {
            if (flags & File_Iter_Flag_SKIP_HIDDEN) {
              is_usable = 0;
            }
            if (file_name[1] == 0) {
              is_usable = 0;
            }
            else if (file_name[1] == '.' && file_name[2] == 0) {
              is_usable = 0;
            }
          }
          if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (flags & File_Iter_Flag_SKIP_DIRECTORY) {
              is_usable = 0;
            }
          } else {
            if (flags & File_Iter_Flag_SKIP_FILE) {
              is_usable = 0;
            }
          }

          // NOTE: emit if usable
          if (is_usable) {
            info_out->name = str8_from_16(arena, str16_cstr((u16 *)file_name));
            info_out->props.size = compose_64bit(w32_iter->find_data.nFileSizeHigh, w32_iter->find_data.nFileSizeLow);
            w32_dense_time_from_file_time(&info_out->props.modified, &w32_iter->find_data.ftLastWriteTime);
            w32_dense_time_from_file_time(&info_out->props.created, &w32_iter->find_data.ftCreationTime);
            info_out->props.flags = w32_file_property_flags_from_dwFileAttributes(attributes);
            result = 1;
            if (!FindNextFileW(w32_iter->handle, &w32_iter->find_data)) {
              w32_iter->flags |= File_Iter_Flag_DONE;
            }
            break;
          }
        } while(FindNextFileW(w32_iter->handle, &w32_iter->find_data));
      }
    } break;

    // NOTE: volume iteration
    case 1: {
      result = (w32_iter->drive_strings_iter_idx < w32_iter->drive_strings.count);
      if (result != 0) {
        memory_zero_struct(info_out);
        info_out->name = w32_iter->drive_strings.v[w32_iter->drive_strings_iter_idx];
        info_out->props.flags |= File_Property_Flag_IS_DIRECTORY;
        w32_iter->drive_strings_iter_idx += 1;
      }
    } break;
  }
  if (!result) {
    w32_iter->flags |= File_Iter_Flag_DONE;
  }
  return(result);
}

internal void
file_iter_end(File_Iter iter) {
  W32_File_Iter *w32_iter = (W32_File_Iter *)iter.v[0];
  HANDLE handle_zero;
  memory_zero_struct(&handle_zero);
  if (!memory_match_struct(&handle_zero, &w32_iter->handle)) {
    FindClose(w32_iter->handle);
  }
}

////////////////////////////////
// NOTE: @per_os_impl Aborting

internal void
abort_self(u64 exit_code) {
  ExitProcess((int)exit_code);
}

////////////////////////////////
// NOTE: @per_os_impl Process Info

internal Process_Info *
get_process_info(void) {
  return(&w32_state.process_info);
}

internal String8
get_current_path(Arena *arena) {
  Temp scratch = scratch_begin(&arena, 1);
  DWORD size = GetCurrentDirectoryW(0, 0);
  u16 *str = push_array_no_zero(scratch.arena, u16, size + 1);
  size = GetCurrentDirectoryW(size + 1, (WCHAR*)str);
  String8 result = str8_from_16(arena, str16_make(str, size));
  scratch_end(scratch);
  return(result);
}

internal b32
set_current_path(String8 path) {
  Temp scratch = scratch_begin(0, 0);
  String16 path16 = str16_from_8(scratch.arena, path);
  b32 result = SetCurrentDirectoryW((WCHAR *)path16.str);
  scratch_end(scratch);
  return(result);
}

////////////////////////////////
// NOTE: @per_os_impl Child Processes

internal Process
process_launch(Process_Launch_Params *params) {
  Process result = {0};
  Temp scratch = scratch_begin(0, 0);

  // NOTE: form full command string
  String8 cmd = {0};
  {
    String_Join join_params = {0};
    join_params.pre = str8_lit("\"");
    join_params.sep = str8_lit("\" \"");
    join_params.post = str8_lit("\"");
    cmd = str8_list_join(scratch.arena, &params->cmd_line, &join_params);
  }

  // NOTE: form environment
  b32 use_null_env_arg = 0;
  String8 env = {0};
  {
    String_Join join_params = {0};
    join_params.sep = str8_lit("\0");
    join_params.post = str8_lit("\0");
    String8_List all_opts = params->env;
    if (params->inherit_env != 0) {
      if (all_opts.count != 0) {
        memory_zero_struct(&all_opts);
        for (String8_Node *n = params->env.first; n != 0; n = n->next) {
          str8_list_push(scratch.arena, &all_opts, n->v);
        }
        for (String8_Node *n = w32_state.process_info.environment.first; n != 0; n = n->next) {
          str8_list_push(scratch.arena, &all_opts, n->v);
        }
      } else {
        use_null_env_arg = 1;
      }
    }
    if (use_null_env_arg == 0) {
      env = str8_list_join(scratch.arena, &all_opts, &join_params);
    }
  }

  // NOTE: utf-8 -> utf-16
  String16 cmd16 = str16_from_8(scratch.arena, cmd);
  String16 dir16 = str16_from_8(scratch.arena, params->path);
  String16 env16 = {0};
  if (use_null_env_arg == 0) {
    env16 = str16_from_8(scratch.arena, env);
  }

  // NOTE: determine creation flags
  DWORD creation_flags = CREATE_UNICODE_ENVIRONMENT;
  if (params->consoleless != 0) {
    creation_flags |= CREATE_NO_WINDOW;
  }

  // NOTE: launch
  BOOL inherit_handles = 0;
  STARTUPINFOW startup_info = {sizeof(startup_info)};
  if (!file_match(params->stdout_file, file_zero())) {
    HANDLE stdout_handle = (HANDLE)params->stdout_file.v[0];
    startup_info.hStdOutput = stdout_handle;
    startup_info.dwFlags |= STARTF_USESTDHANDLES;
    inherit_handles = 1;
  }
  if (!file_match(params->stderr_file, file_zero())) {
    HANDLE stderr_handle = (HANDLE)params->stderr_file.v[0];
    startup_info.hStdError = stderr_handle;
    startup_info.dwFlags |= STARTF_USESTDHANDLES;
    inherit_handles = 1;
  }
  if (!file_match(params->stdin_file, file_zero())) {
    HANDLE stdin_handle = (HANDLE)params->stdin_file.v[0];
    startup_info.hStdInput = stdin_handle;
    startup_info.dwFlags |= STARTF_USESTDHANDLES;
    inherit_handles = 1;
  }

  PROCESS_INFORMATION process_info = {0};
  if (CreateProcessW(0, (WCHAR *)cmd16.str, 0, 0, inherit_handles, creation_flags, use_null_env_arg ? 0 : (WCHAR *)env16.str, (WCHAR *)dir16.str, &startup_info, &process_info)) {
    result.v[0] = (u64)process_info.hProcess;
    CloseHandle(process_info.hThread);
  }

  scratch_end(scratch);
  return(result);
}

internal u64
pid_from_process(Process process) {
  HANDLE process_handle = (HANDLE)process.v[0];
  u64 result = GetProcessId(process_handle);
  return(result);
}

internal b32
process_join(Process process, u64 endt_us, u64 *out_exit_code) {
  HANDLE process_handle = (HANDLE)process.v[0];
  DWORD sleep_ms = w32_sleep_ms_from_endt_us(endt_us);
  DWORD wait_result = WaitForSingleObject(process_handle, sleep_ms);
  b32 process_joined = (wait_result == WAIT_OBJECT_0);
  if (process_joined && out_exit_code) {
    DWORD exit_code = 0;
    if (GetExitCodeProcess(process_handle, &exit_code)) {
      *out_exit_code = exit_code;
    }
  }
  if (process_joined) {
    CloseHandle(process_handle);
  }
  return(process_joined);
}

internal void
process_detach(Process process) {
  HANDLE process_handle = (HANDLE)process.v[0];
  CloseHandle(process_handle);
}

internal b32
process_kill(Process process) {
  HANDLE process_handle = (HANDLE)process.v[0];
  b32 result = TerminateProcess(process_handle, 999);
  return(result);
}

////////////////////////////////
// NOTE: @per_os_impl Dynamically-Loaded Libraries

internal Library
library_open(String8 path) {
  Temp scratch = scratch_begin(0, 0);
  String16 path16 = str16_from_8(scratch.arena, path);
  HMODULE module = LoadLibraryW((LPCWSTR)path16.str);
  Library result = {(u64)module};
  scratch_end(scratch);
  return(result);
}

internal void
library_close(Library lib) {
  HMODULE module = (HMODULE)lib.v[0];
  FreeLibrary(module);
}

internal void_proc *
library_load_proc(Library lib, String8 name) {
  Temp scratch = scratch_begin(0, 0);
  HMODULE module = (HMODULE)lib.v[0];
  name = str8_copy(scratch.arena, name);
  void_proc *result = (void_proc *)GetProcAddress(module, (LPCSTR)name.str);
  scratch_end(scratch);
  return(result);
}

////////////////////////////////
// NOTE: Entry Point

internal void
w32_entry_point_caller(int argc, WCHAR **wargv) {
  // NOTE: get system info
  SYSTEM_INFO sysinfo = {0};
  GetSystemInfo(&sysinfo);

  // NOTE: set up non-dynamically-alloc'd state
  {
    w32_state.microsecond_resolution = 1;
    LARGE_INTEGER large_integer;
    if (QueryPerformanceFrequency(&large_integer)) {
      w32_state.microsecond_resolution = large_integer.QuadPart;
    }
  }
  {
    System_Info *info = &w32_state.system_info;
    info->logical_processor_count = sysinfo.dwNumberOfProcessors;
    info->page_size = sysinfo.dwPageSize;
    info->allocation_granularity = sysinfo.dwAllocationGranularity;
  }
  {
    Process_Info *info = &w32_state.process_info;
    info->pid = GetCurrentProcessId();
  }

  // NOTE: extract arguments
  Arena *arg_arena = arena_alloc(.res_size = MB(1), .cmt_size = KB(32));
  char **argv = push_array(arg_arena, char *, argc);
  for (int idx = 0; idx < argc; idx += 1) {
    String16 arg16 = str16_cstr((u16 *)wargv[idx]);
    argv[idx] = (char *)str8_from_16(arg_arena, arg16).str;
  }

  // NOTE: set up thread context
  Thread_Context *tctx = tctx_alloc();
  tctx_select(tctx);

  // NOTE: set up dynamically-alloc'd state
  Arena *arena = arena_alloc();
  {
    w32_state.arena = arena;
    {
      System_Info *info = &w32_state.system_info;
      u16 str[MAX_COMPUTERNAME_LENGTH + 1] = {0};
      DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
      if (GetComputerNameW((WCHAR *)str, &size)) {
        info->machine_name = str8_from_16(arena, str16_make(str, size));
      }
    }
    {
      Process_Info *info = &w32_state.process_info;
      {
        Temp scratch = scratch_begin(0, 0);
        DWORD size = KB(32);
        u16 *str = push_array_no_zero(scratch.arena, u16, size);
        size = GetModuleFileNameW(0, (WCHAR *)str, size);
        String8 name = str8_from_16(scratch.arena, str16_make(str, size));
        info->exec_file_path = str8_copy(arena, name);
        info->exec_path = str8_chop_last_slash(info->exec_file_path);
        scratch_end(scratch);
      }
      info->init_path = get_current_path(arena);
      {
        WCHAR *env = GetEnvironmentStringsW();
        u64 start_idx = 0;
        for (u64 idx = 0;; idx += 1) {
          if (env[idx] == 0) {
            if (start_idx == idx) {
              break;
            } else {
              String16 string16 = str16_make((u16 *)env + start_idx, idx - start_idx);
              String8 string = str8_from_16(arena, string16);
              str8_list_push(arena, &info->environment, string);
              start_idx = idx+1;
            }
          }
        }
      }
    }
  }

  // NOTE: call into "real" entry point
  main_thread_base_entry_point(argc, argv);
}

#if BUILD_CONSOLE_INTERFACE
int wmain(int argc, WCHAR **argv) {
  w32_entry_point_caller(argc, argv);
  return(0);
}
#else
int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
  // CoInitializeEx(0, COINIT_APARTMENTTHREADED);
  w32_entry_point_caller(__argc, __wargv);
  return(0);
}
#endif
