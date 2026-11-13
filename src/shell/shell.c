#include "shell_core.c"
#if OS_WINDOWS
# include "win32/shell/win32_shell.c"
#elif OS_LINUX
# include "linux/shell/linux_shell.c"
#endif
