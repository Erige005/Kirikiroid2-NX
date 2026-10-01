// Kirikiroid2-NX — credits shown by the About button of the file selector.
#include <string>

// K2NX_VERSION comes from platforms/switch/CMakeLists.txt (same value as the NACP shown by hbmenu)
#ifndef K2NX_VERSION
#define K2NX_VERSION "dev"
#endif

const char *TVPGetK2NXVersion() { return K2NX_VERSION; }

std::string TVPGetK2SCreditsText()
{
	return
		"Kirikiroid2-NX " K2NX_VERSION "\n"
		"Nintendo Switch port of Kirikiroid2 1.3.9\n"
		"\n"
		"Creator & maintainer: Erige005 (from Vietnam)\n"
		"github.com/Erige005/Kirikiroid2-NX\n"
		"gkdm.2k5@gmail.com\n"
		"\n"
		"Based on Kirikiroid2 by zeas2\n"
		"KiriKiri2 / KiriKiri Z by W.Dee and contributors\n"
		"Plugins: extrans (KiriKiri), layerExImage, layerExDraw, fstat (Go Watanabe;\n"
		"layerExDraw / fstat ports from krkr2), AlphaMovie format (T.Imoto)\n"
		"cocos2d-x, Blend2D, FFmpeg, OpenCV, libnx / devkitPro and more:\n"
		"see LICENSES on GitHub.";
}
