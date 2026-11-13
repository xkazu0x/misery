#include "base_core.c"
#include "base_math.c"
#include "base_memory.c"
#include "base_arena.c"
#include "base_string.c"
#include "base_system.c"
#include "base_tctx.c"
#include "base_file.c"
#include "base_process.c"
#include "base_library.c"
#include "base_entry_point.c"

#if OS_WINDOWS
# include "win32/base/win32_base.c"
#elif OS_LINUX
# include "linux/base/linux_base.c"
#else
# error Base layer not implemented for this operating system.
#endif
