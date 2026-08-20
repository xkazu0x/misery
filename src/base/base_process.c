////////////////////////////////
// NOTE: Handle Type Functions

internal Process
process_zero(void) {
  Process result = {0};
  return(result);
}

internal b32
process_match(Process a, Process b) {
  b32 result = memory_match_struct(&a, &b);
  return(result);
}

internal Process_Node *
process_list_push(Arena *arena, Process_List *list, Process p) {
  Process_Node *n = push_array(arena, Process_Node, 1);
  sll_queue_push(list->first, list->last, n);
  list->count += 1;
  n->v = p;
  return(n);
}

////////////////////////////////
// NOTE: Process Launcher Helpers

internal Process
launch_cmd_line(String8 string) {
  Process result = {0};
  Temp scratch = scratch_begin(0, 0);
  u8 split_chars[] = {' '};
  String8_List parts = str8_split(scratch.arena, string, split_chars, array_count(split_chars), 0);
  if (parts.count != 0) {
    // NOTE: unpack exec part
    String8 exec_file_path = parts.first->v;
    String8 exec_path = str8_chop_last_slash(exec_file_path);
    if (exec_path.size == 0) {
      exec_path = get_current_path(scratch.arena);
    }

    // NOTE: find stdout delimiter
    String8_Node *stdout_delimiter_n = 0;
    for (String8_Node *n = parts.first; n != 0; n = n->next) {
      if (str8_match(n->v, str8_lit(">"), 0)) {
        stdout_delimiter_n = n;
        break;
      }
    }

    // NOTE: read stdout path
    String8 stdout_path = {0};
    if (stdout_delimiter_n != 0 && stdout_delimiter_n->next) {
      stdout_path = stdout_delimiter_n->next->v;
    }

    // NOTE: open stdout handle
    File stdout_handle = {0};
    if (stdout_path.size != 0) {
      File file = file_open(stdout_path, File_Access_Flag_WRITE|File_Access_Flag_READ);
      file_close(file);
      stdout_handle = file_open(stdout_path, File_Access_Flag_WRITE|File_Access_Flag_APPEND|File_Access_Flag_SHARE_READ|File_Access_Flag_SHARE_WRITE|File_Access_Flag_INHERITED);
    }

    // NOTE: form command line
    String8_List cmd_line = {0};
    for (String8_Node *n = parts.first; n != stdout_delimiter_n && n != 0; n = n->next) {
      str8_list_push(scratch.arena, &cmd_line, n->v);
    }

    // NOTE: launch
    Process_Launch_Params params = {0};
    params.cmd_line = cmd_line;
    params.path = exec_path;
    params.inherit_env = 1;
    params.stdout_file = stdout_handle;
    result = process_launch(&params);

    // NOTE: close stdout file
    if (stdout_path.size != 0) {
      file_close(stdout_handle);
    }
  }
  scratch_end(scratch);
  return(result);
}

internal Process
launch_cmd_linef(char *fmt, ...) {
  Temp scratch = scratch_begin(0, 0);
  va_list args;
  va_start(args, fmt);
  String8 string = str8fv(scratch.arena, fmt, args);
  Process result = launch_cmd_line(string);
  va_end(args);
  scratch_end(scratch);
  return(result);
}
