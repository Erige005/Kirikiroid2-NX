// [K2S] windowEx.dll for Kirikiroid2-NX.
// Kirikiroid2 APK 1.3.9 ships windowEx built in (WM_* names, ncHitTest, getSystemMetrics are in the binary), but the
// public 1.3.9 source does not. This is a reduced port of krkr2's version (https://github.com/2468785842/krkr2,
// cpp/plugins/windowEx.cpp), itself based on wtnbgo/windowEx (https://github.com/wtnbgo/windowEx).
// On Switch there is one fullscreen window without a frame, a system menu or Win32 messages, so like krkr2 every
// method only keeps script-visible state and returns neutral values. The point is that games calling these methods
// (e.g. Window.registerExEvent() in "Ninki Seiyuu") don't stop with "Member ... does not exist".
// Left out compared to krkr2: MenuItemEx, Debug.console extensions, Pad.registerExEvent (no Pad/MenuItem classes
// on Switch) and the System functions krkr2 re-attaches that Kirikiroid2 already provides.
#include "ncbind/ncbind.hpp"

#define NCB_MODULE_NAME TJS_W("windowEx.dll")

// Win32 non-client hit test codes (WinUser.h), exposed as Window.ncht* constants.
enum {
	HTERROR = -2, HTTRANSPARENT = -1, HTNOWHERE = 0, HTCLIENT = 1, HTCAPTION = 2, HTSYSMENU = 3, HTGROWBOX = 4,
	HTSIZE = HTGROWBOX, HTMENU = 5, HTHSCROLL = 6, HTVSCROLL = 7, HTMINBUTTON = 8, HTREDUCE = HTMINBUTTON,
	HTMAXBUTTON = 9, HTZOOM = HTMAXBUTTON, HTLEFT = 10, HTRIGHT = 11, HTTOP = 12, HTTOPLEFT = 13, HTTOPRIGHT = 14,
	HTBOTTOM = 15, HTBOTTOMLEFT = 16, HTBOTTOMRIGHT = 17, HTBORDER = 18,
};

// Window message numbers for Window.getNotificationNum()/getNotificationName() (subset of WinUser.h)
static const struct { const tjs_char *name; tjs_int num; } k2sWindowMessages[] = {
	{ TJS_W("NULL"), 0x0000 }, { TJS_W("CREATE"), 0x0001 }, { TJS_W("DESTROY"), 0x0002 }, { TJS_W("MOVE"), 0x0003 },
	{ TJS_W("SIZE"), 0x0005 }, { TJS_W("ACTIVATE"), 0x0006 }, { TJS_W("SETFOCUS"), 0x0007 },
	{ TJS_W("KILLFOCUS"), 0x0008 }, { TJS_W("ENABLE"), 0x000A }, { TJS_W("SETTEXT"), 0x000C },
	{ TJS_W("PAINT"), 0x000F }, { TJS_W("CLOSE"), 0x0010 }, { TJS_W("QUIT"), 0x0012 },
	{ TJS_W("ERASEBKGND"), 0x0014 }, { TJS_W("SHOWWINDOW"), 0x0018 }, { TJS_W("ACTIVATEAPP"), 0x001C },
	{ TJS_W("SETCURSOR"), 0x0020 }, { TJS_W("GETMINMAXINFO"), 0x0024 }, { TJS_W("WINDOWPOSCHANGING"), 0x0046 },
	{ TJS_W("WINDOWPOSCHANGED"), 0x0047 }, { TJS_W("POWER"), 0x0048 }, { TJS_W("COPYDATA"), 0x004A },
	{ TJS_W("CONTEXTMENU"), 0x007B }, { TJS_W("DISPLAYCHANGE"), 0x007E }, { TJS_W("NCHITTEST"), 0x0084 },
	{ TJS_W("NCMOUSEMOVE"), 0x00A0 }, { TJS_W("NCLBUTTONDOWN"), 0x00A1 }, { TJS_W("NCLBUTTONUP"), 0x00A2 },
	{ TJS_W("NCRBUTTONDOWN"), 0x00A4 }, { TJS_W("NCRBUTTONUP"), 0x00A5 }, { TJS_W("KEYDOWN"), 0x0100 },
	{ TJS_W("KEYUP"), 0x0101 }, { TJS_W("CHAR"), 0x0102 }, { TJS_W("SYSKEYDOWN"), 0x0104 },
	{ TJS_W("SYSKEYUP"), 0x0105 }, { TJS_W("COMMAND"), 0x0111 }, { TJS_W("SYSCOMMAND"), 0x0112 },
	{ TJS_W("TIMER"), 0x0113 }, { TJS_W("INITMENU"), 0x0116 }, { TJS_W("INITMENUPOPUP"), 0x0117 },
	{ TJS_W("MENUSELECT"), 0x011F }, { TJS_W("MOUSEMOVE"), 0x0200 }, { TJS_W("LBUTTONDOWN"), 0x0201 },
	{ TJS_W("LBUTTONUP"), 0x0202 }, { TJS_W("LBUTTONDBLCLK"), 0x0203 }, { TJS_W("RBUTTONDOWN"), 0x0204 },
	{ TJS_W("RBUTTONUP"), 0x0205 }, { TJS_W("MBUTTONDOWN"), 0x0207 }, { TJS_W("MBUTTONUP"), 0x0208 },
	{ TJS_W("MOUSEWHEEL"), 0x020A }, { TJS_W("ENTERMENULOOP"), 0x0211 }, { TJS_W("EXITMENULOOP"), 0x0212 },
	{ TJS_W("SIZING"), 0x0214 }, { TJS_W("MOVING"), 0x0216 }, { TJS_W("POWERBROADCAST"), 0x0218 },
	{ TJS_W("DEVICECHANGE"), 0x0219 }, { TJS_W("ENTERSIZEMOVE"), 0x0231 }, { TJS_W("EXITSIZEMOVE"), 0x0232 },
	{ TJS_W("DROPFILES"), 0x0233 }, { TJS_W("MOUSELEAVE"), 0x02A3 }, { TJS_W("NCMOUSELEAVE"), 0x02A2 },
	{ TJS_W("HOTKEY"), 0x0312 }, { TJS_W("APPCOMMAND"), 0x0319 },
};

struct WindowEx {
	WindowEx(iTJSDispatch2 *obj)
		: self(obj), sysMenu(NULL), disableResize(false), disableMove(false), enableNCMEvent(false),
		  enableWinMsgHook(false) {}
	~WindowEx() {
		if (sysMenu) sysMenu->Release();
	}

	static WindowEx *GetInstance(iTJSDispatch2 *obj) {
		return ncbInstanceAdaptor<WindowEx>::GetNativeInstance(obj);
	}

	// No-op methods: minimize, maximize, showRestore, focusMenuByKey, set/resetWindowIcon, setOverlayBitmap,
	// bringTo, sendToBack, registerExEvent (there are no window events to register for).
	static tjs_error nop(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		if (r) r->Clear();
		return TJS_S_OK;
	}

	// getWindowRect / getClientRect / getNormalRect -> %[ x, y, w, h ] from the window's own properties
	static tjs_error getWindowRect(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		if (!r) return TJS_S_OK;
		ncbPropAccessor win(obj);
		ncbDictionaryAccessor dict;
		dict.SetValue(TJS_W("x"), win.getIntValue(TJS_W("left")));
		dict.SetValue(TJS_W("y"), win.getIntValue(TJS_W("top")));
		dict.SetValue(TJS_W("w"), win.getIntValue(TJS_W("width")));
		dict.SetValue(TJS_W("h"), win.getIntValue(TJS_W("height")));
		iTJSDispatch2 *dsp = dict.GetDispatch();
		*r = tTJSVariant(dsp, dsp);
		return TJS_S_OK;
	}

	static tjs_error getClientRect(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		if (!r) return TJS_S_OK;
		ncbPropAccessor win(obj);
		ncbDictionaryAccessor dict;
		dict.SetValue(TJS_W("x"), 0);
		dict.SetValue(TJS_W("y"), 0);
		dict.SetValue(TJS_W("w"), win.getIntValue(TJS_W("innerWidth")));
		dict.SetValue(TJS_W("h"), win.getIntValue(TJS_W("innerHeight")));
		iTJSDispatch2 *dsp = dict.GetDispatch();
		*r = tTJSVariant(dsp, dsp);
		return TJS_S_OK;
	}

#define K2S_BOOL_PROP(name, member)                                                                  \
	static tjs_error get_##name(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) { \
		WindowEx *self = GetInstance(obj);                                                         \
		if (r) *r = (tjs_int)(self != NULL && self->member);                                       \
		return TJS_S_OK;                                                                           \
	}                                                                                              \
	static tjs_error set_##name(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) { \
		WindowEx *self = GetInstance(obj);                                                         \
		if (!self) return TJS_E_ACCESSDENYED;                                                      \
		self->member = n > 0 && !!p[0]->AsInteger();                                               \
		return TJS_S_OK;                                                                           \
	}
	K2S_BOOL_PROP(disableResize, disableResize)
	K2S_BOOL_PROP(disableMove, disableMove)
	K2S_BOOL_PROP(enableNCMouseEvent, enableNCMEvent)
#undef K2S_BOOL_PROP

	// property exSystemMenu: kept so scripts read back what they set
	static tjs_error get_exSystemMenu(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		WindowEx *self = GetInstance(obj);
		if (r) {
			if (self && self->sysMenu) *r = tTJSVariant(self->sysMenu, self->sysMenu);
			else r->Clear();
		}
		return TJS_S_OK;
	}
	static tjs_error set_exSystemMenu(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		WindowEx *self = GetInstance(obj);
		if (!self) return TJS_E_ACCESSDENYED;
		if (self->sysMenu) self->sysMenu->Release();
		self->sysMenu = (n > 0 && p[0]->Type() == tvtObject) ? p[0]->AsObject() : NULL;
		return TJS_S_OK;
	}

	// ncHitTest(x, y): the whole screen is client area
	static tjs_error ncHitTest(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		if (n < 2) return TJS_E_BADPARAMCOUNT;
		if (r) *r = (tjs_int)HTCLIENT;
		return TJS_S_OK;
	}

	// setMessageHook(on[, msg]): there are no Win32 messages, so hooks never fire
	static tjs_error setMessageHook(tTJSVariant *r, tjs_int n, tTJSVariant **p, iTJSDispatch2 *obj) {
		WindowEx *self = GetInstance(obj);
		if (!self) return TJS_E_ACCESSDENYED;
		if (n < 2) self->enableWinMsgHook = n >= 1 && !!p[0]->AsInteger();
		if (r) *r = (tjs_int)self->enableWinMsgHook;
		return TJS_S_OK;
	}

	void registerExEvent() {}

	static tjs_int getNotificationNum(ttstr key) {
		for (auto &m : k2sWindowMessages)
			if (key == m.name) return m.num;
		return -1;
	}
	static ttstr getNotificationName(tjs_int num) {
		for (auto &m : k2sWindowMessages)
			if (num == m.num) return m.name;
		return ttstr();
	}

private:
	iTJSDispatch2 *self;
	iTJSDispatch2 *sysMenu;
	bool disableResize, disableMove, enableNCMEvent, enableWinMsgHook;
};

// Games derive from Window, so create the native instance on first use (same as layerExImage/AlphaMovie)
NCB_GET_INSTANCE_HOOK(WindowEx)
{
	NCB_INSTANCE_GETTER(objthis) {
		ClassT *obj = GetNativeInstance(objthis);
		if (!obj) {
			obj = new ClassT(objthis);
			SetNativeInstance(objthis, obj);
		}
		return obj;
	}
	~NCB_GET_INSTANCE_HOOK_CLASS() {}
};

NCB_ATTACH_CLASS_WITH_HOOK(WindowEx, Window) {
	Variant(TJS_W("nchtError"), (tjs_int)(HTERROR & 0xFFFF));
	Variant(TJS_W("nchtTransparent"), (tjs_int)(HTTRANSPARENT & 0xFFFF));
	Variant(TJS_W("nchtNoWhere"), (tjs_int)HTNOWHERE);
	Variant(TJS_W("nchtClient"), (tjs_int)HTCLIENT);
	Variant(TJS_W("nchtCaption"), (tjs_int)HTCAPTION);
	Variant(TJS_W("nchtSysMenu"), (tjs_int)HTSYSMENU);
	Variant(TJS_W("nchtSize"), (tjs_int)HTSIZE);
	Variant(TJS_W("nchtGrowBox"), (tjs_int)HTGROWBOX);
	Variant(TJS_W("nchtMenu"), (tjs_int)HTMENU);
	Variant(TJS_W("nchtHScroll"), (tjs_int)HTHSCROLL);
	Variant(TJS_W("nchtVScroll"), (tjs_int)HTVSCROLL);
	Variant(TJS_W("nchtMinButton"), (tjs_int)HTMINBUTTON);
	Variant(TJS_W("nchtReduce"), (tjs_int)HTREDUCE);
	Variant(TJS_W("nchtMaxButton"), (tjs_int)HTMAXBUTTON);
	Variant(TJS_W("nchtZoom"), (tjs_int)HTZOOM);
	Variant(TJS_W("nchtLeft"), (tjs_int)HTLEFT);
	Variant(TJS_W("nchtRight"), (tjs_int)HTRIGHT);
	Variant(TJS_W("nchtTop"), (tjs_int)HTTOP);
	Variant(TJS_W("nchtTopLeft"), (tjs_int)HTTOPLEFT);
	Variant(TJS_W("nchtTopRight"), (tjs_int)HTTOPRIGHT);
	Variant(TJS_W("nchtBottom"), (tjs_int)HTBOTTOM);
	Variant(TJS_W("nchtBottomLeft"), (tjs_int)HTBOTTOMLEFT);
	Variant(TJS_W("nchtBottomRight"), (tjs_int)HTBOTTOMRIGHT);
	Variant(TJS_W("nchtBorder"), (tjs_int)HTBORDER);

	RawCallback(TJS_W("minimize"), &Class::nop, 0);
	RawCallback(TJS_W("maximize"), &Class::nop, 0);
	RawCallback(TJS_W("showRestore"), &Class::nop, 0);
	RawCallback(TJS_W("resetWindowIcon"), &Class::nop, 0);
	RawCallback(TJS_W("setWindowIcon"), &Class::nop, 0);
	RawCallback(TJS_W("getWindowRect"), &Class::getWindowRect, 0);
	RawCallback(TJS_W("getClientRect"), &Class::getClientRect, 0);
	RawCallback(TJS_W("getNormalRect"), &Class::getWindowRect, 0);
	RawCallback(TJS_W("disableResize"), &Class::get_disableResize, &Class::set_disableResize, 0);
	RawCallback(TJS_W("disableMove"), &Class::get_disableMove, &Class::set_disableMove, 0);
	RawCallback(TJS_W("setOverlayBitmap"), &Class::nop, 0);
	RawCallback(TJS_W("exSystemMenu"), &Class::get_exSystemMenu, &Class::set_exSystemMenu, 0);
	RawCallback(TJS_W("resetExSystemMenu"), &Class::nop, 0);
	RawCallback(TJS_W("enableNCMouseEvent"), &Class::get_enableNCMouseEvent, &Class::set_enableNCMouseEvent, 0);
	RawCallback(TJS_W("ncHitTest"), &Class::ncHitTest, 0);
	RawCallback(TJS_W("focusMenuByKey"), &Class::nop, 0);
	RawCallback(TJS_W("setMessageHook"), &Class::setMessageHook, 0);
	RawCallback(TJS_W("bringTo"), &Class::nop, 0);
	RawCallback(TJS_W("sendToBack"), &Class::nop, 0);

	Method(TJS_W("registerExEvent"), &Class::registerExEvent);
	Method(TJS_W("getNotificationNum"), &Class::getNotificationNum);
	Method(TJS_W("getNotificationName"), &Class::getNotificationName);
}
