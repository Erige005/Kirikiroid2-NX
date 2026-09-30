
/* [K2S] bpg_decoder_get_gray_line(): present in the libbpg used by Kirikiroid2 1.3.9 (LoadBPG.cpp,
 * glmGrayscale mode) but not in upstream libbpg 0.9.8. Decodes one RGBA32 line (the decoder is started
 * with BPG_OUTPUT_FORMAT_RGBA32) and converts it with the engine's own grayscale formula
 * (tvpgl.cpp compose_grayscale: (b*19 + g*183 + r*54) >> 8), so BPG masks match every other format. */
int bpg_decoder_get_gray_line(BPGDecoderContext *s, void *gray_line)
{
    uint8_t *dst = gray_line;
    uint8_t *rgba;
    int i, ret;

    if ((unsigned)s->y >= s->h)
        return -1;
    rgba = av_malloc(s->w * 4);
    if (!rgba)
        return -1;
    ret = bpg_decoder_get_line(s, rgba);
    if (ret == 0) {
        for (i = 0; i < s->w; i++) {
            const uint8_t *p = rgba + i * 4;
            dst[i] = (uint8_t)(((int)p[2] * 19 + (int)p[1] * 183 + (int)p[0] * 54) >> 8);
        }
    }
    av_free(rgba);
    return ret;
}
