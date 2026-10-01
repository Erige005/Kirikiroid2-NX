// Portable decoder for KiriKiri AlphaMovie (.amv, magic "AJPM") files.
// [K2S] Kirikiroid2-NX: reimplementation of the format read by T.Imoto's AlphaMovie.dll (kaede-software.com).
// C++11, depends only on zlib (inflate, for the lossless alpha mode).
//
// Output: 32-bit BGRA (byte order B,G,R,A in memory == 0xAARRGGBB little
// endian), PREMULTIPLIED ("add alpha") as stored in the file; the original
// plugin writes it unchanged into an ltAddAlpha layer.
#ifndef AMV_DECODER_H
#define AMV_DECODER_H

#include <stdint.h>
#include <stddef.h>
#include <vector>

class AmvDecoder {
public:
    AmvDecoder();
    // Parses the header and indexes all FRAM chunks. The data must stay valid
    // (it is not copied) for the lifetime of the decoder.
    bool open(const uint8_t* data, size_t size);
    void close();

    int width() const { return width_; }       // screen size from header
    int height() const { return height_; }
    int numFrames() const { return numFrames_; }
    int fpsRate() const { return fpsRate_; }   // fps = rate / scale
    int fpsScale() const { return fpsScale_; }
    bool jpegAlpha() const { return !zlibAlpha_; }

    // Decodes frame `index` into a full width() x height() BGRA canvas.
    // Pixels outside the frame's coded rectangle are set to 0 (transparent).
    // pitch is in bytes (may be negative for bottom-up buffers).
    bool decodeFrame(int index, uint8_t* bgra, int pitch);

    // Lower level: decodes only the coded rectangle of the frame, exactly like
    // the original plugin (which resizes the target layer to w x h and moves it
    // to (x,y)). On success *x,*y,*w,*h describe the rect (w,h multiples of 16,
    // may be 0 for a fully transparent frame) and rect holds w*h BGRA pixels.
    bool decodeFrameRect(int index, int* x, int* y, int* w, int* h,
                         std::vector<uint32_t>& rect);

    // Diagnostics for the last decoded frame: bits consumed / bits available.
    size_t lastBitsUsed() const { return lastUsed_; }
    size_t lastBitsAvail() const { return lastAvail_; }

private:
    struct Huff {
        // canonical tables (libjpeg style)
        int maxcode[18];
        int valptr[17];
        int mincode[17];
        uint8_t vals[256];
        // 9-bit lookahead: length (0 = not in table) and symbol
        uint8_t lookLen[512];
        uint8_t lookSym[512];
        void build(const uint8_t bits[17], const uint8_t* values);
    };
    struct Bits;
    struct FrameInfo { size_t offset; uint32_t size; };

    static int decodeSymbol(const Huff& h, Bits& br);
    static int decodeDC(const Huff& h, Bits& br, int& pred);
    static int decodeAC(const Huff& h, Bits& br, int16_t* blk);
    static void idctIslow(const int16_t* coef, const uint16_t* q, uint8_t* out);
    static int flatVal(int dc, int q0);
    inline uint32_t ycc(int y, int cb, int cr, int a) const;

    const uint8_t* data_;
    size_t size_;
    int width_, height_, numFrames_, fpsRate_, fpsScale_;
    bool zlibAlpha_;
    uint16_t qY_[64], qC_[64], qA_[64];
    std::vector<FrameInfo> frames_;
    size_t lastUsed_, lastAvail_;
    Huff dcY_, acY_, dcC_, acC_;
    int crR_[256], cbB_[256], crG_[256], cbG_[256];
};

#endif
