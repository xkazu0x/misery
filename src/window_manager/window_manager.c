#include "window_manager_core.c"
#if OS_WINDOWS
# include "win32/window_manager/win32_window_manager.c"
#elif OS_LINUX
# include "linux/window_manager/linux_window_manager.c"
#else
# error Window manager layer not implemented for this operating system.
#endif
