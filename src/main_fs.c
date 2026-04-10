#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

int resolve_path(const char *path, char *resolved_path);

// Basic getattr using resolved path
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

// Placeholder functions (others will implement)
static int fs_open(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int fs_read(const char *path, char *buf, size_t size,
                   off_t offset, struct fuse_file_info *fi) {
    return 0;
}

static int fs_write(const char *path, const char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi) {
    return size;
}

static int fs_unlink(const char *path) {
    return 0;
}

static struct fuse_operations fs_ops = {
    .getattr = fs_getattr,
    .open    = fs_open,
    .read    = fs_read,
    .write   = fs_write,
    .unlink  = fs_unlink,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &fs_ops, NULL);
}
