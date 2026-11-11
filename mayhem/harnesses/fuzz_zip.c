#include <stdint.h>
#include <stdlib.h>

#include "core/zip.h"

/* Input: 2 byte little-endian size of the output buffer, then the compressed stream. */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 3) {
        return -1;
    }
    int out_len = 1 + (data[0] | (data[1] << 8));
    uint8_t *out = malloc(out_len);
    if (!out) {
        return -1;
    }
    zip_decompress(data + 2, (int) (size - 2), out, &out_len);
    free(out);
    return 0;
}
