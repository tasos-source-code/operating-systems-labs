# Project 3: Page Table Inspection & Working-Set Monitor

This project adds a system call to the Linux 6.8.0 kernel that walks the calling
process's page table for a virtual address and returns the state of the matching page
table entry. A user-space program then uses the call to watch the working set of a
synthetic workload.

## The `get_page_info` system call

```c
long get_page_info(unsigned long vaddr, struct page_info *info, int reset_young);

struct page_info {
    unsigned long physical_addr;
    int is_present;
    int is_dirty;
    int is_young;   /* accessed bit */
};
```

Syscall number **462**. The implementation is in `mm/sys_get_page_info.c` (see
[`patch_3`](patch_3)). It works as follows:

1. Takes `mmap_read_lock` and checks the address with `vma_lookup()`.
2. Walks the page table levels `pgd → p4d → pud → pmd → pte`, checking each level.
3. Maps and locks the PTE with `pte_offset_map_lock()`.
4. Reads the present and dirty bits and computes the physical address from the PFN.
5. Reads the young (accessed) bit. If `reset_young` is set, it also clears the bit with
   `ptep_test_and_clear_young()` and flushes the TLB entry with `flush_tlb_page()`, so
   the next access sets the bit again.
6. Copies the result to user space with `copy_to_user()`.

## Working-set monitor (`wset_monitor.c`)

1. Allocates `N` pages and touches each one so that all of them are present.
2. Clears the young bit of every page with `reset_young = 1`.
3. Accesses a random `load%` of the pages. Half of them are **written** (to test the
   dirty bit) and half are **read**.
4. Queries every page again and counts the accessed, present and dirty pages.
5. Compares the counts with the expected values and prints **PASS** or **FAIL**.

`run_tests.sh` runs the monitor for loads of 10–100% and page counts from 10 to 100 000.
All tests pass. See [`report.txt`](report.txt) for the full output.

### Notes from testing

- Without an explicit TLB flush, some accesses did not set the young bit again.
  Adding `flush_tlb_page()` fixed this.
- The dirty count is sometimes off by one, because `malloc` gives back a first page
  that is already dirty. Allocating with `mmap` would avoid this.

## Build & run (needs the patched kernel)

```bash
gcc -c syscall_wrapper.c -o syscall_wrapper.o
ar rcs lib_page_info.a syscall_wrapper.o
gcc -Wall -g -O0 wset_monitor.c -L. -l_page_info -o wset_monitor

./wset_monitor <load_percentage> <num_of_pages>   # e.g. ./wset_monitor 30 1000

chmod +x run_tests.sh
./run_tests.sh
```

## Files

```
Project_3/
├── patch_3                        # diff against linux-source-6.8.0
├── page_info.h                    # shared struct definition
├── syscall_wrapper.c/.h           # user-space wrapper library
├── wset_monitor.c                 # working-set monitor / test program
├── run_tests.sh                   # test sweep
├── report.txt                     # test output & report
├── README.txt                     # original submission notes
└── Project_03.pdf                 # assignment (Greek)
```
