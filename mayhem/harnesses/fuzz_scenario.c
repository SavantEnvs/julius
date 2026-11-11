#include <stdint.h>

#include "game/file.h"
#include "game/file_editor.h"
#include "game/game.h"
#include "scenario/property.h"
#include "workdir.h"

/* game_file_io_read_scenario() (src/game/file_io.c) fread()s a fixed sum of pieces and fails on any
 * shorter file, so a mutation that changes the length would never reach the parsers. Each input is
 * therefore laid over a default scenario written once by upstream's own editor code (the same code the
 * seed comes from): bytes past the end of the input keep their default value, bytes past the file size
 * are dropped. The empty input is the unmodified default scenario. */
enum { SCENARIO_FILE_SIZE = 211692 };

static uint8_t base[SCENARIO_FILE_SIZE];

int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    (void) argc;
    (void) argv;
    mayhem_enter_workdir();
    if (!game_pre_init() || !game_init()) {
        abort();
    }
    game_file_editor_create_scenario(2);
    if (!game_file_editor_write_scenario("base.map")) {
        abort();
    }
    FILE *fp = fopen("base.map", "rb");
    if (!fp || fread(base, 1, sizeof(base), fp) != sizeof(base)) {
        abort();
    }
    fclose(fp);
    remove("base.map");
    return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    static uint8_t file[SCENARIO_FILE_SIZE];
    memcpy(file, base, sizeof(file));
    memcpy(file, data, size < sizeof(file) ? size : sizeof(file));
    mayhem_write_file("fuzz.map", file, sizeof(file));
    if (!game_file_load_scenario_data("fuzz.map")) {
        return -1;
    }
    scenario_set_custom(1);
    game_file_start_scenario("fuzz.map");
    return 0;
}
