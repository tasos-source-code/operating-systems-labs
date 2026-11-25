# Project 4: Deduplicating File System (FUSE)

This project is a user-space file system built with **FUSE**. It stores file contents
with **block-level deduplication**: identical 4 KiB blocks are kept only once on disk,
however many files contain them. The code started from Joseph J. Pfeiffer's
*Big Brother File System* (BBFS), a FUSE pass-through file system.

## Architecture

The file system keeps its data in a backing directory (`rootdir`) with this layout:

```
rootdir/
├── hashes        # persistent hash table: { SHA-1 hash, reference count } entries
├── metadata/     # mirror of the user's directory tree
│   └── <file>    # for each file: the number of blocks and the ordered list of block hashes
└── blocks/
    └── <sha1>    # one file per unique 4 KiB block, named by its hash
```

- **Write:** the data is split into 4 KiB blocks and each block is hashed with SHA-1.
  If the hash is already in the table, its reference count goes up by one. Otherwise
  the block is stored in `blocks/` with a count of 1. The file's metadata entry
  records the sequence of hashes.
- **Read:** the file is rebuilt from its list of hashes by reading the block files.
- **Overwrite / truncate / unlink:** reference counts go down, and a block file is
  deleted when its count reaches zero.
- **Directories:** they are mirrored directly under `metadata/`, so all directory
  operations (`mkdir`, `rmdir`, `readdir`, …) are redirected there.

## Design decisions

- **Hash table:** kept in memory as a growable array and saved to `rootdir/hashes`
  only when blocks are added or removed.
- **Persistent:** all metadata lives in ordinary files, so the file system can be
  unmounted and mounted again without losing data.
- **No fixed limits:** any number of files of any size (in 4 KiB multiples). It is
  not limited to the 10 files × 64 KiB of the assignment.
- **Extra feature:** `truncate()` works for both shrinking and growing a file.

### Implemented functions

- **FUSE callbacks:** `bb_open`, `bb_read`, `bb_write`, `bb_truncate`, `bb_getattr`,
  `bb_mknod`, `bb_unlink`, `bb_init`, `bb_destroy`
- **Helpers:** `ensure_hash_capacity`, `add_hash`, `update_hashes`, `get_hashes`,
  `check_hash`, `bb_fullpath`, `get_block_path`, `write_block`, `delete_block`

## Testing

`experiments/testbench.c` tests the following on the mounted file system:

- creating directories
- writing files of different sizes with partly identical content
- reading the files back and checking every byte
- overwriting single blocks and checking the result
- `truncate()` to smaller and larger sizes, including checking the zero-filled region
- deleting files, after which no blocks remain in `blocks/`

Normal shell tools (`ls`, `cd`, `cp`, `cat`, `echo`, `diff`) also work on the mounted
file system.

## Build & run

Requirements: `libfuse-dev` (FUSE 2.x), `libssl-dev`, `pkg-config`.

```bash
cd project/filesystem
make            # builds ./myfs
make mount      # creates fs/rootdir/{blocks,metadata} and fs/mountdir, then mounts
make test       # builds and runs the testbench against the mount point
make unmount
make clean      # unmounts and deletes the binaries and fs/
```

## Files

```
Project_4/
├── project/
│   ├── filesystem/
│   │   ├── myfile_system.c        # FUSE file system (based on BBFS)
│   │   ├── myfs.h                 # block size, hash table types
│   │   ├── log.c, log.h, params.h # BBFS logging / private data
│   │   └── Makefile
│   ├── experiments/testbench.c    # functional testbench
│   └── report/report_lab4.pdf     # project report (Greek)
└── Project_04.pdf                 # assignment (Greek)
```

## License

The file system code is based on BBFS © 2012 Joseph J. Pfeiffer, Jr., and is
distributed under the **GNU GPLv3**.
