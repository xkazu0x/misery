#ifndef SHELL_H
#define SHELL_H

#include "shell_core.h"
#if OS_WINDOWS
# include "win32/shell/win32_shell.h"
#elif OS_LINUX
# include "linux/shell/linux_shell.h"
#endif

#endif // SHELL_H
