#ifndef BASE_H
#define BASE_H

#include "base_core.h"
#include "base_math.h"
#include "base_memory.h"
#include "base_arena.h"
#include "base_string.h"
#include "base_system.h"
#include "base_tctx.h"
#include "base_file.h"
#include "base_process.h"
#include "base_library.h"
#include "base_entry_point.h"

#if OS_WINDOWS
# include "win32/base/win32_base.h"
#elif OS_LINUX
# include "linux/base/linux_base.h"
#else
# error Base layer not implemented for this operating system.
#endif

#endif // BASE_H
