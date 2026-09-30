# Third-party components of Kirikiroid2-NX

Kirikiroid2-NX statically links the components below. Their licenses apply to the corresponding parts of the
`.nro`. Source code of this project and the scripts that build every component are in this repository
(`platforms/switch/tools/`), which also provides the means to relink against modified versions of the
LGPL-licensed libraries.

| Component | Version | License | Upstream |
|---|---|---|---|
| Kirikiroid2 (engine, UI) | 1.3.9 (zeas2 master) | KiriKiri license (BSD-style), see `LICENSE` | https://github.com/zeas2/Kirikiroid2 |
| KiriKiri2 / KiriKiri Z | (inside Kirikiroid2) | KiriKiri license, see `LICENSE` | https://github.com/krkrz |
| extrans plugin | krkrz/SamplePlugin (2017-12) | KiriKiri license | https://github.com/krkrz/SamplePlugin |
| layerExImage plugin | wtnbgo/layerExImage | author's license + CxImage (zlib-style), see `src/plugins/layerExImage/LICENSE.TXT` | https://github.com/wtnbgo/layerExImage |
| fstat plugin | krkr2 `cpp/plugins/fstat` (port of wtnbgo/fstat) | KiriKiri license | https://github.com/2468785842/krkr2 , https://github.com/wtnbgo/fstat |
| layerExDraw plugin | krkr2 `cpp/plugins/layerex_draw/blend2d` (reimplementation of wtnbgo/layerExDraw) | KiriKiri license | https://github.com/2468785842/krkr2 , https://github.com/wtnbgo/layerExDraw |
| Blend2D | d2027eb (2025-03-08) | zlib | https://github.com/blend2d/blend2d |
| cocos2d-x | 3.11.1 (+ Switch patches) | MIT | https://github.com/cocos2d/cocos2d-x |
| cocos2d-x 3rd-party bundle (tinyxml2, unzip, xxhash, ConvertUTF, poly2tri, clipper, flatbuffers, ...) | v3-deps-94 | zlib / MIT / BSD / Apache-2.0 (per component) | https://github.com/cocos2d/cocos2d-x-3rd-party-libs-bin |
| FFmpeg | 3.2.19 (LGPL build, no GPL parts) | LGPL-2.1-or-later | https://ffmpeg.org |
| OpenCV (core, imgproc) | 3.4.20 | BSD-3-Clause | https://opencv.org |
| p7zip / LZMA SDK | 16.02 / 16.04 | LGPL-2.1 (+ unRAR restriction) / public domain | https://sourceforge.net/projects/p7zip , https://www.7-zip.org/sdk.html |
| UnRAR | 5.6.5 | UnRAR license (freeware; may not be used to re-create the RAR compression algorithm) | https://www.rarlab.com |
| libbpg | 0.9.8 | BSD (libbpg) + LGPL (bundled libavcodec HEVC decoder) | https://bellard.org/bpg |
| jxrlib | 4creators/jxrlib | BSD-2-Clause | https://github.com/4creators/jxrlib |
| GNU libiconv | 1.17 | LGPL-2.1-or-later | https://www.gnu.org/software/libiconv |
| DroidSansFallback.ttf | (from Kirikiroid2 1.3.9 APK) | Apache-2.0 | Android Open Source Project |
| libnx | 4.12.0 | ISC | https://github.com/switchbrew/libnx |
| devkitPro portlibs: mesa, GLFW, SDL2, OpenAL Soft, FreeType, libpng, libjpeg-turbo, libwebp, libarchive, Oniguruma, Opus/opusfile, libvorbis/libogg, zlib, lz4, bzip2, xz, zstd, expat, curl | devkitpro/devkita64:20260219 | MIT / zlib / LGPL-2.0 (OpenAL Soft) / FTL / BSD / … (per component) | https://devkitpro.org |

The Kirikiroid2 UI resources (`.csb` layouts, images, locale files) and the icon are extracted at build time from
zeas2's official Kirikiroid2 1.3.9 release (https://github.com/zeas2/Kirikiroid2/releases/tag/1.3.9); their
sources (`cocos/kr2`) are part of the Kirikiroid2 repository.
