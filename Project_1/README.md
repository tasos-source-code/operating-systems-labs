# Project 1: System Calls & Kernel Modules

This project shows the main ways that user programs and loadable modules talk to the
Linux kernel (6.8.0). It has three parts.

## Part 1: New system calls

The kernel source is patched (see [`patch_1`](patch_1)) to add two system calls:

| No. | Name | Description |
|-----|------|-------------|
| 462 | `hello_syscall` | Prints a greeting to the kernel log |
| 463 | `find_roots` | Starts at the calling process and follows `task_struct->parent` up to `init` (PID 1). It logs each ancestor's PID and name. |

The patch changes the following files:

- `arch/x86/entry/syscalls/syscall_64.tbl`: registers the syscall numbers
- `include/linux/syscalls.h`: adds the prototypes
- `kernel/my_syscall.c`, `kernel/find_roots.c`: the implementations
- `kernel/Makefile`: builds the new objects

A small static library wraps the raw `syscall()` call so that user programs can use it:

| File | Purpose |
|------|---------|
| `roots.h` / `roots.c` | `find_roots_wrapper()` calls `syscall(__NR_find_roots)` |
| `find_roots_lib.c` | Test program that calls the wrapper |

## Part 2: Modified Kyber I/O scheduler (`project1_module/`)

`project1-kyber.c` is a copy of the kernel's `block/kyber-iosched.c`, built as a
standalone module with these changes:

- Every `kyber_*` function and type is renamed with the team prefix
  (`team_385838473988_kyber_*`).
- The elevator is registered under the name **`my_team_kyber`**.
- A `printk` in the dispatch path confirms that the modified scheduler is the one
  serving requests.

## Part 3: sysfs interface (`project1_sysfs/`)

`sysfs_module.c` creates the kobject `/sys/kernel/team_385838473988/` with one
attribute, `find_roots`. Reading that file does the same walk to `init` as the
syscall: it logs each ancestor to `dmesg` and returns the PID of the reading process.

## Build & run

### find_roots user program (needs the patched kernel)

```bash
gcc -w -c roots.c -o roots.o
ar rcs libroots.a roots.o
gcc -w find_roots_lib.c -L. -lroots -o find_roots
./find_roots && dmesg | tail
```

### Kyber module

```bash
cd project1_module
make
sudo insmod project1-kyber.ko
cat /sys/block/sda/queue/scheduler            # mq-deadline my_team_kyber none
sudo bash -c 'echo my_team_kyber > /sys/block/sda/queue/scheduler'
echo "this is a test" > ~/mytest && sync
dmesg | tail                                   # "In team_..._kyber_dispatch_request function"
sudo bash -c 'echo mq-deadline > /sys/block/sda/queue/scheduler'   # restore
sudo rmmod project1-kyber
```

### sysfs module

```bash
cd project1_sysfs
make
sudo insmod sysfs_module.ko
cat /sys/kernel/team_385838473988/find_roots
dmesg | tail
sudo rmmod sysfs_module
```

## Files

```
Project_1/
├── patch_1                       # diff against linux-source-6.8.0
├── roots.c, roots.h              # syscall wrapper library
├── find_roots_lib.c              # test program
├── project1_module/              # modified Kyber I/O scheduler
├── project1_sysfs/               # sysfs module
├── README.txt                    # original submission notes
└── *.pdf                         # assignment & instructions (Greek)
```
