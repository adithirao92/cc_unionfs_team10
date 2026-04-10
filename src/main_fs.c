#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/fs_state.h"

/* External functions from other source files */
extern int resolve_path(const char *path, char *resolved_path);

extern int fs_open(const char *, struct fuse_file_info *);
extern int fs_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
extern int fs_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);

extern int fs_readdir(const char *, void *, fuse_fill_dir_t,
                      off_t, struct fuse_file_info *,
                      enum fuse_readdir_flags);

extern int fs_mkdir(const char *, mode_t);
extern int fs_rmdir(const char *);

/* GETATTR */
static int fs_getattr(const char *path, struct stat *stbuf,
                      struct fuse_file_info *fi)
{
    (void) fi;

    char resolved[1024];
    int res = resolve_path(path, resolved);

    if (res != 0)
        return res;

    if (lstat(resolved, stbuf) == -1)
        return -errno;

    return 0;
}

/* Placeholder unlink (P4 will implement) */
static int fs_unlink(const char *path)
{
    (void) path;
    return 0;
}

/* FUSE operations */
static struct fuse_operations fs_ops = {
    .getattr = fs_getattr,
    .open    = fs_open,
    .read    = fs_read,
    .write   = fs_write,
    .readdir = fs_readdir,
    .mkdir   = fs_mkdir,
    .rmdir   = fs_rmdir,
    .unlink  = fs_unlink,
};

/* MAIN */
int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr,
                "Usage: %s <lower_dir> <upper_dir> <mountpoint> [-f]\n",
                argv[0]);
        return 1;
    }

    struct unionfs_state *state = malloc(sizeof(struct unionfs_state));
    if (!state) {
        perror("malloc failed");
        return 1;
    }

    state->lower_dir = realpath(argv[1], NULL);
    state->upper_dir = realpath(argv[2], NULL);

    if (!state->lower_dir || !state->upper_dir) {
        fprintf(stderr, "Error resolving lower/upper directories\n");
        return 1;
    }

    /* Build argv for FUSE */
    char *fuse_argv[argc];
    int fuse_argc = 0;

    fuse_argv[fuse_argc++] = argv[0];   // program name
    fuse_argv[fuse_argc++] = argv[3];   // mountpoint

    for (int i = 4; i < argc; i++) {
        fuse_argv[fuse_argc++] = argv[i];
    }

    return fuse_main(fuse_argc, fuse_argv, &fs_ops, state);
}
