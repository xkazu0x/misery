#include "base_core.c"
#include "base_math.c"
#include "base_memory.c"
#include "base_arena.c"
#include "base_string.c"
#include "base_system.c"
#include "base_thread_context.c"
#include "base_process.c"
#include "base_file.c"
#include "base_dynamic_library.c"
#include "base_entry_point.c"

#if OS_WINDOWS
# include "win32/base/win32_base.c"
#else
# error Operating System backend not found for base layer.
#endif
