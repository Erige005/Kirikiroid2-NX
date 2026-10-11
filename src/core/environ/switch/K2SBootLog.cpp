// [K2S] Boot diagnostics, all written to sdmc:/switch/kirikiroid2/.
//  - k2s_boot.txt  : stages reached. The priority-101 constructor runs before every other C++ static
//                    initializer (libnx mounts the SD card in __appInit, before constructors).
//  - k2s_crash.txt : written by __libnx_exception_handler on a CPU exception (PC/LR + fp backtrace).
#include "tjsCommHead.h"
#include "tjsError.h"
#include <switch.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <exception>
#include <typeinfo>
#include <cxxabi.h>

typedef void (*InitFn)(void);
extern "C" InitFn __init_array_start[] __attribute__((visibility("hidden")));
extern "C" InitFn __init_array_end[] __attribute__((visibility("hidden")));

static const char *kBootLog = "sdmc:/switch/kirikiroid2/k2s_boot.txt";
static u64 s_imageBase = 0;

static u64 imageBase()
{
    if (!s_imageBase) {
        MemoryInfo mi;
        u32 pi;
        if (R_SUCCEEDED(svcQueryMemory(&mi, &pi, (u64)&imageBase)))
            s_imageBase = mi.addr; // .text is the first segment of the NRO
    }
    return s_imageBase;
}

extern "C" void K2SBootMark(const char *stage)
{
    FILE *f = fopen(kBootLog, "a");
    if (f) {
        fprintf(f, "%s\n", stage);
        fclose(f);
    }
}

static void onTerminate()
{
    FILE *f = fopen(kBootLog, "a");
    if (f) {
        fprintf(f, "!! std::terminate");
        if (std::type_info *t = abi::__cxa_current_exception_type())
            fprintf(f, " (uncaught exception type %s)", t->name());
        fprintf(f, "\n");
        if (std::exception_ptr ep = std::current_exception()) {
            try {
                std::rethrow_exception(ep);
            } catch (const std::exception &e) {
                fprintf(f, "   what(): %s\n", e.what());
            } catch (const TJS::eTJSError &e) {
                fprintf(f, "   TJS error: %s\n", e.GetMessage().AsStdString().c_str());
            } catch (...) {
            }
        }
        fclose(f);
    }
    svcExitProcess();
}

__attribute__((constructor(101))) static void K2SBootFirst()
{
    mkdir("sdmc:/switch", 0777);
    mkdir("sdmc:/switch/kirikiroid2", 0777);
    std::set_terminate(onTerminate);

    FILE *f = fopen(kBootLog, "w");
    if (!f) return;
    const u64 base = imageBase();
    fprintf(f, "0 build " __DATE__ " " __TIME__ "\n1 static-init start (image base 0x%lx, %ld init entries)\n", base,
            (long)(__init_array_end - __init_array_start));
    fflush(f);
    fsync(fileno(f));

    // Note: .init_array is inside RELRO (read-only after libnx relocates), so initializers can't be
    // re-dispatched from here. Crashes during static init are caught by __libnx_exception_handler below.
    fclose(f);
}

// last (default-priority entries run after all prioritized ones, so this marks "prioritized done" only
// if it had a priority; with none it lands among the TU constructors — keep it prioritized-last)
__attribute__((constructor(65535))) static void K2SBootPrioritizedDone()
{
    K2SBootMark("2 prioritized static-init done (TU constructors next)");
}

// ---- abort / assert: newlib's abort() just exits the process silently on Switch -------------
static void logBacktrace(FILE *f, u64 fp)
{
    const u64 base = imageBase();
    for (int i = 0; i < 32 && fp && (fp & 7) == 0; ++i) {
        u64 ret = ((u64 *)fp)[1];
        u64 next = ((u64 *)fp)[0];
        fprintf(f, "  #%d elf+0x%lx\n", i, ret - base);
        if (next <= fp) break;
        fp = next;
    }
}

extern "C" __attribute__((noreturn)) void abort(void)
{
    FILE *f = fopen(kBootLog, "a");
    if (f) {
        fprintf(f, "!! abort() called from elf+0x%lx, backtrace:\n",
                (u64)__builtin_return_address(0) - imageBase());
        logBacktrace(f, (u64)__builtin_frame_address(0));
        fclose(f);
    }
    svcExitProcess();
    __builtin_unreachable();
}

extern "C" __attribute__((noreturn)) void __assert_func(const char *file, int line, const char *func, const char *expr)
{
    FILE *f = fopen(kBootLog, "a");
    if (f) {
        fprintf(f, "!! assert failed: %s at %s:%d (%s)\n", expr ? expr : "?", file ? file : "?", line, func ? func : "?");
        logBacktrace(f, (u64)__builtin_frame_address(0));
        fclose(f);
    }
    svcExitProcess();
    __builtin_unreachable();
}

// ---- crash handler --------------------------------------------------------------------------
extern "C" {
alignas(16) u8 __nx_exception_stack[0x4000];
u64 __nx_exception_stack_size = sizeof(__nx_exception_stack);
// use our handler even when Atmosphère's creport/debugger would take the exception first
u32 __nx_exception_ignoredebug = 1;

void __libnx_exception_handler(ThreadExceptionDump *ctx)
{
    const u64 base = imageBase();
    FILE *f = fopen("sdmc:/switch/kirikiroid2/k2s_crash.txt", "w");
    if (f) {
        fprintf(f, "Kirikiroid2-Switch crash (build " __DATE__ " " __TIME__ ")\nerror_desc: 0x%x\nimage base: 0x%lx\n", ctx->error_desc, base);
        fprintf(f, "PC  = 0x%lx  (elf+0x%lx)\n", ctx->pc.x, ctx->pc.x - base);
        fprintf(f, "LR  = 0x%lx  (elf+0x%lx)\n", ctx->lr.x, ctx->lr.x - base);
        fprintf(f, "FP  = 0x%lx  SP = 0x%lx\nFAR = 0x%lx\n", ctx->fp.x, ctx->sp.x, ctx->far.x);
        for (int i = 0; i < 29; ++i)
            fprintf(f, "X%-2d = 0x%lx\n", i, ctx->cpu_gprs[i].x);
        u64 fp = ctx->fp.x;
        fprintf(f, "backtrace (elf offsets):\n");
        for (int i = 0; i < 32 && fp && (fp & 7) == 0; ++i) {
            u64 ret = ((u64 *)fp)[1];
            u64 next = ((u64 *)fp)[0];
            fprintf(f, "  #%d 0x%lx\n", i, ret - base);
            if (next <= fp) break;
            fp = next;
        }
        fclose(f);
    }
    fflush(stdout); // k2s_log.txt is block-buffered (main.cpp); after the crash file, in case this faults
    K2SBootMark("!! CPU exception, see k2s_crash.txt");
    svcExitProcess();
}
}
