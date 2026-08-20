#ifndef DYNAMIC_LIBRARY_H
#define DYNAMIC_LIBRARY_H

typedef struct Library Library;
struct Library {
  u64 v[1];
};

////////////////////////////////
// NOTE: @per_os_impl Dynamically-Loaded Libraries

internal Library    library_open(String8 path);
internal void       library_close(Library lib);
internal void_proc *library_load_proc(Library lib, String8 name);

#endif // DYNAMIC_LIBRARY_H
