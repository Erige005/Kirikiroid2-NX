/* [K2S] libnx has no mmap: minimal stub so blend2d's BLFileMapping compiles; mapping always fails. */
#pragma once
#include <stddef.h>
#include <sys/types.h>
#include <errno.h>
#define PROT_READ 1
#define PROT_WRITE 2
#define MAP_SHARED 1
#define MAP_PRIVATE 2
#define MAP_FAILED ((void *)-1)
static inline void *mmap(void *a, size_t l, int p, int f, int fd, off_t o) { (void)a; (void)l; (void)p; (void)f; (void)fd; (void)o; errno = ENOSYS; return MAP_FAILED; }
static inline int munmap(void *a, size_t l) { (void)a; (void)l; return 0; }
