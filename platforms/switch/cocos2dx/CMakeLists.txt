# Builds cocos2d-x 3.11.1 (vendor/cocos2d-x/current-3.11.1) as a static library for Nintendo Switch.
# cocos is compiled as CC_PLATFORM_LINUX with __SWITCH__ patches (see vendor branch `switch`):
# OpenGL ES 2.0 via mesa + switch-glfw, libnx touch/swkbd/shared-font.
# Only the modules Kirikiroid2 1.3.9 uses are built (no audio, physics, lua/js, storage).
cmake_minimum_required(VERSION 3.13)
project(cocos2d_switch C CXX)

# cocos2d-x 3.11.1 = the version inside APK 1.3.9 (D-017). Switch patches: patches/0001-switch-platform.patch
set(COCOS_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../vendor/cocos2d-x/current-3.11.1)
get_filename_component(COCOS_ROOT ${COCOS_ROOT} ABSOLUTE)
set(COCOS_DIR ${COCOS_ROOT}/cocos)
set(EXT_DIR ${COCOS_ROOT}/external)

# The module lists use `if(LINUX)` and paths relative to cocos/.
set(LINUX TRUE)
foreach(m 2d 3d platform math renderer base deprecated ui editor-support/cocostudio)
  include(${COCOS_DIR}/${m}/CMakeLists.txt)
endforeach()
include(${COCOS_ROOT}/extensions/CMakeLists.txt)
# The module files also call include_directories() with host paths (/usr/include ...): drop them.
set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "")

set(COCOS_SRC
  cocos2d.cpp
  ${COCOS_2D_SRC} ${COCOS_3D_SRC} ${COCOS_PLATFORM_SRC} ${COCOS_MATH_SRC} ${COCOS_RENDERER_SRC}
  ${COCOS_BASE_SRC} ${COCOS_DEPRECATED_SRC} ${COCOS_UI_SRC} ${COCOS_CS_SRC}
  network/HttpClient.cpp
)
list(TRANSFORM COCOS_SRC PREPEND ${COCOS_DIR}/)

# extensions: only GUI/CCScrollView (TableView) is used by Kirikiroid2
set(EXT_SRC
  ${COCOS_ROOT}/extensions/GUI/CCScrollView/CCScrollView.cpp
  ${COCOS_ROOT}/extensions/GUI/CCScrollView/CCTableView.cpp
  ${COCOS_ROOT}/extensions/GUI/CCScrollView/CCTableViewCell.cpp
)

set(EXTERNAL_SRC
  ${EXT_DIR}/tinyxml2/tinyxml2.cpp
  ${EXT_DIR}/unzip/ioapi.cpp ${EXT_DIR}/unzip/ioapi_mem.cpp ${EXT_DIR}/unzip/unzip.cpp
  ${EXT_DIR}/xxhash/xxhash.c
  ${EXT_DIR}/clipper/clipper.cpp
  ${EXT_DIR}/xxtea/xxtea.cpp
  ${EXT_DIR}/flatbuffers/idl_parser.cpp ${EXT_DIR}/flatbuffers/idl_gen_text.cpp
)

# Particle Universe (PU) 3D particles: referenced by cocostudio's Particle3D reader.
file(GLOB_RECURSE PARTICLE3D_SRC ${COCOS_ROOT}/extensions/Particle3D/*.cpp)

add_library(cocos2d STATIC ${COCOS_SRC} ${EXT_SRC} ${EXTERNAL_SRC} ${PARTICLE3D_SRC}
  ${COCOS_DIR}/base/CCController-switch.cpp
  ${CMAKE_CURRENT_LIST_DIR}/SimpleAudioEngine-switch.cpp)

target_compile_definitions(cocos2d PUBLIC
  LINUX
  CC_USE_PHYSICS=0 CC_USE_3D_PHYSICS=0 CC_USE_NAVMESH=0
  CC_ENABLE_CHIPMUNK_INTEGRATION=0 CC_ENABLE_BOX2D_INTEGRATION=0 CC_ENABLE_BULLET_INTEGRATION=0
  CC_USE_TIFF=0 CC_USE_WEBP=1
  USE_FILE32API
  $<$<CONFIG:Debug>:COCOS2D_DEBUG=1>
)

target_include_directories(cocos2d PUBLIC
  ${COCOS_DIR}
  ${COCOS_DIR}/platform
  ${COCOS_DIR}/platform/desktop
  ${COCOS_DIR}/editor-support
  ${COCOS_DIR}/editor-support/cocostudio
  ${COCOS_DIR}/audio/include
  ${COCOS_ROOT}
  ${COCOS_ROOT}/extensions
  ${EXT_DIR}
  ${EXT_DIR}/tinyxml2
  ${EXT_DIR}/unzip
  ${EXT_DIR}/xxhash
  ${EXT_DIR}/ConvertUTF
  ${EXT_DIR}/edtaa3func
  ${EXT_DIR}/poly2tri ${EXT_DIR}/poly2tri/common ${EXT_DIR}/poly2tri/sweep
  ${EXT_DIR}/clipper
  ${EXT_DIR}/xxtea
  ${DEVKITPRO}/portlibs/switch/include/freetype2
  ${DEVKITPRO}/portlibs/switch/include/GLFW
  ${DEVKITPRO}/portlibs/switch/include/webp
)

# PUBLIC: cocos 3.10 headers need these with GCC 15 in every consumer (incl. Kirikiroid2 code).
target_compile_options(cocos2d PUBLIC
  $<$<COMPILE_LANGUAGE:CXX>:-fpermissive -Wno-template-body>
  -Wno-deprecated-declarations -Wno-invalid-offsetof -Wno-narrowing
)
set_target_properties(cocos2d PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

# Libraries we cross-compile ourselves (tools/portlibs/*.sh), e.g. libiconv.
if(NOT K2S_PORTLIBS)
  set(K2S_PORTLIBS $ENV{HOME}/k2s-portlibs)
endif()
target_include_directories(cocos2d BEFORE PUBLIC ${K2S_PORTLIBS}/include)
target_link_libraries(cocos2d PUBLIC ${K2S_PORTLIBS}/lib/libiconv.a)

find_package(PkgConfig REQUIRED)
pkg_check_modules(COCOS_DEPS REQUIRED IMPORTED_TARGET freetype2 libpng libjpeg libwebp libcurl zlib)
# glfw3.pc requires a 'vulkan' package that the portlibs don't ship, so link glfw by hand.
target_link_libraries(cocos2d PUBLIC glfw3 PkgConfig::COCOS_DEPS EGL GLESv2 glapi drm_nouveau nx m)
