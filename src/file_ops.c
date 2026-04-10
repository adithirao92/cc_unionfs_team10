#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int resolve_path(const char *path, char *resolved_path);

// Copy file from lower_dir → upper_dir
static int copy_to_upper(const char *src, const char *dest) {
    int in = open(src, O_RDONLY);
    if (in < 0) return -errno;

    int out = open(dest, O_WRONLY | O_CREAT, 0644);
    if (out < 0) return -errno;

    char buf[4096];
    ssize_t n;

    while ((n = read(in, buf, sizeof(buf))) > 0) {
        write(out, buf, n);
    }

    close(in);
    close(out);
    return 0;
}

// OPEN with CoW
int fs_open(const char *path, struct fuse_file_info *fi) {
    char resolved[1024];
    int res = resolve_path(path, resolved);

    if (res != 0)
        return res;

    // If writing → ensure file is in upper layer
    if ((fi->flags & O_WRONLY) || (fi->flags & O_RDWR)) {
        // If file is from lower layer → copy needed
        if (strstr(resolved, "lower")) {
            char upper_path[1024];
            snprintf(upper_path, sizeof(upper_path), "./upper%s", path);

            copy_to_upper(resolved, upper_path);
            strcpy(resolved, upper_path);
        }
    }

    int fd = open(resolved, fi->flags);
    if (fd < 0)
        return -errno;

    close(fd);
    return 0;
}

// READ
int fs_read(const char *path, char *buf, size_t size,
            off_t offset, struct fuse_file_info *fi) {

    char resolved[1024];
    int res = resolve_path(path, resolved);
    if (res != 0)
        return res;

    int fd = open(resolved, O_RDONLY);
    if (fd < 0)
        return -errno;

    lseek(fd, offset, SEEK_SET);
    int bytes = read(fd, buf, size);

    close(fd);
    return bytes;
}

// WRITE
int fs_write(const char *path, const char *buf, size_t size,
             off_t offset, struct fuse_file_info *fi) {

    char resolved[1024];
    int res = resolve_path(path, resolved);
    if (res != 0)
        return res;

    int fd = open(resolved, O_WRONLY);
    if (fd < 0)
        return -errno;

    lseek(fd, offset, SEEK_SET);
    int bytes = write(fd, buf, size);

    close(fd);
    return bytes;
}
