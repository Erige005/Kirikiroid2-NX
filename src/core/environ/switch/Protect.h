// [K2S] Protect.h is not in the public zeas2 source. It declares TVPProtectInit(), the APK integrity /
// licence check of the Android app (SysInitIntf.cpp: `if (!TVPProtectInit()) return;`).
// It has nothing to do with game data protection; there is no APK to verify on Switch.
#pragma once
#ifdef __cplusplus
inline bool TVPProtectInit() { return true; }
#endif
