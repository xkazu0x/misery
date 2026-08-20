#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "window_manager_core.h"
#if OS_WINDOWS
# include "win32/window_manager/win32_window_manager.h"
#else
# error Window manager layer not implemented for this operating system.
#endif

#endif // WINDOW_MANAGER_H
