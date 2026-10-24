#include "sh_core.c"
#if OS_WINDOWS
# include "w32/sh/w32_sh.c"
#elif OS_LINUX
# include "lnx/sh/lnx_sh.c"
#endif
