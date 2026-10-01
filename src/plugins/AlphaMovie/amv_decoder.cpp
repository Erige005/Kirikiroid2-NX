// [K2S] Portable decoder for KiriKiri AlphaMovie (.amv, "AJPM") files. See amv_decoder.h.
#include "amv_decoder.h"
#include <string.h>
#include <zlib.h>

namespace {

inline uint32_t rd32(const uint8_t* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
inline uint16_t rd16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
inline int asr(int v, int n) { return v >= 0 ? (v >> n) : ~((~v) >> n); }  // arithmetic shift right
inline uint8_t clamp255(int v) { return (uint8_t)(v < 0 ? 0 : (v > 255 ? 255 : v)); }

// Standard JPEG Annex K tables (as embedded in AlphaMovie.dll)
const uint8_t kDcLumBits[17] = {0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
const uint8_t kDcChrBits[17] = {0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
const uint8_t kDcVals[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const uint8_t kAcLumBits[17] = {0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d};
const uint8_t kAcLumVals[162] = {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa};
const uint8_t kAcChrBits[17] = {0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77};
const uint8_t kAcChrVals[162] = {
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa};

// zigzag -> natural order, padded so that corrupt run lengths stay in range
const uint8_t kNatural[64 + 16] = {
    0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63};

const uint32_t kMagicAJPM = 0x4d504a41;  // "AJPM"
const uint32_t kMagicFRAM = 0x4d415246;  // "FRAM"

}  // namespace

// MSB-first bit reader over a byte buffer; reads past the end yield zeros.
struct AmvDecoder::Bits {
    const uint8_t* p;
    size_t n;     // bytes
    size_t pos;   // bit position
    uint32_t peek(int cnt) const {  // cnt <= 25
        uint32_t v = 0;
        size_t byte = pos >> 3;
        for (int i = 0; i < 4; ++i) v = (v << 8) | (byte + i < n ? p[byte + i] : 0);
        v <<= (pos & 7);
        return v >> (32 - cnt);
    }
    void skip(int cnt) { pos += cnt; }
    int get(int cnt) {
        if (cnt == 0) return 0;
        uint32_t v = peek(cnt);
        pos += cnt;
        return (int)v;
    }
};

void AmvDecoder::Huff::build(const uint8_t bits[17], const uint8_t* values) {
    int huffsize[257], huffcode[257];
    int p = 0;
    for (int l = 1; l <= 16; ++l)
        for (int i = 0; i < bits[l]; ++i) huffsize[p++] = l;
    huffsize[p] = 0;
    int total = p;
    memcpy(vals, values, total);
    int code = 0, si = huffsize[0];
    p = 0;
    while (huffsize[p]) {
        while (huffsize[p] == si) huffcode[p++] = code++;
        code <<= 1;
        ++si;
    }
    p = 0;
    for (int l = 1; l <= 16; ++l) {
        if (bits[l]) {
            valptr[l] = p;
            mincode[l] = huffcode[p];
            p += bits[l];
            maxcode[l] = huffcode[p - 1];
        } else {
            maxcode[l] = -1;
        }
    }
    maxcode[17] = 0xFFFFF;
    memset(lookLen, 0, sizeof(lookLen));
    memset(lookSym, 0, sizeof(lookSym));
    for (int i = 0; i < total; ++i) {
        int l = huffsize[i];
        if (l > 9) break;
        int base = huffcode[i] << (9 - l);
        for (int j = 0; j < (1 << (9 - l)); ++j) {
            lookLen[base + j] = (uint8_t)l;
            lookSym[base + j] = vals[i];
        }
    }
}

int AmvDecoder::decodeSymbol(const Huff& h, Bits& br) {
    uint32_t look = br.peek(9);
    if (h.lookLen[look]) {
        br.skip(h.lookLen[look]);
        return h.lookSym[look];
    }
    int l = 9;
    int code = (int)look;
    br.skip(9);
    while (l <= 16 && code > h.maxcode[l]) {
        code = (code << 1) | br.get(1);
        ++l;
    }
    if (l > 16) return -1;  // invalid code
    return h.vals[h.valptr[l] + code - h.mincode[l]];
}

static inline int extendBits(int v, int s) { return v < (1 << (s - 1)) ? v - (1 << s) + 1 : v; }

int AmvDecoder::decodeDC(const Huff& h, Bits& br, int& pred) {
    int s = decodeSymbol(h, br);
    if (s < 0) return pred;
    if (s) pred += extendBits(br.get(s), s);
    return pred;
}

// Decodes AC coefficients into blk (natural order). Returns the zigzag index at
// which decoding stopped (1 means "DC only", 64 means full block).
int AmvDecoder::decodeAC(const Huff& h, Bits& br, int16_t* blk) {
    int k = 1;
    while (k < 64) {
        int sym = decodeSymbol(h, br);
        if (sym < 0) return 64;
        int r = sym >> 4, s = sym & 15;
        if (s) {
            k += r;
            blk[kNatural[k]] = (int16_t)extendBits(br.get(s), s);
        } else {
            if (r != 15) break;
            k += 15;
        }
        ++k;
    }
    return k;
}

// IJG jidctint.c "islow" (CONST_BITS=13, PASS1_BITS=2) incl. its dequantisation,
// exactly as used by the plugin's plain-C path. Output 8x8 bytes, stride 8.
void AmvDecoder::idctIslow(const int16_t* in, const uint16_t* q, uint8_t* out) {
    int ws[64];
    for (int c = 0; c < 8; ++c) {
        const int16_t* x = in + c;
        const uint16_t* qq = q + c;
        int* w = ws + c;
        if (!x[8] && !x[16] && !x[24] && !x[32] && !x[40] && !x[48] && !x[56]) {
            int dc = x[0] * qq[0] * 4;
            for (int r = 0; r < 8; ++r) w[r * 8] = dc;
            continue;
        }
        int z2 = x[16] * qq[16], z3 = x[48] * qq[48];
        int z1 = (z2 + z3) * 4433;
        int t2 = z1 + z3 * -15137;
        int t3 = z1 + z2 * 6270;
        z2 = x[0] * qq[0];
        z3 = x[32] * qq[32];
        int t0 = (z2 + z3) * 8192, t1 = (z2 - z3) * 8192;
        int t10 = t0 + t3, t13 = t0 - t3, t11 = t1 + t2, t12 = t1 - t2;
        int a0 = x[56] * qq[56], a1 = x[40] * qq[40], a2 = x[24] * qq[24], a3 = x[8] * qq[8];
        int zz1 = a0 + a3, zz2 = a1 + a2, zz3 = a0 + a2, zz4 = a1 + a3;
        int z5 = (zz3 + zz4) * 9633;
        a0 *= 2446; a1 *= 16819; a2 *= 25172; a3 *= 12299;
        zz1 *= -7373; zz2 *= -20995; zz3 = zz3 * -16069 + z5; zz4 = zz4 * -3196 + z5;
        a0 += zz1 + zz3; a1 += zz2 + zz4; a2 += zz2 + zz3; a3 += zz1 + zz4;
        w[0] = asr(t10 + a3 + 1024, 11);
        w[56] = asr(t10 - a3 + 1024, 11);
        w[8] = asr(t11 + a2 + 1024, 11);
        w[48] = asr(t11 - a2 + 1024, 11);
        w[16] = asr(t12 + a1 + 1024, 11);
        w[40] = asr(t12 - a1 + 1024, 11);
        w[24] = asr(t13 + a0 + 1024, 11);
        w[32] = asr(t13 - a0 + 1024, 11);
    }
    for (int r = 0; r < 8; ++r) {
        const int* w = ws + r * 8;
        uint8_t* o = out + r * 8;
        if (!w[1] && !w[2] && !w[3] && !w[4] && !w[5] && !w[6] && !w[7]) {
            uint8_t v = clamp255(asr(w[0] + 16, 5) + 128);
            for (int i = 0; i < 8; ++i) o[i] = v;
            continue;
        }
        int z1 = (w[2] + w[6]) * 4433;
        int t2 = z1 + w[6] * -15137;
        int t3 = z1 + w[2] * 6270;
        int t0 = (w[0] + w[4]) * 8192, t1 = (w[0] - w[4]) * 8192;
        int t10 = t0 + t3, t13 = t0 - t3, t11 = t1 + t2, t12 = t1 - t2;
        int a0 = w[7], a1 = w[5], a2 = w[3], a3 = w[1];
        int zz1 = a0 + a3, zz2 = a1 + a2, zz3 = a0 + a2, zz4 = a1 + a3;
        int z5 = (zz3 + zz4) * 9633;
        a0 *= 2446; a1 *= 16819; a2 *= 25172; a3 *= 12299;
        zz1 *= -7373; zz2 *= -20995; zz3 = zz3 * -16069 + z5; zz4 = zz4 * -3196 + z5;
        a0 += zz1 + zz3; a1 += zz2 + zz4; a2 += zz2 + zz3; a3 += zz1 + zz4;
        const int R = 0x20000;
        o[0] = clamp255(asr(t10 + a3 + R, 18) + 128);
        o[7] = clamp255(asr(t10 - a3 + R, 18) + 128);
        o[1] = clamp255(asr(t11 + a2 + R, 18) + 128);
        o[6] = clamp255(asr(t11 - a2 + R, 18) + 128);
        o[2] = clamp255(asr(t12 + a1 + R, 18) + 128);
        o[5] = clamp255(asr(t12 - a1 + R, 18) + 128);
        o[3] = clamp255(asr(t13 + a0 + R, 18) + 128);
        o[4] = clamp255(asr(t13 - a0 + R, 18) + 128);
    }
}

// Value of a DC-only block on the plugin's "flat macroblock" fast path
// (note: no rounding, unlike the IDCT's DC-only shortcut).
int AmvDecoder::flatVal(int dc, int q0) { return clamp255(asr(dc * q0, 3) + 128); }

inline uint32_t AmvDecoder::ycc(int y, int cb, int cr, int a) const {
    int r = clamp255(y + crR_[cr]);
    int g = clamp255(y + asr(cbG_[cb] + crG_[cr], 16));
    int b = clamp255(y + cbB_[cb]);
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

AmvDecoder::AmvDecoder()
    : data_(0), size_(0), width_(0), height_(0), numFrames_(0), fpsRate_(0), fpsScale_(0), zlibAlpha_(false), lastUsed_(0), lastAvail_(0) {
    dcY_.build(kDcLumBits, kDcVals);
    dcC_.build(kDcChrBits, kDcVals);
    acY_.build(kAcLumBits, kAcLumVals);
    acC_.build(kAcChrBits, kAcChrVals);
    // libjpeg jdcolor.c build_ycc_rgb_table (SCALEBITS 16)
    for (int i = 0; i < 256; ++i) {
        int x = i - 128;
        crR_[i] = asr(91881 * x + 32768, 16);
        cbB_[i] = asr(116130 * x + 32768, 16);
        crG_[i] = -46802 * x;
        cbG_[i] = -22554 * x + 32768;
    }
}

void AmvDecoder::close() {
    data_ = 0;
    size_ = 0;
    frames_.clear();
    width_ = height_ = numFrames_ = 0;
}

bool AmvDecoder::open(const uint8_t* d, size_t n) {
    close();
    if (!d || n < 40) return false;
    if (rd32(d) != kMagicAJPM) return false;
    if (rd32(d + 8) != 0) return false;  // revision must be 0
    uint32_t dataOff = rd32(d + 12);
    uint32_t qsize = dataOff - 40;
    if (qsize != 0x80 && qsize != 0xC0) return false;
    if (dataOff > n) return false;
    uint32_t nf = rd32(d + 20);
    uint32_t scale = rd32(d + 24), rate = rd32(d + 28);
    uint16_t w = rd16(d + 32), h = rd16(d + 34);
    uint32_t attr = rd32(d + 36);
    if (!nf || !scale || !rate || !w || !h) return false;
    if (attr & 1) zlibAlpha_ = false;          // lossy (JPEG) alpha
    else if (attr & 2) zlibAlpha_ = true;      // lossless (zlib) alpha
    else return false;
    if (zlibAlpha_ != (qsize == 0x80)) return false;
    const uint8_t* q = d + 40;
    for (int i = 0; i < 64; ++i) {
        qY_[i] = q[i];
        qC_[i] = q[64 + i];
        qA_[i] = (qsize == 0xC0) ? q[128 + i] : 0;
    }
    // index the frame chunks: "FRAM", u32 size, u32 frameIndex, ... ; next = off + 8 + size
    frames_.assign(nf, FrameInfo());
    std::vector<bool> seen(nf, false);
    size_t off = dataOff;
    while (off + 12 <= n) {
        if (rd32(d + off) != kMagicFRAM) break;
        uint32_t sz = rd32(d + off + 4);
        uint32_t idx = rd32(d + off + 8);
        if (sz < 12 || off + 8 + (size_t)sz > n) break;
        if (idx < nf && !seen[idx]) {
            frames_[idx].offset = off;
            frames_[idx].size = sz;
            seen[idx] = true;
        }
        off += 8 + (size_t)sz;
    }
    for (uint32_t i = 0; i < nf; ++i)
        if (!seen[i]) { frames_.clear(); return false; }
    data_ = d;
    size_ = n;
    width_ = w;
    height_ = h;
    numFrames_ = (int)nf;
    fpsRate_ = (int)rate;
    fpsScale_ = (int)scale;
    return true;
}

bool AmvDecoder::decodeFrameRect(int index, int* ox, int* oy, int* ow, int* oh, std::vector<uint32_t>& rect) {
    if (!data_ || index < 0 || index >= numFrames_) return false;
    const FrameInfo& fi = frames_[index];
    const uint8_t* c = data_ + fi.offset;
    const uint32_t chunk = fi.size;  // bytes after the size field
    // common: +8 frameIndex, +12 s16 x, +14 s16 y, +16 u16 w, +18 u16 h
    int x = (int16_t)rd16(c + 12), y = (int16_t)rd16(c + 14);
    int w = rd16(c + 16), h = rd16(c + 18);
    if ((w & 15) || (h & 15)) return false;
    *ox = x; *oy = y; *ow = w; *oh = h;
    rect.assign((size_t)w * h, 0);
    if (!w || !h) return true;

    const uint8_t* bits;
    size_t nbits;
    std::vector<uint8_t> alpha;
    if (zlibAlpha_) {
        if (chunk < 16) return false;
        uint32_t zsize = rd32(c + 20);
        if ((uint64_t)zsize + 16 > chunk) return false;
        alpha.resize((size_t)w * h);
        uLongf dlen = (uLongf)alpha.size();
        int zr = uncompress(&alpha[0], &dlen, c + 24, zsize);
        if (zr != Z_OK) return false;
        bits = c + 24 + zsize;
        nbits = chunk - 16 - zsize;
    } else {
        bits = c + 20;
        nbits = chunk - 12;
    }
    Bits br = {bits, nbits, 0};
    int predY = 0, predC = 0;  // DC predictors: one per Huffman table (Y+A share, Cb+Cr share)
    const int mbw = w >> 4, mbh = h >> 4;
    const int nblk = zlibAlpha_ ? 6 : 10;
    int16_t coef[10][64];
    uint8_t pix[10][64];
    int endk[10];
    // block order in stream: Cb, Cr, Y0, Y1, Y2, Y3, [A0, A1, A2, A3]
    for (int my = 0; my < mbh; ++my) {
        for (int mx = 0; mx < mbw; ++mx) {
            memset(coef, 0, sizeof(coef));
            int sum = 0;
            for (int b = 0; b < nblk; ++b) {
                bool chroma = b < 2;
                coef[b][0] = (int16_t)decodeDC(chroma ? dcC_ : dcY_, br, chroma ? predC : predY);
                endk[b] = decodeAC(chroma ? acC_ : acY_, br, coef[b]);
                sum += endk[b];
            }
            uint32_t* dst = &rect[(size_t)(my * 16) * w + mx * 16];
            const uint8_t* ap = zlibAlpha_ ? &alpha[(size_t)(my * 16) * w + mx * 16] : 0;
            if (sum == nblk) {
                // all blocks DC-only: flat fast path
                int cb = flatVal(coef[0][0], qC_[0]);
                int cr = flatVal(coef[1][0], qC_[0]);
                for (int qd = 0; qd < 4; ++qd) {
                    int yv = flatVal(coef[2 + qd][0], qY_[0]);
                    int av = zlibAlpha_ ? 0 : flatVal(coef[6 + qd][0], qA_[0]);
                    uint32_t rgb = ycc(yv, cb, cr, 0);
                    int bx = (qd & 1) * 8, by = (qd >> 1) * 8;
                    for (int yy = 0; yy < 8; ++yy) {
                        uint32_t* row = dst + (size_t)(by + yy) * w + bx;
                        const uint8_t* arow = ap ? ap + (size_t)(by + yy) * w + bx : 0;
                        for (int xx = 0; xx < 8; ++xx)
                            row[xx] = rgb | ((uint32_t)(arow ? arow[xx] : av) << 24);
                    }
                }
            } else {
                idctIslow(coef[0], qC_, pix[0]);
                idctIslow(coef[1], qC_, pix[1]);
                for (int b = 2; b < 6; ++b) idctIslow(coef[b], qY_, pix[b]);
                if (!zlibAlpha_)
                    for (int b = 6; b < 10; ++b) idctIslow(coef[b], qA_, pix[b]);
                for (int yy = 0; yy < 16; ++yy) {
                    uint32_t* row = dst + (size_t)yy * w;
                    const uint8_t* arow = ap ? ap + (size_t)yy * w : 0;
                    for (int xx = 0; xx < 16; ++xx) {
                        int blk = (yy >> 3) * 2 + (xx >> 3);
                        int in = (yy & 7) * 8 + (xx & 7);
                        int ci = (yy >> 1) * 8 + (xx >> 1);
                        int a = arow ? arow[xx] : pix[6 + blk][in];
                        row[xx] = ycc(pix[2 + blk][in], pix[0][ci], pix[1][ci], a);
                    }
                }
            }
        }
    }
    lastUsed_ = br.pos;
    lastAvail_ = nbits * 8;
    return true;
}

bool AmvDecoder::decodeFrame(int index, uint8_t* bgra, int pitch) {
    int x, y, w, h;
    std::vector<uint32_t> rect;
    if (!decodeFrameRect(index, &x, &y, &w, &h, rect)) return false;
    for (int r = 0; r < height_; ++r) memset(bgra + (ptrdiff_t)r * pitch, 0, (size_t)width_ * 4);
    for (int r = 0; r < h; ++r) {
        int dy = y + r;
        if (dy < 0 || dy >= height_) continue;
        int x0 = x < 0 ? -x : 0;
        int x1 = (x + w > width_) ? width_ - x : w;
        if (x1 <= x0) continue;
        uint8_t* drow = bgra + (ptrdiff_t)dy * pitch + (size_t)(x + x0) * 4;
        const uint32_t* srow = &rect[(size_t)r * w + x0];
        for (int i = 0; i < x1 - x0; ++i) {
            uint32_t p = srow[i];
            drow[i * 4 + 0] = (uint8_t)p;
            drow[i * 4 + 1] = (uint8_t)(p >> 8);
            drow[i * 4 + 2] = (uint8_t)(p >> 16);
            drow[i * 4 + 3] = (uint8_t)(p >> 24);
        }
    }
    return true;
}
