// [K2S] Instantiates the COM interface GUIDs (IID_IInStream, IID_IOutStream, ...) of 7-Zip (p7zip),
// which a normal 7-Zip build gets from its Guid translation unit. Needed by XP3ArchiveRepack.cpp.
// IID_IUnknown is already defined by sound/win32/tvpsnd.cpp (same standard COM GUID), so the copy
// that MyInitGuid.h would emit is renamed away in this TU.
#define IID_IUnknown K2S_7zip_IID_IUnknown_unused
#include "7zip/CPP/Common/MyInitGuid.h"
#include "7zip/CPP/7zip/IStream.h"
#include "7zip/CPP/7zip/ICoder.h"
#include "7zip/CPP/7zip/IProgress.h"
#include "7zip/CPP/7zip/IPassword.h"
#include "7zip/CPP/7zip/Archive/IArchive.h"
