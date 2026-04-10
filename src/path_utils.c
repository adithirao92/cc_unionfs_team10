#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "../include/fs_state.h"

#define STATE ((struct unionfs_state *) fuse_get_context()->private_data)

// Resolve file path across layers
int resolve_path(const char *path, char *resolved_path) {
    struct unionfs_state *state = STATE;

    char upper_path[1024];
    char lower_path[1024];
    char whiteout_path[1024];

    snprintf(upper_path, sizeof(upper_path), "%s%s", state->upper_dir, path);
    snprintf(lower_path, sizeof(lower_path), "%s%s", state->lower_dir, path);

    // extract filename
    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;

    snprintf(whiteout_path, sizeof(whiteout_path),
             "%s/.wh.%s", state->upper_dir, name);

    // 1. check whiteout
    if (access(whiteout_path, F_OK) == 0) {
        return -ENOENT;
    }

    // 2. check upper layer
    if (access(upper_path, F_OK) == 0) {
        strcpy(resolved_path, upper_path);
        return 0;
    }

    // 3. check lower layer
    if (access(lower_path, F_OK) == 0) {
        strcpy(resolved_path, lower_path);
        return 0;
    }

    return -ENOENT;
}
