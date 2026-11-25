#ifndef SYSCALL_WRAPPER_H
#define SYSCALL_WRAPPER_H

struct page_info;

long get_page_info_wrapper(unsigned long vaddr, struct page_info *info, int reset_young);

#endif