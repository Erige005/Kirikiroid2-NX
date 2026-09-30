// [K2S] Nintendo Switch implementation of the platform layer (Platform.h + the functions that
// environ/android/AndroidUtils.cpp provides on Android). Behaviour follows the Android version;
// JNI calls are replaced by newlib/libnx equivalents.
#include <switch.h>
#include "tjsCommHead.h"
#include "Platform.h"
#include "StorageImpl.h"
#include "EventIntf.h"
#include "RenderManager.h"
#include "ConfigManager/LocaleConfigManager.h"
#include "platform/CCFileUtils.h"
#include "base/CCDirector.h"
#include "base/CCScheduler.h"
#include <EGL/egl.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>
#include <utime.h>
#include <unistd.h>
#include <sys/stat.h>
#include <malloc.h>

// ---- logging / timing -----------------------------------------------------------------------

void TVPPrintLog(const char *str) {
	printf("%s", str); // nxlinkStdio() forwards stdout to `nxlink -s`
}

tjs_uint32 TVPGetRoughTickCount32()
{
	struct timespec on;
	if (clock_gettime(CLOCK_MONOTONIC, &on) == 0)
		return (tjs_uint32)(on.tv_sec * 1000 + on.tv_nsec / 1000000);
	return 0;
}

void TVPRelinquishCPU() { svcSleepThread(0); }

// ---- memory ---------------------------------------------------------------------------------
// Android asks the Java side (ActivityManager); here the process memory pool is what matters:
// in title-takeover mode that is ~3.2 GB, in applet mode ~400 MB.

static void _queryMemory(u64 &total, u64 &used) {
	total = used = 0;
	svcGetInfo(&total, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0);
	svcGetInfo(&used, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0);
}

tjs_int TVPGetSystemFreeMemory() {
	u64 total, used;
	_queryMemory(total, used);
	return (tjs_int)((total - used) / (1024 * 1024));
}

tjs_int TVPGetSelfUsedMemory() {
	u64 total, used;
	_queryMemory(total, used);
	return (tjs_int)(used / (1024 * 1024));
}

void TVPGetMemoryInfo(TVPMemoryInfo &m) {
	u64 total, used;
	_queryMemory(total, used);
	m.MemTotal = total / 1024;
	m.MemFree = (total - used) / 1024;
	m.SwapTotal = 0;
	m.SwapFree = 0;
	m.VirtualTotal = total / 1024;
	m.VirtualUsed = used / 1024;
}

void TVPForceSwapBuffer() {
	eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW));
}

// ---- storage paths --------------------------------------------------------------------------
// Android lists mounted storages (/storage/emulated/0, SD cards...). On Switch there is one: the SD card.
// The engine must only ever see Unix-style paths ("/switch/..."): libnx resolves a leading '/' on the
// default device (sdmc), while a "sdmc:" prefix is parsed by TJS as a *media type* ("Not supported media
// type sdmc") and gets its trailing '/' stripped by the file selector ("sdmc:" = current directory).
// The UI already treats "/" as the root (FileSelectorForm RootPathLen = 1).

std::vector<std::string> TVPGetDriverPath() {
	return { "/" };
}

std::vector<std::string> TVPGetAppStoragePath() {
	return {}; // single root, like the iOS sandbox case in TVPBaseFileSelectorForm
}

bool TVPCheckStartupPath(const std::string &path) {
	// Android checks write permission (SAF); the SD card is always writable on a hacked Switch
	return true;
}

const char *TVPGetK2NXVersion(); // K2SCredits.cpp
std::string TVPGetPackageVersionString() {
	return std::string("NX ") + TVPGetK2NXVersion() + " (Kirikiroid2 1.3.9)";
}

std::string TVPGetDeviceID() {
	return "nintendo-switch";
}

void TVPControlAdDialog(int adType, int arg1, int arg2) {}
void TVPFetchSDCardPermission() {}
void TVPSendToOtherApp(const std::string &filename) {}
void TVPProcessInputEvents() {}

void TVPCheckAndSendDumps(const std::string &dumpdir, const std::string &packageName, const std::string &versionStr);
bool TVPCheckStartupArg() {
	return false; // Android: only dump check; no launch arguments on Switch (yet)
}

void Android_PushEvents(const std::function<void()> &func) {
	cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread(func);
}

// ---- language -------------------------------------------------------------------------------

std::string TVPGetCurrentLanguage() {
	// format expected by LocaleConfigManager: locale/<code>.xml, e.g. "en_us", "ja_jp", "zh_cn", "zh_tw"
	std::string ret = "en_us";
	u64 code = 0;
	SetLanguage lang;
	if (R_SUCCEEDED(setInitialize())) {
		if (R_SUCCEEDED(setGetSystemLanguage(&code)) && R_SUCCEEDED(setMakeLanguage(code, &lang))) {
			switch (lang) {
			case SetLanguage_JA: ret = "ja_jp"; break;
			case SetLanguage_ZHCN: case SetLanguage_ZHHANS: ret = "zh_cn"; break;
			case SetLanguage_ZHTW: case SetLanguage_ZHHANT: ret = "zh_tw"; break;
			default: ret = "en_us"; break;
			}
		}
		setExit();
	}
	return ret;
}

// ---- files ----------------------------------------------------------------------------------

static bool _TVPCreateFolders(const ttstr &folder)
{
	if (folder.IsEmpty()) return true;
	if (TVPCheckExistentLocalFolder(folder)) return true;

	const tjs_char *p = folder.c_str();
	tjs_int i = folder.GetLen() - 1;
	if (p[i] == TJS_W(':')) return true;
	while (i >= 0 && (p[i] == TJS_W('/') || p[i] == TJS_W('\\'))) i--;
	if (i >= 0 && p[i] == TJS_W(':')) return true;
	for (; i >= 0; i--) {
		if (p[i] == TJS_W(':') || p[i] == TJS_W('/') || p[i] == TJS_W('\\')) break;
	}
	ttstr parent(p, i + 1);
	if (!TVPCreateFolders(parent)) return false;
	return mkdir(folder.AsStdString().c_str(), 0777) == 0;
}

bool TVPCreateFolders(const ttstr &folder)
{
	if (folder.IsEmpty()) return true;
	const tjs_char *p = folder.c_str();
	tjs_int i = folder.GetLen() - 1;
	if (p[i] == TJS_W(':')) return true;
	if (p[i] == TJS_W('/') || p[i] == TJS_W('\\')) i--;
	return _TVPCreateFolders(ttstr(p, i + 1));
}

bool TVPWriteDataToFile(const ttstr &filepath, const void *data, unsigned int size) {
	// Android writes to "<name>.<time>.bak" first when the file exists, then swaps; keep that.
	std::string filename = filepath.AsStdString();
	cocos2d::FileUtils *fileutil = cocos2d::FileUtils::getInstance();
	if (fileutil->isFileExist(filename)) {
		char suffix[32];
		snprintf(suffix, sizeof(suffix), ".%d.bak", (int)time(nullptr));
		std::string tempname = filename + suffix;
		if (rename(filename.c_str(), tempname.c_str()) == 0) {
			FILE *fp = fopen(filename.c_str(), "wb");
			if (fp) {
				bool ret = fwrite(data, 1, size, fp) == size;
				fclose(fp);
				remove(tempname.c_str());
				return ret;
			}
			rename(tempname.c_str(), filename.c_str());
		}
		return false;
	}
	FILE *fp = fopen(filename.c_str(), "wb");
	if (!fp) return false;
	bool ret = fwrite(data, 1, size, fp) == size;
	fclose(fp);
	return ret;
}

bool TVPDeleteFile(const std::string &filename) {
	return unlink(filename.c_str()) == 0;
}

bool TVPRenameFile(const std::string &from, const std::string &to) {
	return rename(from.c_str(), to.c_str()) == 0;
}

bool TVP_stat(const char *name, tTVP_stat &s) {
	struct stat t;
	bool ret = !stat(name, &t);
	s.st_mode = t.st_mode;
	s.st_size = t.st_size;
	s.st_atime = t.st_atim.tv_sec;
	s.st_mtime = t.st_mtim.tv_sec;
	s.st_ctime = t.st_ctim.tv_sec;
	return ret;
}

bool TVP_stat(const tjs_char *name, tTVP_stat &s) {
	tTJSNarrowStringHolder holder(name);
	return TVP_stat(holder, s);
}

void TVP_utime(const char *name, time_t modtime) {
	struct utimbuf utb;
	utb.modtime = modtime;
	utb.actime = modtime;
	utime(name, &utb);
}

// ---- application ----------------------------------------------------------------------------

void TVPExitApplication(int code) {
	TVPDeliverCompactEvent(TVP_COMPACT_LEVEL_MAX);
	if (!TVPIsSoftwareRenderManager())
		iTVPTexture2D::RecycleProcess();
	exit(code);
}

// ---- IME ------------------------------------------------------------------------------------
// TODO(K2S): Android shows the soft keyboard over a text field and streams key events into cocos'
// IMEDispatcher. On Switch this should open swkbd (inline mode) — phase 2. Text input boxes that go
// through TVPShowSimpleInputBox already use swkbd (below).
void TVPShowIME(int x, int y, int w, int h) {}
void TVPHideIME() {}

// ---- dialogs --------------------------------------------------------------------------------

int TVPShowSimpleInputBox(ttstr &text, const ttstr &caption, const ttstr &prompt, const std::vector<ttstr> &vecButtons) {
	// Android: modal Java dialog with an EditText; returns the pressed button index (0 = OK).
	SwkbdConfig kbd;
	if (R_FAILED(swkbdCreate(&kbd, 0))) return -1;
	swkbdConfigMakePresetDefault(&kbd);
	std::string header = caption.AsStdString(), guide = prompt.AsStdString(), init = text.AsStdString();
	swkbdConfigSetHeaderText(&kbd, header.c_str());
	swkbdConfigSetGuideText(&kbd, guide.c_str());
	swkbdConfigSetInitialText(&kbd, init.c_str());
	char out[2048] = { 0 };
	Result rc = swkbdShow(&kbd, out, sizeof(out));
	swkbdClose(&kbd);
	if (R_FAILED(rc)) return vecButtons.size() > 1 ? 1 : -1; // cancelled -> second button ("cancel")
	text = out;
	return 0;
}

// TODO(K2S): proper modal message box. Android shows a Java AlertDialog while the engine thread waits
// and keeps swapping buffers. Switch has no separate UI thread, so this needs a small self-drawn modal
// loop (GLES + shared font) — tracked in PROGRESS.md. Until then: 1 button -> system error applet
// (shows the text, OK), several buttons -> error applet + first button.
int TVPShowSimpleMessageBox(const ttstr &text, const ttstr &caption, const std::vector<ttstr> &vecButtons) {
	std::string msg = caption.AsStdString() + "\n\n" + text.AsStdString();
	printf("[MessageBox] %s\n", msg.c_str());
	ErrorApplicationConfig c;
	if (R_SUCCEEDED(errorApplicationCreate(&c, msg.c_str(), nullptr))) {
		errorApplicationShow(&c);
	}
	return 0;
}

extern "C" int TVPShowSimpleMessageBox(const char *pszText, const char *pszTitle, unsigned int nButton, const char **btnText) {
	std::vector<ttstr> vecButtons;
	for (unsigned int i = 0; i < nButton; ++i) vecButtons.emplace_back(btnText[i]);
	return TVPShowSimpleMessageBox(ttstr(pszText), ttstr(pszTitle), vecButtons);
}

// ---- libc gaps used by libarchive (extracting archives keeps no Unix ownership on FAT/exFAT) ----
extern "C" {
struct passwd; struct group;
int lchown(const char *, uid_t, gid_t) { return 0; }
struct passwd *getpwnam(const char *) { return nullptr; }
struct group *getgrnam(const char *) { return nullptr; }
mode_t umask(mode_t) { return 022; }

// used by opencv (sysconf, posix_memalign) and libarchive (flock)
long sysconf(int name) {
	switch (name) {
	case _SC_PAGESIZE: return 0x1000;
	case _SC_NPROCESSORS_CONF:
	case _SC_NPROCESSORS_ONLN: return 3; // cores available to applications
	default: return -1;
	}
}
int posix_memalign(void **memptr, size_t alignment, size_t size) {
	void *p = memalign(alignment, size);
	if (!p) return 12; // ENOMEM
	*memptr = p;
	return 0;
}
int flock(int, int) { return 0; }
}
