// [K2S] Force-included (-include) into every Kirikiroid2 translation unit on Switch.
// Bridges the gaps between Android's bionic libc (what 1.3.9 was written against) and newlib/libnx.
#pragma once
#ifdef __SWITCH__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <sched.h>

// newlib #defines st_atime/st_mtime/st_ctime as st_Xtim.tv_sec, which also rewrites the fields of
// Kirikiroid2's own `tTVP_stat` (Platform.h). Real `struct stat` users on Switch use st_Xtim explicitly.
// (p7zip sources define K2S_KEEP_STAT_MACROS: they only use the real struct stat)
#ifndef K2S_KEEP_STAT_MACROS
#undef st_atime
#undef st_mtime
#undef st_ctime
#endif

// bionic has the *64 variants; newlib's off_t is already 64-bit.
#define lseek64 lseek
#define off64_t off_t

// bionic/glibc global `long timezone` (seconds west of UTC); newlib calls it _timezone.
#define timezone _timezone

#ifndef ULLONG_MAX
#define ULLONG_MAX (~0ULL)
#endif
#ifndef LLONG_MAX
#define LLONG_MAX 0x7fffffffffffffffLL
#endif

#ifdef __cplusplus
#include <string>
#include <climits>
#include <cstddef>
extern "C" {
#endif

// newlib has no timegm() (used by p7zip's wine_date_and_time.cpp): UTC struct tm -> time_t.
static inline time_t k2s_timegm(struct tm *t)
{
    long long y = t->tm_year + 1900LL, m = t->tm_mon + 1;
    if (m <= 2) { y -= 1; m += 12; }
    long long days = 365 * y + y / 4 - y / 100 + y / 400 + (153 * (m - 3) + 2) / 5 + t->tm_mday - 719469;
    return (time_t)(days * 86400 + t->tm_hour * 3600 + t->tm_min * 60 + t->tm_sec);
}
#define timegm k2s_timegm

// Thread priorities: newlib/libnx pthreads don't expose these; ThreadImpl.cpp only uses them to
// raise/lower priority, which is a no-op here (libnx threads keep their creation priority).
static inline int k2s_pthread_getschedparam(pthread_t, int *policy, struct sched_param *param)
{
    if (policy) *policy = 0;
    if (param) param->sched_priority = 0;
    return 0;
}
static inline int k2s_pthread_setschedparam(pthread_t, int, const struct sched_param *) { return 0; }
#define pthread_getschedparam k2s_pthread_getschedparam
#define pthread_setschedparam k2s_pthread_setschedparam

#ifdef __cplusplus
}
#endif

#endif // __SWITCH__
