#ifndef MAYHEM_WORKDIR_H
#define MAYHEM_WORKDIR_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Julius loads everything by relative file name from the working directory, so each harness runs in
 * its own scratch directory under $TMPDIR (cwd of a Mayhem run is writable, the image dir is not). */
static char mayhem_workdir[4096];

static void mayhem_remove_workdir(void)
{
    DIR *d = opendir(mayhem_workdir);
    if (d) {
        struct dirent *e;
        char path[8192];
        while ((e = readdir(d)) != NULL) {
            snprintf(path, sizeof(path), "%s/%s", mayhem_workdir, e->d_name);
            unlink(path);
        }
        closedir(d);
    }
    if (chdir("/") == 0) {
        rmdir(mayhem_workdir);
    }
}

static void mayhem_enter_workdir(void)
{
    const char *tmp = getenv("TMPDIR");
    snprintf(mayhem_workdir, sizeof(mayhem_workdir), "%s/julius-fuzz-XXXXXX", tmp && *tmp ? tmp : "/tmp");
    if (!mkdtemp(mayhem_workdir) || chdir(mayhem_workdir) != 0) {
        abort();
    }
    atexit(mayhem_remove_workdir);
}

static void mayhem_write_file(const char *name, const uint8_t *data, size_t size)
{
    FILE *fp = fopen(name, "wb");
    if (!fp) {
        abort();
    }
    if (size && fwrite(data, 1, size, fp) != size) {
        abort();
    }
    fclose(fp);
}

#endif
