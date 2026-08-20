#ifndef WIN32_BASE_H
#define WIN32_BASE_H

////////////////////////////////
// NOTE: Includes/Libraries

#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "user32")

////////////////////////////////
// NOTE: File Iterator Types

typedef struct W32_File_Iter W32_File_Iter;
struct W32_File_Iter {
  File_Iter_Flags flags;
  HANDLE handle;
  WIN32_FIND_DATAW find_data;
  b32 is_volume_iter;
  String8_Array drive_strings;
  u64 drive_strings_iter_idx;
};

////////////////////////////////
// NOTE: State

typedef struct W32_State W32_State;
struct W32_State {
  Arena *arena;
  
  // NOTE: info
  System_Info system_info;
  Process_Info process_info;
  u64 microsecond_resolution;
};

////////////////////////////////
// NOTE: Globals

global W32_State w32_state = {0};

////////////////////////////////
// NOTE: File Info Conversion Helpers

internal File_Property_Flags w32_file_property_flags_from_dwFileAttributes(DWORD dwFileAttributes);

////////////////////////////////
// NOTE: Time Conversion Helpers

internal void w32_date_time_from_system_time(Date_Time *out, SYSTEMTIME *in);
internal void w32_system_time_from_date_time(SYSTEMTIME *out, Date_Time *in);
internal void w32_dense_time_from_file_time(Dense_Time *out, FILETIME *in);
internal u32 w32_sleep_ms_from_endt_us(u64 endt_us);

#endif // WIN32_BASE_H
