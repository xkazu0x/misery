#ifndef WM_H
#define WM_H

#include "wm_core.h"
#if OS_WINDOWS
# include "w32/wm/w32_wm.h"
#elif OS_LINUX
# include "lnx/wm/lnx_wm.h"
#else
# error Window manager layer not implemented for this operating system.
#endif

#endif // WM_H
