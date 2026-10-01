// [K2S] AlphaMovie.dll for Kirikiroid2-NX: plays ".amv" (AJPM) alpha movies into a Layer.
// Reimplementation of T.Imoto's AlphaMovie plugin (kaede-software.com, KiriKiri license) from its TJS
// reference (manual/tjs_ref.html); the frame decoder is in amv_decoder.cpp. The APK 1.3.9 ships this DLL,
// the public Kirikiroid2 source doesn't.
#include "tjsCommHead.h"
#include "ncbind/ncbind.hpp"
#include "StorageIntf.h"
#include "amv_decoder.h"
#include <vector>
#include <string.h>

#define NCB_MODULE_NAME TJS_W("AlphaMovie.dll")

class AlphaMovie {
	AmvDecoder decoder;
	std::vector<uint8_t> data;	// whole file (the decoder works on memory)
	std::vector<uint32_t> rect;	// last decoded frame rect
	ttstr nextFile;
	bool hasNextFile = false;
	bool playing = false;
	bool loop = false, nextLoop = false;
	tjs_int frame = 0;		// next frame to show
	tjs_int preloadSamples = 4;
	tjs_int left = 0, top = 0;

	static void readFile(const ttstr &name, std::vector<uint8_t> &out) {
		tTJSBinaryStream *s = TVPCreateStream(TVPGetPlacedPath(name).IsEmpty() ? name : TVPGetPlacedPath(name), TJS_BS_READ);
		try {
			out.resize((size_t)s->GetSize());
			if (!out.empty()) s->ReadBuffer(out.data(), (tjs_uint)out.size());
		} catch (...) { delete s; throw; }
		delete s;
	}
	void openInternal(const ttstr &name) {
		std::vector<uint8_t> buf;
		readFile(name, buf);
		data.swap(buf);
		frame = 0;
		if (!decoder.open(data.data(), data.size()))
			TVPThrowExceptionMessage(TJS_W("AlphaMovie: unsupported or broken movie: %1"), name);
	}

public:
	AlphaMovie() {}

	void open(ttstr filename) {
		playing = false;
		hasNextFile = false;
		openInternal(filename);
	}
	void clear() {}
	void play() { playing = true; }
	void stop() { playing = false; }
	bool isPlaying() const { return playing; }
	void setPosition(tjs_int x, tjs_int y) { left = x; top = y; }
	void setNextMovieFile(ttstr filename) { nextFile = filename; hasNextFile = true; }

	// draws the next frame into `layer` and returns the frame number drawn (-1 when nothing is open)
	tjs_int showNextImage(iTJSDispatch2 *layer) {
		if (!layer || decoder.numFrames() <= 0) return -1;
		const tjs_int shown = frame;
		// like the original: the layer becomes the frame's coded rect (bounding box of the visible part),
		// placed at (left + x, top + y)
		int x = 0, y = 0, w = 0, h = 0;
		if (!decoder.decodeFrameRect(shown, &x, &y, &w, &h, rect))
			TVPThrowExceptionMessage(TJS_W("AlphaMovie: broken frame"));
		const bool empty = (w <= 0 || h <= 0);
		const tjs_int lw = empty ? 1 : w, lh = empty ? 1 : h;	// a 0x0 layer isn't allowed here
		tTJSVariant v;
		layer->PropGet(0, TJS_W("imageWidth"), nullptr, &v, layer);
		tjs_int iw = (tjs_int)v;
		layer->PropGet(0, TJS_W("imageHeight"), nullptr, &v, layer);
		tjs_int ih = (tjs_int)v;
		if (iw != lw || ih != lh) {
			tTJSVariant a[2] = { lw, lh };
			tTJSVariant *pa[2] = { &a[0], &a[1] };
			layer->FuncCall(0, TJS_W("setImageSize"), nullptr, nullptr, 2, pa, layer);
			layer->FuncCall(0, TJS_W("setSizeToImageSize"), nullptr, nullptr, 0, nullptr, layer);
		}
		{
			tTJSVariant a[2] = { left + x, top + y };
			tTJSVariant *pa[2] = { &a[0], &a[1] };
			layer->FuncCall(0, TJS_W("setPos"), nullptr, nullptr, 2, pa, layer);
		}
		layer->PropGet(0, TJS_W("mainImageBufferForWrite"), nullptr, &v, layer);
		uint8_t *buf = reinterpret_cast<uint8_t *>((tjs_intptr_t)(tjs_int64)v);
		layer->PropGet(0, TJS_W("mainImageBufferPitch"), nullptr, &v, layer);
		tjs_int pitch = (tjs_int)v;
		if (buf) {
			if (empty) memset(buf, 0, 4);
			else for (int r = 0; r < h; r++) memcpy(buf + r * pitch, &rect[(size_t)r * w], (size_t)w * 4);
		}
		{
			tTJSVariant a[4] = { 0, 0, lw, lh };
			tTJSVariant *pa[4] = { &a[0], &a[1], &a[2], &a[3] };
			layer->FuncCall(0, TJS_W("update"), nullptr, nullptr, 4, pa, layer);
		}
		// advance
		if (frame + 1 < decoder.numFrames()) {
			frame++;
		} else if (hasNextFile) {
			hasNextFile = false;
			loop = nextLoop;
			openInternal(nextFile);
		} else if (loop) {
			frame = 0;
		}
		return shown;
	}

	tjs_int getNumOfFrame() const { return decoder.numFrames(); }
	tjs_int getFrame() const { return frame; }
	void setFrame(tjs_int f) { if (f >= 0 && f < decoder.numFrames()) frame = f; }
	bool getLoop() const { return loop; }
	void setLoop(bool b) { loop = b; }
	bool getNextLoop() const { return nextLoop; }
	void setNextLoop(bool b) { nextLoop = b; }
	tjs_int getPreloadSamples() const { return preloadSamples; }
	void setPreloadSamples(tjs_int n) { preloadSamples = n; }
	tjs_int getLeft() const { return left; }
	void setLeft(tjs_int x) { left = x; }
	tjs_int getTop() const { return top; }
	void setTop(tjs_int y) { top = y; }
	tjs_int getScreenWidth() const { return decoder.width(); }
	tjs_int getScreenHeight() const { return decoder.height(); }
	tjs_int getFPSScale() const { return decoder.fpsScale(); }
	tjs_int getFPSRate() const { return decoder.fpsRate(); }
};

// Games subclass it without calling super.AlphaMovie() (e.g. Akatsuki no Goei's plugin_alphamovie.ks), which the
// original DLL tolerates: create the native instance on first use instead of failing with "Invalid object context".
NCB_GET_INSTANCE_HOOK(AlphaMovie)
{
	NCB_INSTANCE_GETTER(objthis) {
		ClassT *obj = GetNativeInstance(objthis);
		if (!obj) {
			obj = new ClassT();
			SetNativeInstance(objthis, obj);
		}
		return obj;
	}
	~NCB_GET_INSTANCE_HOOK_CLASS() {}
};

NCB_REGISTER_CLASS(AlphaMovie) {
	NCB_CONSTRUCTOR(());
	NCB_METHOD(open);
	NCB_METHOD(clear);
	NCB_METHOD(showNextImage);
	NCB_METHOD(isPlaying);
	NCB_METHOD(play);
	NCB_METHOD(stop);
	NCB_METHOD(setPosition);
	NCB_METHOD(setNextMovieFile);
	NCB_PROPERTY_RO(numOfFrame, getNumOfFrame);
	NCB_PROPERTY(frame, getFrame, setFrame);
	NCB_PROPERTY(loop, getLoop, setLoop);
	NCB_PROPERTY(nextLoop, getNextLoop, setNextLoop);
	NCB_PROPERTY(preloadSamples, getPreloadSamples, setPreloadSamples);
	NCB_PROPERTY(left, getLeft, setLeft);
	NCB_PROPERTY(top, getTop, setTop);
	NCB_PROPERTY_RO(screenWidth, getScreenWidth);
	NCB_PROPERTY_RO(screenHeight, getScreenHeight);
	NCB_PROPERTY_RO(FPSScale, getFPSScale);
	NCB_PROPERTY_RO(FPSRate, getFPSRate);
}
