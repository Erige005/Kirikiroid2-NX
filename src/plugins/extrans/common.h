#ifndef commonH
#define commonH

#include "tp_stub.h"
#include "RenderManager.h"

//---------------------------------------------------------------------------
// [K2S] Kirikiroid2 removed GetScanLine/GetScanLineForWrite/GetPitchBytes from iTVPScanLineProvider
// (transhandler.h: providers expose textures). With the software render manager (the default,
// "renderer"="software") textures keep CPU pixels, so the original extrans pixel loops run unchanged
// through these adapters. Destination pixels must go through GetTextureForRender() (copy-on-write).
// Returns TJS_E_FAIL when a texture has no CPU access (OpenGL renderer).
//---------------------------------------------------------------------------
static inline tjs_error K2S_GetScanLine(iTVPScanLineProvider *p, tjs_int line, const void **out)
{
	iTVPTexture2D *t = p->GetTexture();
	*out = t ? t->GetScanLineForRead(line) : nullptr;
	return *out ? TJS_S_OK : TJS_E_FAIL;
}
static inline tjs_error K2S_GetScanLineForWrite(iTVPScanLineProvider *p, tjs_int line, void **out)
{
	iTVPTexture2D *t = p->GetTextureForRender();
	*out = t ? t->GetScanLineForWrite(line) : nullptr;
	return *out ? TJS_S_OK : TJS_E_FAIL;
}
static inline tjs_error K2S_GetPitchBytes(iTVPScanLineProvider *p, tjs_int *pitch)
{
	iTVPTexture2D *t = p->GetTexture();
	if (!t) return TJS_E_FAIL;
	*pitch = t->GetPitch();
	return TJS_S_OK;
}
static inline tjs_error K2S_GetPitchBytesForWrite(iTVPScanLineProvider *p, tjs_int *pitch)
{
	iTVPTexture2D *t = p->GetTextureForRender();
	if (!t) return TJS_E_FAIL;
	*pitch = t->GetPitch();
	return TJS_S_OK;
}

//---------------------------------------------------------------------------
static inline bool Clip(tjs_int &l, tjs_int &r, tjs_int cl, tjs_int cr)
{
	// 線分 l <-> r (l<r) を、 cl <-> cr (cl<cr) でクリッピングし、結果を l r に返す
	// クリッピングした結果、線分 l - r が残れば真、消滅すれば偽を返す
	if(l < cl) l = cl;
	if(r > cr) r = cr;
	if(l >= r) return false;
	return true;
}
//---------------------------------------------------------------------------
static inline tjs_uint32 Blend(tjs_uint32 a, tjs_uint32 b, tjs_int opa)
{
	// a と b を混合比 opa で混合して返す ( opa = 0 ～ 255, 0 = a, 255 = b )
	tjs_uint32 ret;
	tjs_uint32 tmp;

	tmp = a & 0x000000ff;  ret   = 0x000000ff & (tmp + (( (b & 0x000000ff) - tmp ) * opa >> 8));
	tmp = a & 0x0000ff00;  ret  |= 0x0000ff00 & (tmp + (( (b & 0x0000ff00) - tmp ) * opa >> 8));
	tmp = a & 0x00ff0000;  ret  |= 0x00ff0000 & (tmp + (( (b & 0x00ff0000) - tmp ) * opa >> 8));
	tmp = a >> 24;
	ret  |= (0x000000ff & (tmp + (( (b >> 24) - tmp ) * opa >> 8))) << 24;

	return ret;
}
//---------------------------------------------------------------------------
static inline void Swap_tjs_int(tjs_int &a, tjs_int &b)
{
	// a と b を入れ替える
	tjs_int tmp = a;
	a = b;
	b = tmp;
}
//---------------------------------------------------------------------------

#endif

