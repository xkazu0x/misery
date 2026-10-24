#ifndef SH_H
#define SH_H

#include "sh_core.h"
#if OS_WINDOWS
# include "w32/sh/w32_sh.h"
#elif OS_LINUX
# include "lnx/sh/lnx_sh.h"
#endif

#endif // SH_H
