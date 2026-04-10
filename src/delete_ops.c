#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "../include/fs_state.h"

int resolve_path(const char *path, char *resolved_path);

// UNLINK (WHITEOUT)
int fs_unlink(const char *path) {

    struct fuse_context *ctx = fuse_get_context();
    struct unionfs_state *state = ctx->private_data;

    char upper_path[4096];
    char lower_path[4096];
    char whiteout_path[4096];

    snprintf(upper_path, sizeof(upper_path), "%s%s", state->upper_dir, path);
    snprintf(lower_path, sizeof(lower_path), "%s%s", state->lower_dir, path);

    printf("UNLINK: path=%s\n", path);
    printf("UNLINK: upper_path=%s\n", upper_path);
    printf("UNLINK: lower_path=%s\n", lower_path);

    // Case 1: file exists in upper → just delete it directly
    if (access(upper_path, F_OK) == 0) {
        printf("UNLINK: deleting from upper\n");
        if (unlink(upper_path) == -1)
            return -errno;
        return 0;
    }

    // Case 2: file exists only in lower → create whiteout in upper
    if (access(lower_path, F_OK) == 0) {

        // Get the parent directory of the file (relative)
        char parent_dir[1024];
        strncpy(parent_dir, path, sizeof(parent_dir));
        char *slash = strrchr(parent_dir, '/');

        // Extract just the filename
        const char *name = strrchr(path, '/');
        name = name ? name + 1 : path;

        if (slash && slash != parent_dir) {
            // File is in a subdirectory e.g. /subdir/test.txt
            *slash = '\0';
            snprintf(whiteout_path, sizeof(whiteout_path),
                     "%s%s/.wh.%s", state->upper_dir, parent_dir, name);
        } else {
            // File is at root level e.g. /test.txt
            snprintf(whiteout_path, sizeof(whiteout_path),
                     "%s/.wh.%s", state->upper_dir, name);
        }

        printf("UNLINK: creating whiteout at %s\n", whiteout_path);

        int fd = open(whiteout_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            printf("UNLINK: fopen failed: %s\n", strerror(errno));
            return -errno;
        }
        close(fd);

        printf("UNLINK: whiteout created successfully\n");
        return 0;
    }

    printf("UNLINK: file not found anywhere\n");
    return -ENOENT;
}
