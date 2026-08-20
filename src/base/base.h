#ifndef BASE_H
#define BASE_H

#include "base_context_cracking.h"

#include "base_core.h"
#include "base_math.h"
#include "base_memory.h"
#include "base_arena.h"
#include "base_string.h"
#include "base_system.h"
#include "base_thread_context.h"
#include "base_file.h"
#include "base_process.h"
#include "base_dynamic_library.h"
#include "base_entry_point.h"

#if OS_WINDOWS
# include "win32/base/win32_base.h"
#else
# error Operating System backend not found for base layer.
#endif

#endif // BASE_H
