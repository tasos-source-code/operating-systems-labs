# Operating Systems Lab Projects

Lab assignments for the **Operating Systems (ECE318)** course, Department of Electrical
and Computer Engineering, University of Thessaly, academic year 2025–2026.

The four projects cover the main parts of an operating system: system calls and kernel
modules, CPU scheduling, virtual memory and page tables, and file systems. Projects 1
and 3 change the Linux 6.8.0 kernel itself. Project 2 runs in a user-space scheduler
simulator, and Project 4 is a FUSE file system.

| # | Project | Topic | Main techniques |
|---|---------|-------|-----------------|
| 1 | [System Calls & Kernel Modules](Project_1/README.md) | Kernel ↔ user-space interfaces | New syscalls, a modified Kyber I/O scheduler module, sysfs attributes |
| 2 | [SJF Process Scheduler](Project_2/README.md) | CPU scheduling | Shortest Job First with burst estimation and a "goodness" metric, compared against Round Robin |
| 3 | [Page Table Inspection](Project_3/README.md) | Virtual memory | A syscall that walks the page table and reports present/dirty/young bits, plus a working-set monitor |
| 4 | [Deduplicating FUSE File System](Project_4/README.md) | File systems | Block-level deduplication with SHA-1 content addressing on top of FUSE |

## Repository layout

```
.
├── Project_1/   # find_roots syscall, Kyber I/O scheduler module, sysfs module
├── Project_2/   # Scheduler virtual machine + SJF / Round Robin scheduler
├── Project_3/   # get_page_info syscall + working-set monitor & tests
└── Project_4/   # Deduplicating file system (FUSE), testbench, report
```

Each project folder has its own `README.md` with build and run instructions and a
summary of the results. Each folder also keeps the assignment description
(`Project_0X.pdf`, in Greek) and the notes the team submitted (`README.txt` /
`report.txt` / `report_lab4.pdf`).

## Environment

- Ubuntu running in a virtual machine, with the **Linux 6.8.0** kernel source tree
  (Projects 1 and 3)
- `gcc`, `make`, and kernel headers for building modules
- `libfuse` (FUSE 2.x) and OpenSSL `libcrypto` (Project 4)

> ⚠️ Projects 1 and 3 patch and rebuild the kernel, and they load custom modules. Run
> them only inside a disposable virtual machine.

## Team

- Nikolaos Dimitriou
- Anastasios Papadopoulos
- Anastasia Efthymiou

## License

The original work in this repository is released under the [Apache License 2.0](LICENSE).

Some files are derived from third-party code and keep their original licenses:

- `Project_1/project1_module/project1-kyber.c`, the kernel patches (`patch_1`,
  `patch_3`) and the kernel modules: derived from the Linux kernel (**GPL-2.0**).
- `Project_4/project/filesystem/*`: based on the *Big Brother File System* by
  Joseph J. Pfeiffer (**GPL-3.0**).
- `Project_2/Scheduler_VM/` VM files (`cpu.c`, `cpuinit.c`, `list.h`, …): provided by
  the course instructors.
