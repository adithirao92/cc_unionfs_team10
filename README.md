# UnionFS - Layered File System using FUSE

##  Overview

This project implements a simplified **Union File System (UnionFS)** using **FUSE (Filesystem in Userspace)**.
It combines two directories — a **lower (read-only)** layer and an **upper (read-write)** layer — into a single unified mount point.

The system supports:

* File reading from both layers
* Copy-on-Write (CoW) for modifications
* Directory merging
* Whiteout mechanism for deletions

---

##  Concepts Implemented

###  1. Layered Filesystem

* **Lower directory** → base (read-only)
* **Upper directory** → writable layer
* **Mount point** → merged view

---

###  2. Copy-on-Write (CoW)

* If a file exists only in lower layer and is modified:

  * It is copied to upper layer
  * Changes are applied there

---

###  3. Directory Merging

* Contents of both directories are merged
* Upper layer takes priority over lower layer
* Duplicate entries are avoided

---

###  4. Whiteout Mechanism

* When a file from lower layer is deleted:

  * A `.wh.<filename>` file is created in upper layer
  * This hides the file from the merged view

---

##  Project Structure

```
cc_unionfs_team10/
│
├── src/
│   ├── main_fs.c        # FUSE setup and main logic
│   ├── path_utils.c    # Path resolution logic (P1)
│   ├── file_ops.c      # File operations + CoW (P2)
│   ├── dir_ops.c       # Directory operations (P3)
│   ├── delete_ops.c    # Whiteout logic (P4)
│
├── include/
│   └── fs_state.h      # Shared state structure
│
├── lower/              # Lower (read-only) directory
├── upper/              # Upper (read-write) directory
├── mnt/                # Mount point
│
├── mini_unionfs        # Compiled binary
└── README.md
```

---

##  Installation & Compilation

###  Prerequisites

* Linux (Ubuntu recommended)
* FUSE3 installed

```bash
sudo apt update
sudo apt install fuse3 libfuse3-dev
```

---

###  Compile

```bash
gcc -Wall src/*.c `pkg-config fuse3 --cflags --libs` -o mini_unionfs
```

---

##  Running the Filesystem

### 1. Create directories

```bash
mkdir lower upper mnt
```

### 2. Add sample file

```bash
echo "hello" > lower/test.txt
```

### 3. Mount filesystem

```bash
./mini_unionfs lower upper mnt -f
```

---

##  Testing

###  View merged files

```bash
ls mnt
```

---

###  Modify file (Copy-on-Write)

```bash
echo "new content" >> mnt/test.txt
```

 File will be copied to upper layer

---

###  Delete file (Whiteout)

```bash
rm mnt/test.txt
```

 Check upper layer:

```bash
ls upper
```

Expected:

```
.wh.test.txt
```

---

##  Unmount

```bash
fusermount -u mnt
```

If stuck:

```bash
fusermount -uz mnt
```

---

##  Team Contributions

| Member | Contribution                                       |
| ------ | -------------------------------------------------- |
| CS027  | Path resolution (`resolve_path`)                   |
| CS012  | File operations + Copy-on-Write                    |
| CS013  | Directory operations (`readdir`, `mkdir`, `rmdir`) |
| CS017  | Whiteout deletion logic (`unlink`)                 |

---

##  Limitations

* No advanced permission handling
* No symbolic link support
* Whiteout logic is basic (no nested directories handling)
* Not production-ready (educational implementation)

---

##  Conclusion

This project demonstrates the core concepts of modern layered filesystems like Docker's overlay filesystem.
It provides hands-on experience with:

* Filesystem design
* FUSE programming
* Linux file handling

---
