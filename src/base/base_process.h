#ifndef BASE_PROCESS_H
#define BASE_PROCESS_H

typedef struct Process_Info Process_Info;
struct Process_Info {
  u32 pid;
  String8 exec_file_path;
  String8 exec_path;
  String8 init_path;
  String8_List environment;
};

typedef struct Process Process;
struct Process {
  u64 v[1];
};

typedef struct Process_Node Process_Node;
struct Process_Node {
  Process_Node *next;
  Process v;
};

typedef struct Process_List Process_List;
struct Process_List {
  Process_Node *first;
  Process_Node *last;
  u64 count;
};

typedef struct Process_Launch_Params Process_Launch_Params;
struct Process_Launch_Params {
  String8_List cmd_line;
  String8 path;
  String8_List env;
  b32 inherit_env;
  b32 consoleless;
  File stdout_file;
  File stderr_file;
  File stdin_file;
};

////////////////////////////////
// NOTE: Handle Type Functions

internal Process       process_zero(void);
internal b32           process_match(Process a, Process b);
internal Process_Node *process_list_push(Arena *arena, Process_List *list, Process p);

////////////////////////////////
// NOTE: Process Launcher Helpers

internal Process launch_cmd_line(String8 string);
internal Process launch_cmd_linef(char *fmt, ...);

////////////////////////////////
// NOTE: @per_os_impl Aborting

internal void abort_self(u64 exit_code);

////////////////////////////////
// NOTE: @per_os_impl Process Info

internal Process_Info *get_process_info(void);
internal String8       get_current_path(Arena *arena);
internal b32           set_current_path(String8 path);

////////////////////////////////
// NOTE: @per_os_impl Child Processes

internal Process process_launch(Process_Launch_Params *params);
internal u64     process_pid(Process process);
internal b32     process_join(Process process, u64 endt_us, u64 *out_exit_code);
internal void    process_detach(Process process);
internal b32     process_kill(Process process);

#endif // BASE_PROCESS_H
