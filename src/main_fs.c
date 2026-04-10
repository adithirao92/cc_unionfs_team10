#define FUSE_USE_VERSION 31
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "../include/fs_state.h"

// Function declarations
int resolve_path(const char *path, char *resolved_path);
int fs_open(const char *, struct fuse_file_info *);
int fs_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
int fs_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);

// Global state pointer
struct unionfs_state *global_state;

// GETATTR
static int fs_getattr(const char *path, struct stat *stbuf,
                      struct fuse_file_info *fi) {

    char resolved[1024];
    int res = resolve_path(path, resolved);

    if (res != 0)
        return res;

    if (lstat(resolved, stbuf) == -1)
        return -errno;

    return 0;
}

// UNLINK placeholder (P4 will implement)
static int fs_unlink(const char *path) {
    return 0;
}

// FUSE operations struct
static struct fuse_operations fs_ops = {
    .getattr = fs_getattr,
    .open    = fs_open,
    .read    = fs_read,
    .write   = fs_write,
    .unlink  = fs_unlink,
};

// MAIN FUNCTION
int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <lower_dir> <upper_dir> <mountpoint>\n", argv[0]);
        return 1;
    }

    // Allocate memory for state
    struct unionfs_state *state = malloc(sizeof(struct unionfs_state));
    if (!state) {
        perror("malloc failed");
        return 1;
    }

    state->lower_dir = argv[1];
    state->upper_dir = argv[2];

    // Save globally (so other files can access if needed)
    global_state = state;

    // Shift arguments for FUSE
    // argv[1] = mountpoint
    argv[1] = argv[3];

    // Adjust argc
    argc = 2;

    return fuse_main(argc, argv, &fs_ops, state);
}
