#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include "../include/fs_state.h"

int resolve_path(const char *path, char *resolved_path);

// READDIR (MOST IMPORTANT)
int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi,
               enum fuse_readdir_flags flags) {
printf("READDIR CALLED for %s\n", path);
    struct fuse_context *ctx = fuse_get_context();
    struct unionfs_state *state = ctx->private_data;

    char upper_path[1024];
    char lower_path[1024];

    snprintf(upper_path, sizeof(upper_path), "%s%s", state->upper_dir, path);
    snprintf(lower_path, sizeof(lower_path), "%s%s", state->lower_dir, path);

    DIR *dp;
    struct dirent *entry;

    // Add default entries
    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    // Track duplicates (simple approach)
    char seen[100][256];
    int count = 0;

    // 1. Read upper_dir first (priority)
    dp = opendir(upper_path);
    if (dp) {
        while ((entry = readdir(dp)) != NULL) {
            filler(buf, entry->d_name, NULL, 0, 0);
            strcpy(seen[count++], entry->d_name);
        }
        closedir(dp);
    }

    // 2. Read lower_dir (avoid duplicates)
    dp = opendir(lower_path);
    if (dp) {
        while ((entry = readdir(dp)) != NULL) {

            int duplicate = 0;
            for (int i = 0; i < count; i++) {
                if (strcmp(seen[i], entry->d_name) == 0) {
                    duplicate = 1;
                    break;
                }
            }

            if (!duplicate) {
                filler(buf, entry->d_name, NULL, 0, 0);
            }
        }
        closedir(dp);
    }

    return 0;
}

// MKDIR (upper layer only)
int fs_mkdir(const char *path, mode_t mode) {

    struct fuse_context *ctx = fuse_get_context();
    struct unionfs_state *state = ctx->private_data;

    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", state->upper_dir, path);

    if (mkdir(full_path, mode) == -1)
        return -errno;

    return 0;
}

// RMDIR (upper layer only)
int fs_rmdir(const char *path) {

    struct fuse_context *ctx = fuse_get_context();
    struct unionfs_state *state = ctx->private_data;

    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", state->upper_dir, path);

    if (rmdir(full_path) == -1)
        return -errno;

    return 0;
}
