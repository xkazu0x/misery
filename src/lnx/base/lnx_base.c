////////////////////////////////
// NOTE: Helpers

internal Date_Time
lnx_date_time_from_tm(struct tm tm, u16 msec) {
  Date_Time result = {0};
  result.sec  = tm.tm_sec;
  result.min  = tm.tm_min;
  result.hour = tm.tm_hour;
  result.day  = tm.tm_mday-1;
  result.mon  = tm.tm_mon;
  result.year = tm.tm_year+1900;
  result.msec = msec;
  return(result);
}

internal struct tm
lnx_tm_from_date_time(Date_Time dt) {
  struct tm result = {0};
  result.tm_sec  = dt.sec;
  result.tm_min  = dt.min;
  result.tm_hour = dt.hour;
  result.tm_mday = dt.day+1;
  result.tm_mon  = dt.mon;
  result.tm_year = dt.year-1900;
  return(result);
}

internal Dense_Time
lnx_dense_time_from_timespec(struct timespec ts) {
  struct tm tm = {0};
  gmtime_r(&ts.tv_sec, &tm);
  Date_Time dt = lnx_date_time_from_tm(tm, ts.tv_nsec/million(1));
  Dense_Time result = dense_time_from_date_time(dt);
  return(result);
}

internal File_Properties
lnx_file_properties_from_stat(struct stat *st) {
  File_Properties result = {0};
  result.size     = st->st_size;
  result.created  = lnx_dense_time_from_timespec(st->st_ctim);
  result.modified = lnx_dense_time_from_timespec(st->st_mtim);
  if (st->st_mode & S_IFDIR) {
    result.flags |= File_Property_Flag_IS_DIRECTORY;
  }
  return(result);
}

//////////////////////////
// NOTE: @per_os_impl Time

internal u64
get_time_us(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  u64 result = t.tv_sec*million(1) + (t.tv_nsec/thousand(1));
  return(result);
}

internal void
sleep_ms(u32 ms) {
  usleep(ms*thousand(1));
}

////////////////////////////////
// NOTE: @per_os_impl Memory Allocation

internal void *
memory_reserve(u64 size) {
  void *result = mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (result == MAP_FAILED) {
    result = 0;
  }
  return(result);
}

internal b32
memory_commit(void *ptr, u64 size) {
  mprotect(ptr, size, PROT_READ|PROT_WRITE);
  return(1);
}

internal void
memory_decommit(void *ptr, u64 size) {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

internal void
memory_release(void *ptr, u64 size) {
  munmap(ptr, size);
}

////////////////////////////////
// NOTE: @per_os_impl System Info

internal System_Info *
get_system_info(void) {
  return(&lnx_state.system_info);
}

////////////////////////////////
// NOTE: @per_os_impl File System

// NOTE: files

internal File
file_open(String8 path, File_Access_Flags flags) {
  File result = {0};
  Temp scratch = scratch_begin(0, 0);
  char *path_cstr = (char *)str8_copy(scratch.arena, path).str;
  int lnx_flags = 0;
  if (flags & File_Access_Flag_READ && flags & File_Access_Flag_WRITE) {
    lnx_flags = O_RDWR;
  } else if (flags & File_Access_Flag_READ) {
    lnx_flags = O_RDONLY;
  } else if (flags & File_Access_Flag_WRITE) {
    lnx_flags = O_WRONLY;
  }
  if (flags & File_Access_Flag_APPEND) {
    lnx_flags |= O_APPEND;
  }
  if (flags & (File_Access_Flag_WRITE|File_Access_Flag_APPEND)) {
    lnx_flags |= O_CREAT;
  }
  lnx_flags |= O_CLOEXEC;
  int fd = open(path_cstr, lnx_flags, 0755);
  if (fd != -1) {
    result.v[0] = fd;
  }
  scratch_end(scratch);
  return(result);
}

internal void
file_close(File file) {
  if (!file_match(file, file_zero())) {
    int fd = (int)file.v[0];
    close(fd);
  }
}

internal u64
file_read(File file, u64 min, u64 max, void *out_data) {
  u64 result = 0;
  if (!file_match(file, file_zero())) {
    int fd = (int)file.v[0];
    u64 total_num_bytes_to_read = max - min;
    u64 total_num_bytes_read = 0;
    u64 total_num_bytes_left_to_read = total_num_bytes_to_read;
    for (; total_num_bytes_left_to_read > 0;) {
      int read_result = pread(fd, (u8 *)out_data+total_num_bytes_read, total_num_bytes_left_to_read, min+total_num_bytes_read);
      if (read_result >= 0) {
        total_num_bytes_read += read_result;
        total_num_bytes_left_to_read -= read_result;
      }
      else if (errno != EINTR) {
        break;
      }
    }
    result = total_num_bytes_read;
  }
  return(result);
}

internal u64
file_write(File file, u64 min, u64 max, void *data) {
  u64 result = 0;
  if (!file_match(file, file_zero())) {
    int fd = (int)file.v[0];
    u64 total_num_bytes_to_write = max - min;
    u64 total_num_bytes_written = 0;
    u64 total_num_bytes_left_to_write = total_num_bytes_to_write;
    for (; total_num_bytes_left_to_write > 0;) {
      int write_result = pwrite(fd, (u8 *)data+total_num_bytes_written, total_num_bytes_left_to_write, min+total_num_bytes_written);
      if (write_result >= 0) {
        total_num_bytes_written += write_result;
        total_num_bytes_left_to_write -= write_result;
      }
      else if (errno != EINTR) {
        break;
      }
    }
    result = total_num_bytes_written;
  }
  return(result);
}

internal File_Properties
file_properties(File file) {
  File_Properties result = {0};
  if (!file_match(file, file_zero())) {
    int fd = (int)file.v[0];
    struct stat st = {0};
    if (fstat(fd, &st) != -1) {
      result = lnx_file_properties_from_stat(&st);
    }
  }
  return(result);
}

internal b32
file_path_remove(String8 path) {
  b32 result = 0;
  Temp scratch = scratch_begin(0, 0);
  char *path_cstr = (char *)str8_copy(scratch.arena, path).str;
  if (remove(path_cstr) != -1) {
    result = 1;
  }
  scratch_end(scratch);
  return(result);
}

internal b32
file_path_move(String8 dst, String8 src) {
  b32 result = 0;
  Temp scratch = scratch_begin(0, 0);
  char *dst_cstr = (char *)str8_copy(scratch.arena, dst).str;
  char *src_cstr = (char *)str8_copy(scratch.arena, src).str;
  if (rename(src_cstr, dst_cstr) != -1) {
    result = 1;
  }
  scratch_end(scratch);
  return(result);
}

internal b32
file_path_copy(String8 dst, String8 src) {
  // TODO: fix, result is always zero.
  b32 result = 0;
  File src_h = file_open(src, File_Access_Flag_READ);
  File dst_h = file_open(src, File_Access_Flag_WRITE);
  if (!file_match(src_h, file_zero()) &&
      !file_match(dst_h, file_zero())) {
    int src_fd = (int)src_h.v[0];
    int dst_fd = (int)dst_h.v[0];
    File_Properties src_props = file_properties(src_h);
    int total_num_bytes_to_copy = src_props.size;
    int total_num_bytes_copied = 0;
    int total_num_bytes_left_to_copy = total_num_bytes_to_copy;
    for (; total_num_bytes_left_to_copy > 0;) {
      off_t sendfile_off = total_num_bytes_copied;
      int sendfile_result = sendfile(dst_fd, src_fd, &sendfile_off, total_num_bytes_left_to_copy);
      if (sendfile_result <= 0) {
        break;
      }
      u64 bytes_copied = (u64)sendfile_result;
      total_num_bytes_copied += bytes_copied;
      total_num_bytes_left_to_copy -= bytes_copied;
    }
  }
  file_close(src_h);
  file_close(dst_h);
  return(result);
}

internal String8
full_path_from_path(Arena *arena, String8 path) {
  Temp scratch = scratch_begin(&arena, 1);
  char *path_cstr = (char *)str8_copy(scratch.arena, path).str;
  char buffer[PATH_MAX] = {0};
  realpath(path_cstr, buffer);
  String8 result = str8_copy(arena, str8_cstr(buffer));
  scratch_end(scratch);
  return(result);
}

internal b32
file_path_exists(String8 path) {
  Temp scratch = scratch_begin(0, 0);
  String8 path_copy = str8_copy(scratch.arena, path);
  int access_result = access((char *)path_copy.str, F_OK);
  b32 result = 0;
  if (access_result == 0) {
    result = 1;
  }
  scratch_end(scratch);
  return(result);
}

// NOTE: directories

internal b32
directory_make(String8 path) {
  b32 result = 0;
  Temp scratch = scratch_begin(0, 0);
  String8 path_copy = str8_copy(scratch.arena, path);
  if (mkdir((char *)path_copy.str, 0755) != -1) {
    result = 1;
  } else {
    result = file_path_exists(path);
  }
  scratch_end(scratch);
  return(result);
}

internal b32
directory_path_exists(String8 path) {
  b32 result = 0;
  Temp scratch = scratch_begin(0, 0);
  String8 path_copy = str8_copy(scratch.arena, path);
  DIR *dir = opendir((char *)path_copy.str);
  if (dir) {
    closedir(dir);
    result = 1;
  }
  scratch_end(scratch);
  return(result);
}

// NOTE: directory iteration

internal File_Iter
file_iter_begin(Arena *arena, String8 path, File_Iter_Flags flags) {
  LNX_File_Iter *iter = push_array(arena, LNX_File_Iter, 1);
  iter->flags = flags;
  iter->path = str8_copy(arena, path);
  iter->dir = opendir((char *)iter->path.str);
  File_Iter result = {(u64)iter};
  return(result);
}

internal b32
file_iter_next(Arena *arena, File_Iter iter, File_Info *info_out) {
  b32 good = 0;
  LNX_File_Iter *lnx_iter = (LNX_File_Iter *)iter.v[0];
  for (; lnx_iter->dir != 0;) {
    // NOTE: get next entry
    lnx_iter->dp = readdir(lnx_iter->dir);
    good = (lnx_iter->dp != 0);

    // NOTE: unpack entry info
    struct stat st = {0};
    int stat_result = 0;
    if (good) {
      Temp scratch = scratch_begin(&arena, 1);
      String8 full_path = str8f(scratch.arena, "%.*s/%s", str8_fmt(lnx_iter->path), lnx_iter->dp->d_name);
      stat_result = stat((char *)full_path.str, &st);
      scratch_end(scratch);
    }

    // NOTE: determine if filtered
    b32 filtered = 0;
    if (good) {
      filtered = 
        ((st.st_mode == S_IFDIR && lnx_iter->flags & File_Iter_Flag_SKIP_DIRECTORY) ||
         (st.st_mode == S_IFREG && lnx_iter->flags & File_Iter_Flag_SKIP_FILE) ||
         (lnx_iter->dp->d_name[0] == '.' && lnx_iter->dp->d_name[1] == 0) ||
         (lnx_iter->dp->d_name[0] == '.' && lnx_iter->dp->d_name[1] == '.' && lnx_iter->dp->d_name[2] == 0));
    }

    // NOTE: output & exit, if good & unfiltered
    if (good && !filtered) {
      info_out->name = str8_copy(arena, str8_cstr(lnx_iter->dp->d_name));
      if (stat_result != -1) {
        info_out->props = lnx_file_properties_from_stat(&st);
      }
      break;
    }

    // NOTE: exit if not good
    if (!good) {
      break;
    }
  }
  return(good);
}

internal void
file_iter_end(File_Iter iter) {
  LNX_File_Iter *lnx_iter = (LNX_File_Iter *)iter.v[0];
  closedir(lnx_iter->dir);
}

////////////////////////////////
// NOTE: @per_os_impl Aborting

internal void
abort_self(u64 exit_code) {
  exit((int)exit_code);
}

////////////////////////////////
// NOTE: @per_os_impl Process Info

internal Process_Info *
get_process_info(void) {
  return(&lnx_state.process_info);
}

internal String8
get_current_path(Arena *arena) {
  char *cwdir = getcwd(0, 0);
  String8 result = str8_copy(arena, str8_cstr(cwdir));
  free(cwdir);
  return(result);
}

internal b32
set_current_path(String8 path) {
  Temp scratch = scratch_begin(0, 0);
  char *path_cstr = (char *)str8_copy(scratch.arena, path).str;
  b32 result = (chdir(path_cstr) != -1);
  scratch_end(scratch);
  return(result);
}

////////////////////////////////
// NOTE: @per_os_impl Child Processes

internal Process
process_launch(Process_Launch_Params *params) {
  Process result = {0};
  posix_spawn_file_actions_t file_actions = {0};
  int file_actions_init_code = posix_spawn_file_actions_init(&file_actions);
  if (file_actions_init_code == 0) {
    Temp scratch = scratch_begin(0, 0);

    if (params->path.size != 0) {
      int chdir_code = posix_spawn_file_actions_addchdir_np(&file_actions, (char *)str8_copy(scratch.arena, params->path).str);
      assert(chdir_code == 0);
    }

    if (!file_match(params->stdout_file, file_zero())) {
      int stdout_code = posix_spawn_file_actions_adddup2(&file_actions, (int)params->stdout_file.v[0], STDOUT_FILENO);
      assert(stdout_code == 0);
    }
    if (!file_match(params->stderr_file, file_zero())) {
      int stderr_code = posix_spawn_file_actions_adddup2(&file_actions, (int)params->stderr_file.v[0], STDERR_FILENO);
      assert(stderr_code == 0);
    }
    if (!file_match(params->stdin_file, file_zero())) {
      int stdin_code = posix_spawn_file_actions_adddup2(&file_actions, (int)params->stdin_file.v[0], STDIN_FILENO);
      assert(stdin_code == 0);
    }

    File std_files[] = {params->stdout_file, params->stderr_file, params->stdin_file};
    for_each_element(i, std_files) {
      int fd = (int)std_files[i].v[0];
      b32 is_unique = !file_match(std_files[i], file_zero());
      for_each_index(j, i) {is_unique &= !file_match(std_files[i], std_files[j]);}
      if (is_unique && fd > STDERR_FILENO) {
        int close_code = posix_spawn_file_actions_addclose(&file_actions, fd);
        assert(close_code == 0);
      }
    }

    posix_spawnattr_t attr = {0};
    int attr_init_code = posix_spawnattr_init(&attr);
    if (attr_init_code == 0) {
      // NOTE: package argv
      char **argv = push_array(scratch.arena, char *, params->cmd_line.count + 1);
      {
        argv[0] = (char *)str8_copy(scratch.arena, params->cmd_line.first->v).str;
        u64 arg_idx = 1;
        for_each_node(String8_Node, n, params->cmd_line.first->next) {
          argv[arg_idx] = (char *)str8_copy(scratch.arena, n->v).str;
          arg_idx += 1;
        }
      }

      // NOTE: package envp
      char **envp = 0;
      if (params->inherit_env != 0) {
        envp = lnx_state.default_env;
      } else {
        envp = push_array(scratch.arena, char *, params->env.count + 1);
        u64 env_idx = 0;
        for_each_node(String8_Node, n, params->env.first) {
          envp[env_idx] = (char *)str8_copy(scratch.arena, n->v).str;
          env_idx += 1;
        }
      }

      // NOTE: spawn process
      pid_t pid = 0;
      int spawn_code = posix_spawnp(&pid, argv[0], &file_actions, &attr, argv, envp);
      if (spawn_code == 0) {
        result.v[0] = (u64)pid;
      }

      // NOTE: clean up attributes
      int attr_destroy_code = posix_spawnattr_destroy(&attr);
      (void)attr_destroy_code;
    }
    scratch_end(scratch);

    // NOTE: clean up file actions
    int file_actions_destroy_code = posix_spawn_file_actions_destroy(&file_actions);
    (void)file_actions_destroy_code;
  }
  return(result);
}

internal u64
process_pid(Process process) {
  u64 result = (u64)process.v[0];
  return(result);
}

internal b32
process_join(Process process, u64 endt_us, u64 *out_exit_code) {
  b32 result = 0;
  pid_t pid = (pid_t)process.v[0];
  for (;;) {
    int status = 0;
    pid_t wait_result = waitpid(pid, &status, (endt_us == max_u64) ? 0 : WNOHANG);
    if ((wait_result == pid) && (WIFEXITED(status) || WIFSIGNALED(status))) {
      result = 1;
      if (out_exit_code != 0) {
        if      (WIFEXITED(status))   {*out_exit_code = WEXITSTATUS(status);}
        else if (WIFSIGNALED(status)) {*out_exit_code = WTERMSIG(status) + 128;}
      }
      break;
    }

    if (wait_result == -1) {break;}
    if (endt_us == 0) {break;}

    u64 now_us = get_time_us();
    if (now_us >= endt_us) {break;};

    u64 left_us = endt_us - now_us;
    u64 sleep_us = min(left_us, thousand(1));
    usleep((useconds_t)sleep_us);
  }
  return(result);
}

internal void
process_detach(Process process) {
  // NOTE: not need to close pid
}

internal b32
process_kill(Process process) {
  int error_code = kill((pid_t)process.v[0], SIGKILL);
  b32 is_killed = (error_code == 0);
  return(is_killed);
}

////////////////////////////////
// NOTE: @per_os_impl Dynamically-Loaded Libraries

internal Library
library_open(String8 path) {
  Temp scratch = scratch_begin(0, 0);
  char *path_cstr = (char *)str8_copy(scratch.arena, path).str;
  void *so = dlopen(path_cstr, RTLD_LAZY|RTLD_LOCAL);
  Library result = {(u64)so};
  scratch_end(scratch);
  return(result);
}

internal void
library_close(Library lib) {
  void *so = (void *)lib.v[0];
  dlclose(so);
}

internal void_proc *
library_load_proc(Library lib, String8 name) {
  Temp scratch = scratch_begin(0, 0);
  void *so = (void *)lib.v[0];
  char *name_cstr = (char *)str8_copy(scratch.arena, name).str;
  void_proc *result = (void_proc *)dlsym(so, name_cstr);
  scratch_end(scratch);
  return(result);
}

////////////////////////////////
// NOTE: Entry Point

int
main(int argc, char **argv) {
  // NOTE: setup os layer
  {
    // NOTE: grab statically allocated system/process info
    {
      System_Info *info = &lnx_state.system_info;
      info->logical_processor_count = (u32)get_nprocs();
      info->page_size               = getpagesize();
      info->allocation_granularity  = info->page_size;
    }
    {
      Process_Info *info = &lnx_state.process_info;
      info->pid = (u32)getpid();
    }

    // NOTE: setup thread context
    TCTX *tctx = tctx_alloc();
    tctx_select(tctx);

    // NOTE: setup dynamically allocated state
    lnx_state.arena = arena_alloc();

    // NOTE: cache default environment
    {
      u64 env_count = 0;
      for (; __environ[env_count] != 0; env_count += 1);
      char **default_env = push_array(lnx_state.arena, char *, env_count);
      for_each_index(idx, env_count) {
        default_env[idx] = (char *)str8_copy(lnx_state.arena, str8_cstr(__environ[idx])).str;
      }
      default_env[env_count] = 0;
      lnx_state.default_env_count = env_count;
      lnx_state.default_env = default_env;
    }

    // NOTE: grab dynamically allocated system info
    {
      Temp scratch = scratch_begin(0, 0);
      System_Info *info = &lnx_state.system_info;

      // NOTE: get machine name
      b32 got_final_result = 0;
      u8 *buffer = 0;
      int size = 0;
      for (s64 cap = 4096, r = 0; r < 4; cap *= 2, r += 1) {
        scratch_end(scratch);
        buffer = push_array(scratch.arena, u8, cap);
        int gethostname_result = gethostname((char *)buffer, cap);
        size = cstr8_len(buffer);
        if (gethostname_result == 0 && size < cap) {
          got_final_result = 1;
          break;
        }
      }

      // NOTE: save name to info
      if (got_final_result && size > 0) {
        info->machine_name.size = size;
        info->machine_name.str = push_array_no_zero(lnx_state.arena, u8, info->machine_name.size + 1);
        memory_copy(info->machine_name.str, buffer, info->machine_name.size);
        info->machine_name.str[info->machine_name.size] = 0;
      }

      scratch_end(scratch);
    }

    // NOTE: grab dynamically allocated process info
    {
      Temp scratch = scratch_begin(0, 0);
      Process_Info *info = &lnx_state.process_info;
      
      // NOTE: grab exec file path
      {
        // NOTE: get self string
        b32 got_final_result = 0;
        u8 *buffer = 0;
        int size = 0;
        for (s64 cap = PATH_MAX, r = 0; r < 4; cap *= 2, r += 1) {
          scratch_end(scratch);
          buffer = push_array_no_zero(scratch.arena, u8, cap);
          size = readlink("/proc/self/exe", (char *)buffer, cap);
          if (size < cap) {
            got_final_result = 1;
            break;
          }
        }

        // NOTE: save
        if (got_final_result && size > 0) {
          String8 exec_file_path = str8_make(buffer, size);
          info->exec_file_path = str8_copy(lnx_state.arena, exec_file_path);
          info->exec_path = str8_chop_last_slash(info->exec_file_path);
        }
      }

      // NOTE: grab initial directory
      info->init_path = get_current_path(lnx_state.arena);

      scratch_end(scratch);
    }
  }

  // NOTE: call into "real" entry point
  main_thread_base_entry_point(argc, argv);
}
