#ifndef PAGE_INFO_H
#define PAGE_INFO_H

struct page_info {
    unsigned long physical_addr;
    int is_present;
    int is_dirty;
    int is_young;
};

#endif