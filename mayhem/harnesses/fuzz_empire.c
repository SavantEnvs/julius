#include <stdint.h>

#include "empire/empire.h"
#include "empire/object.h"
#include "workdir.h"

/* One empire slot of c32.emp (custom-scenario empire file): a 4 byte header (initial scroll x/y)
 * followed by the 12800 byte object table that empire_load() reads at offset 1280 + 12800 * id. */
enum { SCROLL_SIZE = 4, EMPIRE_HEADER_SIZE = 1280, EMPIRE_DATA_SIZE = 12800 };

int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    (void) argc;
    (void) argv;
    mayhem_enter_workdir();
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    static uint8_t file[EMPIRE_HEADER_SIZE + EMPIRE_DATA_SIZE];
    memset(file, 0, sizeof(file));
    size_t used = size < SCROLL_SIZE + EMPIRE_DATA_SIZE ? size : SCROLL_SIZE + EMPIRE_DATA_SIZE;
    size_t head = used < SCROLL_SIZE ? used : SCROLL_SIZE;
    memcpy(file, data, head);
    memcpy(file + EMPIRE_HEADER_SIZE, data + head, used - head);
    mayhem_write_file("c32.emp", file, sizeof(file));

    empire_load_editor(0, 1136, 640);
    empire_init_scenario();
    return 0;
}
