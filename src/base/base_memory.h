#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

////////////////////////////////
// NOTE: @per_os_impl Memory Allocation

internal void *memory_reserve(u64 size);
internal b32   memory_commit(void *ptr, u64 size);
internal void  memory_decommit(void *ptr, u64 size);
internal void  memory_release(void *ptr, u64 size);

#endif // BASE_MEMORY_H
