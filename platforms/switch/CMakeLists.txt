# Kirikiroid2 (zeas2 1.3.9) for Nintendo Switch — top-level build.
# Source lists mirror src/core/Android.mk and src/plugins/Android.mk of the original ndk-build,
# with environ/android replaced by environ/switch.
cmake_minimum_required(VERSION 3.13)
project(kirikiroid2_switch C CXX)

# Release = what the Android build used (project/android/jni/Application.mk: -O3 -DNDEBUG).
# The software renderer and plugins like layerExImage are CPU-bound; Debug (-Og) is several times slower.
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

add_subdirectory(cocos2dx)

set(K2_SRC ${CMAKE_CURRENT_LIST_DIR}/../../src)
get_filename_component(K2_SRC ${K2_SRC} ABSOLUTE)
set(CORE ${K2_SRC}/core)
if(NOT K2S_PORTLIBS)
  set(K2S_PORTLIBS $ENV{HOME}/k2s-portlibs)
endif()
set(DKP_PORTLIBS $ENV{DEVKITPRO}/portlibs/switch)

# ---- core (LOCAL_MODULE := krkr2) -----------------------------------------------------------
macro(k2_glob var)
  file(GLOB _g ${ARGN})
  list(APPEND ${var} ${_g})
endmacro()

set(CORE_SRC)
k2_glob(CORE_SRC
  ${CORE}/visual/*.cpp
  ${CORE}/base/7zip/C/*.c
  ${CORE}/base/*.cpp ${CORE}/base/win32/*.cpp
  ${CORE}/environ/*.cpp ${CORE}/environ/cocos2d/*.cpp ${CORE}/environ/switch/*.cpp
  ${CORE}/environ/ConfigManager/*.cpp
  ${CORE}/environ/ui/*.cpp ${CORE}/environ/ui/extension/*.cpp
  ${CORE}/extension/*.cpp
  ${CORE}/movie/*.cpp ${CORE}/movie/*/*.cpp
  ${CORE}/msg/*.cpp
  ${CORE}/sound/*.cpp ${CORE}/sound/win32/*.cpp
  ${CORE}/tjs2/*.cpp
  ${CORE}/utils/*.c ${CORE}/utils/*.cpp ${CORE}/utils/encoding/*.c
  ${CORE}/utils/minizip/*.c ${CORE}/utils/minizip/*.cpp ${CORE}/utils/win32/*.cpp
  ${CORE}/visual/gl/*.cpp ${CORE}/visual/ogl/*.cpp ${CORE}/visual/win32/*.cpp
  # ARM NEON module (ARM_neon in project/android/jni/Android.mk)
  ${CORE}/visual/ARM/*.cpp ${CORE}/sound/ARM/*.c
)
# base/7zip = LZMA SDK 16.04 (tools/fetch_lzma_sdk.sh); the author's trimmed copy was not published.
# Only the C decoder/encoder (single-threaded, _7ZIP_ST) and the C++ parts XP3ArchiveRepack uses.
set(SZ ${CORE}/base/7zip)
list(REMOVE_ITEM CORE_SRC ${SZ}/C/LzFindMt.c ${SZ}/C/MtCoder.c ${SZ}/C/DllSecur.c
                          ${SZ}/C/Bcj2Enc.c)
set(SZ_CPP_SRC
  ${SZ}/CPP/7zip/Archive/7z/7zOut.cpp ${SZ}/CPP/7zip/Archive/7z/7zEncode.cpp
  ${SZ}/CPP/7zip/Archive/7z/7zSpecStream.cpp ${SZ}/CPP/7zip/Archive/Common/CoderMixer2.cpp
  ${SZ}/CPP/7zip/Common/CreateCoder.cpp ${SZ}/CPP/7zip/Common/FilterCoder.cpp
  ${SZ}/CPP/7zip/Common/MethodProps.cpp ${SZ}/CPP/7zip/Common/StreamBinder.cpp
  ${SZ}/CPP/7zip/Common/LimitedStreams.cpp ${SZ}/CPP/7zip/Common/InBuffer.cpp
  ${SZ}/CPP/7zip/Archive/7z/7zHeader.cpp
  ${SZ}/CPP/7zip/Common/StreamObjects.cpp ${SZ}/CPP/7zip/Common/StreamUtils.cpp ${SZ}/CPP/7zip/Common/OutBuffer.cpp
  ${SZ}/CPP/7zip/Compress/LzmaEncoder.cpp
  ${SZ}/CPP/Common/MyString.cpp ${SZ}/CPP/Common/MyVector.cpp ${SZ}/CPP/Common/MyWindows.cpp
  ${SZ}/CPP/Common/IntToString.cpp ${SZ}/CPP/Common/StringConvert.cpp
  ${SZ}/CPP/Windows/Synchronization.cpp ${SZ}/CPP/myWindows/wine_date_and_time.cpp
  ${SZ}/CPP/Windows/PropVariant.cpp ${SZ}/CPP/7zip/Common/InOutTempBuffer.cpp
  ${SZ}/CPP/Windows/FileIO.cpp ${SZ}/CPP/Windows/FileDir.cpp ${SZ}/CPP/Windows/FileFind.cpp
  ${SZ}/CPP/Windows/FileName.cpp ${SZ}/CPP/Windows/TimeUtils.cpp ${SZ}/CPP/Common/MyString.cpp
  ${SZ}/CPP/Common/Wildcard.cpp ${SZ}/CPP/Common/UTFConvert.cpp
)
list(APPEND CORE_SRC ${SZ_CPP_SRC})
# p7zip's Windows emulation (windows.h etc.) only for 7-Zip C++ and the two files that include it,
# so the rest of the engine never sees a fake windows.h.
set(SZ_USERS ${SZ_CPP_SRC} ${CORE}/environ/XP3ArchiveRepack.cpp ${CORE}/environ/switch/K2S7zipGuids.cpp)
set(SZ_DEFS _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _REENTRANT ENV_UNIX BREAK_HANDLER UNICODE _UNICODE UNIX_USE_WIN_FILE)
set_source_files_properties(${SZ_USERS} PROPERTIES
  INCLUDE_DIRECTORIES "${SZ}/CPP/myWindows;${SZ}/CPP/include_windows;${SZ}/CPP"
  COMPILE_DEFINITIONS "${SZ_DEFS}"
  COMPILE_OPTIONS "-include;${SZ}/CPP/myWindows/StdAfx.h")
# p7zip's POSIX C/Threads.c (pthread-based) includes its fake "windows.h" for the basic types
set_source_files_properties(${SZ}/C/Threads.c PROPERTIES
  INCLUDE_DIRECTORIES "${SZ}/CPP/myWindows;${SZ}/CPP/include_windows;${SZ}/CPP"
  COMPILE_DEFINITIONS "_REENTRANT;ENV_UNIX")
# p7zip itself reads real `struct stat` times through the newlib macros that k2s_compat.h #undefs
set_source_files_properties(${SZ_CPP_SRC} PROPERTIES
  COMPILE_DEFINITIONS "${SZ_DEFS};K2S_KEEP_STAT_MACROS")
list(APPEND CORE_SRC
  ${CORE}/environ/win32/SystemControl.cpp
  ${CORE}/msg/win32/MsgImpl.cpp
  ${CORE}/msg/win32/OptionsDesc.cpp
)
list(REMOVE_ITEM CORE_SRC
  ${CORE}/visual/Resampler.cpp
  ${CORE}/base/win32/FuncStubs.cpp ${CORE}/base/win32/SusieArchive.cpp
  ${CORE}/environ/MainFormUnit.cpp
  ${CORE}/sound/xmmlib.cpp ${CORE}/sound/WaveFormatConverter_SSE.cpp
  ${CORE}/visual/win32/GDIFontRasterizer.cpp ${CORE}/visual/win32/NativeFreeTypeFace.cpp
  ${CORE}/visual/win32/TVPSysFont.cpp ${CORE}/visual/win32/VSyncTimingThread.cpp
)

set(CORE_INC
  ${CORE} ${CORE}/base ${CORE}/base/win32 ${CORE}/environ ${CORE}/environ/win32 ${CORE}/environ/switch
  ${CORE}/environ/sdl ${CORE}/msg ${CORE}/msg/win32 ${CORE}/extension ${CORE}/sound ${CORE}/sound/win32
  ${CORE}/tjs2 ${CORE}/utils ${CORE}/utils/win32 ${CORE}/visual ${CORE}/visual/ARM ${CORE}/visual/win32
  ${K2_SRC}/plugins
  ${CMAKE_CURRENT_LIST_DIR}/include_shims
  ${K2S_PORTLIBS}/include
  ${DKP_PORTLIBS}/include/freetype2
  ${DKP_PORTLIBS}/include/opus
  ${DKP_PORTLIBS}/include/SDL2
  ${K2S_PORTLIBS}/include/jxrlib
)

# Kirikiroid2-NX version: NACP (hbmenu) and the About/credits text
set(K2NX_VERSION "1.0.1")
set(K2_DEFS TJS_TEXT_OUT_CRLF __STDC_CONSTANT_MACROS USE_UNICODE_FSTRING _7ZIP_ST typeof=decltype)
# -fno-delete-null-pointer-checks: TJS2 (tTJSVariantString::GetLength etc.) tests `if (!this)` because an
# empty ttstr holds a NULL pointer. The 2018 NDK clang kept those checks; GCC 15 deletes them (UB) and
# empty strings then crash (FAR=0x3c in TVPRebuildAutoPathTable). -fno-strict-aliasing likewise matches
# what that code base was tuned for.
set(K2_CXX_FLAGS -fexceptions -fno-delete-null-pointer-checks -fno-strict-aliasing
                 -Wno-inconsistent-missing-override -Wno-register -Wno-write-strings
                 -Wno-deprecated-declarations -Wno-narrowing)

set(K2S_COMPAT ${CMAKE_CURRENT_LIST_DIR}/include_shims/k2s_compat.h)

# jxrlib headers need its portability switches (same as tools/portlibs/build_jxrlib.sh)
set_source_files_properties(${CORE}/visual/LoadJXR.cpp PROPERTIES COMPILE_DEFINITIONS "__ANSI__;DISABLE_PERF_MEASUREMENT")

add_library(krkr2core STATIC ${CORE_SRC})
target_compile_options(krkr2core PUBLIC -include ${K2S_COMPAT})
target_include_directories(krkr2core PUBLIC ${CORE_INC})
target_compile_definitions(krkr2core PUBLIC ${K2_DEFS})
target_compile_options(krkr2core PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${K2_CXX_FLAGS}> $<$<COMPILE_LANGUAGE:C>:-std=gnu17 -Wno-incompatible-pointer-types -fno-delete-null-pointer-checks -fno-strict-aliasing>)
set_target_properties(krkr2core PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
target_link_libraries(krkr2core PUBLIC cocos2d)

# ---- plugins (LOCAL_MODULE := kr2plugin) ----------------------------------------------------
set(PLUGIN_SRC)
k2_glob(PLUGIN_SRC ${K2_SRC}/plugins/*.cpp ${K2_SRC}/plugins/*.c)
# plugins the APK ships but the public source lacks (docs/PLUGINS.md), one folder each
k2_glob(PLUGIN_SRC ${K2_SRC}/plugins/extrans/*.cpp ${K2_SRC}/plugins/layerExImage/*.cpp
                   ${K2_SRC}/plugins/fstat/*.cpp ${K2_SRC}/plugins/layerExDraw/*.cpp
                   ${K2_SRC}/plugins/AlphaMovie/*.cpp ${K2_SRC}/plugins/windowEx/*.cpp)
list(APPEND PLUGIN_SRC ${K2_SRC}/plugins/ncbind/ncbind.cpp)
# fstat (from krkr2) uses std::filesystem
set_source_files_properties(${K2_SRC}/plugins/fstat/main.cpp PROPERTIES COMPILE_OPTIONS "-std=gnu++17")
# layerExDraw (from krkr2) is C++17 on blend2d (tools/portlibs/build_blend2d.sh)
set_source_files_properties(${K2_SRC}/plugins/layerExDraw/LayerExDraw.cpp PROPERTIES COMPILE_OPTIONS "-std=gnu++17")
add_library(kr2plugin STATIC ${PLUGIN_SRC})
target_include_directories(kr2plugin PRIVATE ${K2_SRC}/plugins ${K2_SRC}/plugins/ncbind ${CORE}
                           ${DKP_PORTLIBS}/include/freetype2)
target_compile_options(kr2plugin PRIVATE ${K2_CXX_FLAGS} -fvisibility=hidden)
set_target_properties(kr2plugin PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
target_link_libraries(kr2plugin PUBLIC krkr2core)

# ---- executable -----------------------------------------------------------------------------
# main.cpp lives in environ/switch and is globbed into krkr2core; the exe needs one TU of its own,
# so it is taken out of the library and compiled here.
list(REMOVE_ITEM CORE_SRC ${CORE}/environ/switch/main.cpp)
get_target_property(_core_src krkr2core SOURCES)
list(REMOVE_ITEM _core_src ${CORE}/environ/switch/main.cpp)
set_target_properties(krkr2core PROPERTIES SOURCES "${_core_src}")

add_executable(kirikiroid2 ${CORE}/environ/switch/main.cpp)
set_target_properties(kirikiroid2 PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

set(PL ${K2S_PORTLIBS}/lib)
target_link_libraries(kirikiroid2 PRIVATE
  -Wl,--whole-archive kr2plugin -Wl,--no-whole-archive   # plugins self-register (ncbAutoRegister)
  krkr2core
  ${PL}/libavformat.a ${PL}/libavfilter.a ${PL}/libavcodec.a ${PL}/libswscale.a ${PL}/libswresample.a ${PL}/libavutil.a
  ${PL}/libopencv_imgproc.a ${PL}/libopencv_core.a ${K2S_PORTLIBS}/share/OpenCV/3rdparty/lib/libtegra_hal.a
  ${PL}/libblend2d.a ${PL}/libunrar.a ${PL}/libbpg.a ${PL}/libjxr.a ${PL}/libiconv.a
  onig openal opusfile opus vorbisfile vorbis ogg archive lzma bz2 zstd lz4 expat
  SDL2 turbojpeg jpeg webp png16 freetype bz2 z
  cocos2d
)

# NRO metadata shown by hbmenu / app stores. Icon = Kirikiroid2's original launcher icon (APK
# res/drawable-xxhdpi-v4/ic_launcher.png) scaled to 256x256 by make_icon.py (see tools/prepare_assets.sh).
set(K2NX_ASSETS ${CMAKE_CURRENT_LIST_DIR}/../../../build)
nx_generate_nacp(kirikiroid2.nacp NAME "Kirikiroid2-NX" AUTHOR "Erige005 (Kirikiroid2 by zeas2)" VERSION "${K2NX_VERSION}")
nx_create_nro(kirikiroid2 NACP kirikiroid2.nacp ICON ${K2NX_ASSETS}/icon.jpg ROMFS ${K2NX_ASSETS}/romfs)

# credits text (environ/switch/K2SCredits.cpp) shows the same version as the NACP
set_source_files_properties(${CORE}/environ/switch/K2SCredits.cpp PROPERTIES COMPILE_DEFINITIONS "K2NX_VERSION=\"${K2NX_VERSION}\"")
