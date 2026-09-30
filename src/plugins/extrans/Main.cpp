//---------------------------------------------------------------------------
// [K2S] extrans.dll built into Kirikiroid2 for Switch.
// Original: krkrz/SamplePlugin extrans (W.Dee and contributors, KiriKiri license),
// https://github.com/krkrz/SamplePlugin/tree/master/extrans — the transition sources in this folder are
// unmodified except for the <windows.h> include. APK 1.3.9 ships these transitions (wave, mosaic, turn,
// rotatezoom, rotatevanish, rotateswap, ripple) but the public zeas2 source does not.
// The DLL entry points V2Link/V2Unlink are replaced by ncbind registration, the same way the other
// built-in plugins (e.g. wutcwf.cpp) are wired.
//---------------------------------------------------------------------------
#include "ncbind/ncbind.hpp"
#include "wave.h"
#include "mosaic.h"
#include "turn.h"
#include "rotatetrans.h"
#include "ripple.h"

#define NCB_MODULE_NAME TJS_W("extrans.dll")

static void extrans_init()
{
	RegisterWaveTransHandlerProvider();
	RegisterMosaicTransHandlerProvider();
	RegisterTurnTransHandlerProvider();
	RegisterRotateTransHandlerProvider();
	RegisterRippleTransHandlerProvider();
}

static void extrans_uninit()
{
	UnregisterWaveTransHandlerProvider();
	UnregisterMosaicTransHandlerProvider();
	UnregisterTurnTransHandlerProvider();
	UnregisterRotateTransHandlerProvider();
	UnregisterRippleTransHandlerProvider();
}

NCB_PRE_REGIST_CALLBACK(extrans_init);
NCB_POST_UNREGIST_CALLBACK(extrans_uninit);
