#ifndef LNX_BASE_H
#define LNX_BASE_H

////////////////////////////////
// NOTE: Includes/Libraries

#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <spawn.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

////////////////////////////////
// NOTE: File Iterator Types

typedef struct LNX_File_Iter LNX_File_Iter;
struct LNX_File_Iter {
  File_Iter_Flags flags;
  String8 path;
  DIR *dir;
  struct dirent *dp;
};

////////////////////////////////
// NOTE: State

typedef struct LNX_State LNX_State;
struct LNX_State {
  Arena *arena;
  System_Info system_info;
  Process_Info process_info;
  u64 default_env_count;
  char **default_env;
};

////////////////////////////////
// NOTE: Globals

global LNX_State lnx_state = {0};

////////////////////////////////
// NOTE: Helpers

internal Date_Time lnx_date_time_from_tm(struct tm tm, u16 msec);
internal struct tm lnx_tm_from_date_time(Date_Time dt);
internal Dense_Time lnx_dense_time_from_timespec(struct timespec ts);
internal File_Properties lnx_file_properties_from_stat(struct stat *st);

#endif // LNX_BASE_H
