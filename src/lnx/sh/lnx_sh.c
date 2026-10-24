////////////////////////////////
// NOTE: @per_os_impl Shell Operations

internal void
sh_message(b32 error, String8 title, String8 message) {
  if (error) {
    fprintf(stderr, "[X] ");
  }
  fprintf(stderr, "%.*s\n", str8_varg(title));
  fprintf(stderr, "%.*s\n", str8_varg(message));
}
