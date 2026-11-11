#include <stdint.h>

#include "game/file.h"
#include "game/game.h"
#include "workdir.h"

int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    (void) argc;
    (void) argv;
    mayhem_enter_workdir();
    if (!game_pre_init() || !game_init()) {
        abort();
    }
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    mayhem_write_file("fuzz.sav", data, size);
    game_file_load_saved_game("fuzz.sav");
    return 0;
}
