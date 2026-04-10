#ifndef FS_STATE_H
#define FS_STATE_H

struct unionfs_state {
    char *lower_dir;
    char *upper_dir;
};

#endif
