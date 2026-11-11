#include <stdint.h>
#include <stdio.h>

#include "core/smacker.h"

/* smacker_open() takes ownership of the FILE and closes it. */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size == 0) {
        return -1;
    }
    FILE *fp = fmemopen((void *) data, size, "rb");
    if (!fp) {
        return -1;
    }
    smacker s = smacker_open(fp);
    if (!s) {
        return 0;
    }
    int frames, usf, width, height, y_scale;
    smacker_get_frames_info(s, &frames, &usf);
    smacker_get_video_info(s, &width, &height, &y_scale);
    for (int track = 0; track < 7; track++) {
        int enabled, channels, bitdepth, rate;
        smacker_get_audio_info(s, track, &enabled, &channels, &bitdepth, &rate);
    }

    smacker_frame_status status = smacker_first_frame(s);
    while (status == SMACKER_FRAME_OK) {
        const color_t *palette = smacker_get_frame_palette(s);
        const uint8_t *video = smacker_get_frame_video(s);
        volatile uint8_t sink = (uint8_t) palette[0] + (width > 0 && height > 0 ? video[0] : 0);
        (void) sink;
        for (int track = 0; track < 7; track++) {
            int len = smacker_get_frame_audio_size(s, track);
            const uint8_t *audio = smacker_get_frame_audio(s, track);
            if (audio && len > 0) {
                sink = audio[len - 1];
            }
        }
        status = smacker_next_frame(s);
    }
    smacker_close(s);
    return 0;
}
