// [K2S] Nintendo Switch entry point (replaces project/android/jni/src/SDL_android_main.cpp).
#include <switch.h>
#include <stdio.h>
#include <sys/stat.h>
#include "cocos2d/AppDelegate.h"

// stdout/stderr: to `nxlink -s` when netloaded with a listening PC, otherwise to a log file on the SD card
// (sdmc:/switch/kirikiroid2/k2s_log.txt), so crashes on real hardware can be diagnosed.
static void setupLogging()
{
    if (nxlinkStdio() >= 0) return;
    mkdir("sdmc:/switch", 0777);
    mkdir("sdmc:/switch/kirikiroid2", 0777);
    // keep the previous run's log (e.g. the session that saved, when the bug shows after a relaunch)
    remove("sdmc:/switch/kirikiroid2/k2s_log.prev.txt");
    rename("sdmc:/switch/kirikiroid2/k2s_log.txt", "sdmc:/switch/kirikiroid2/k2s_log.prev.txt");
    if (freopen("sdmc:/switch/kirikiroid2/k2s_log.txt", "w", stdout)) {
        // line-buffered: unbuffered stdout made every character a separate SD-card write (slow when
        // engines/scripts log a lot). Crashes still keep everything up to the last complete line.
        setvbuf(stdout, nullptr, _IOLBF, 4096);
        dup2(fileno(stdout), fileno(stderr));
    }
    printf("Kirikiroid2-Switch log (build " __DATE__ " " __TIME__ ")\n");
}

extern "C" void K2SBootMark(const char *stage); // K2SBootLog.cpp

int main(int argc, char **argv)
{
    K2SBootMark("3 main");
    socketInitializeDefault();
    setupLogging();
    K2SBootMark("4 logging ready");
    Result rc0 = romfsInit();     // UI assets from APK 1.3.9 (tools/extract_apk_assets.sh)
    K2SBootMark(R_SUCCEEDED(rc0) ? "5 romfs ok" : "5 romfs FAILED");

    TVPAppDelegate app;
    K2SBootMark("6 app delegate created, entering run()");
    int rc = cocos2d::Application::getInstance()->run();
    K2SBootMark("7 run() returned");

    romfsExit();
    socketExit();
    return rc;
}
