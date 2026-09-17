#include "wm_core.c"
#if OS_WINDOWS
# include "w32/wm/w32_wm.c"
#elif OS_LINUX
# include "lnx/wm/lnx_wm.c"
#else
# error Window manager layer not implemented for this operating system.
#endif
