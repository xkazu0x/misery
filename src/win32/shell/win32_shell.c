////////////////////////////////
// NOTE: @per_os_impl Shell Operations

internal void
sh_message(b32 error, String8 title, String8 message) {
  Temp scratch = scratch_begin(0, 0);
  String16 title16 = str16_from_8(scratch.arena, title);
  String16 message16 = str16_from_8(scratch.arena, message);
  MessageBoxW(0, (WCHAR *)message16.str, (WCHAR *)title16.str, MB_OK|(!!error*MB_ICONERROR));
  scratch_end(scratch);
}
