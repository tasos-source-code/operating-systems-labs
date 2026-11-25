#include "syscall_wrapper.h"
#include "page_info.h"
#include <unistd.h>
#include <sys/syscall.h>

long get_page_info_wrapper(unsigned long vaddr, struct page_info *info, int reset_young) {
    return syscall(__NR_get_page_info, vaddr, info, reset_young);
}