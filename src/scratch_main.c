#include "base/base.h"
// #include "wm/wm.h"

#include "base/base.c"
// #include "wm/wm.c"

internal void
entry_point(int argc, char **argv) {
  String8 os_str = string_from_operating_system(Operating_System_CURRENT);
  String8 arch_str = string_from_arch(Arch_CURRENT);
  String8 cc_str = string_from_compiler(Compiler_CURRENT);
  printf("--- Build Info ----------------------\n");
  printf("os: %.*s\n", str8_fmt(os_str));
  printf("arch: %.*s\n", str8_fmt(arch_str));
  printf("cc: %.*s\n", str8_fmt(cc_str));
  printf("\n");

  System_Info *sinfo = get_system_info();
  printf("--- System Info ---------------------\n");
  printf("logical_processor_count: %d\n", sinfo->logical_processor_count);
  printf("page_size: %lu\n", sinfo->page_size);
  printf("allocation_granularity: %lu\n", sinfo->allocation_granularity);
  printf("machine_name: %.*s\n", str8_fmt(sinfo->machine_name));
  printf("\n");

  Process_Info *pinfo = get_process_info();
  printf("--- Process Info --------------------\n");
  printf("pid: %d\n", pinfo->pid);
  printf("exec_file_path: %.*s\n", str8_fmt(pinfo->exec_file_path));
  printf("exec_path: %.*s\n", str8_fmt(pinfo->exec_path));
  printf("init_path: %.*s\n", str8_fmt(pinfo->init_path));
  printf("\n");

  Process process = launch_cmd_line(s("echo hello, misery."));
  process_join(process, max_u64, 0);
}
