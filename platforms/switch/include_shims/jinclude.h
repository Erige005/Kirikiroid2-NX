/* [K2S] Stand-in for libjpeg-turbo's internal jinclude.h (not installed by devkitPro's switch-libjpeg-turbo).
 * LoadJPEG.cpp only needs the standard headers and the classic helper macros it defines. */
#ifndef K2S_JINCLUDE_H
#define K2S_JINCLUDE_H
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MEMZERO(target, size)       memset((void *)(target), 0, (size_t)(size))
#define MEMCOPY(dest, src, size)    memcpy((void *)(dest), (const void *)(src), (size_t)(size))
#define SIZEOF(object)              ((size_t)sizeof(object))
#define JFREAD(file, buf, sizeofbuf) \
  ((size_t)fread((void *)(buf), (size_t)1, (size_t)(sizeofbuf), (file)))
#define JFWRITE(file, buf, sizeofbuf) \
  ((size_t)fwrite((const void *)(buf), (size_t)1, (size_t)(sizeofbuf), (file)))
#endif
