// Kirikiroid2-NX — credits shown by the About button of the file selector.
#include <string>

#define K2NX_VERSION "0.1.1-alpha"

const char *TVPGetK2NXVersion() { return K2NX_VERSION; }

std::string TVPGetK2SCreditsText()
{
	return
		"Kirikiroid2-NX " K2NX_VERSION "\n"
		"Nintendo Switch port of Kirikiroid2 1.3.9\n"
		"\n"
		"Creator & maintainer: Erige005 (Vietnam)\n"
		"github.com/Erige005/Kirikiroid2-NX\n"
		"gkdm.2k5@gmail.com\n"
		"\n"
		"Based on Kirikiroid2 by zeas2\n"
		"KiriKiri2 / KiriKiri Z by W.Dee and contributors\n"
		"Plugins: extrans (KiriKiri), layerExImage (Go Watanabe / CxImage)\n"
		"cocos2d-x, FFmpeg, OpenCV, libnx / devkitPro and more:\n"
		"see LICENSES on GitHub.";
}
