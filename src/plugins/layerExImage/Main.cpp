//---------------------------------------------------------------------------
// [K2S] layerExImage.dll built into Kirikiroid2 for Switch.
// Original: wtnbgo/layerExImage (Go Watanabe; parts from CxImage, see LICENSE.TXT),
// https://github.com/wtnbgo/layerExImage. APK 1.3.9 ships it (light, colorize, modulate, noise,
// generateWhiteNoise, gaussianBlur) but the public zeas2 source does not.
// Registration follows the other built-in layer plugins (layerExMovie.cpp): layerExBase_GL gives the
// software render manager's pixel buffer.
//---------------------------------------------------------------------------
#include "ncbind/ncbind.hpp"
#include "LayerExImage.h"

#define NCB_MODULE_NAME TJS_W("layerExImage.dll")

NCB_GET_INSTANCE_HOOK(layerExImage)
{
	NCB_INSTANCE_GETTER(objthis) {
		ClassT* obj = GetNativeInstance(objthis);
		if (!obj) {
			obj = new ClassT(objthis);
			SetNativeInstance(objthis, obj);
		}
		obj->reset();
		return obj;
	}
	~NCB_GET_INSTANCE_HOOK_CLASS () {
	}
};

NCB_ATTACH_CLASS_WITH_HOOK(layerExImage, Layer) {
	NCB_METHOD(light);
	NCB_METHOD(colorize);
	NCB_METHOD(modulate);
	NCB_METHOD(noise);
	NCB_METHOD(generateWhiteNoise);
	NCB_METHOD(gaussianBlur);
}
