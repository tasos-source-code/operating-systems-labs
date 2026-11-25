#include "syscall_wrapper.h"
#include <stdio.h>
#include "page_info.h"
#include <stdlib.h>

#define int_per_page 1024 //1024 integers or 4096 bytes
int size = 280 * 1024;
int load_percentage = 20;
int num_of_pages;

struct results{
    int accessed_pages;
    int present_pages;
    int dirty_pages;
};

void shuffle(int arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

int load_simulation(int *my_space){
    int i;
    volatile int res;
    int page_id[num_of_pages];
    for(int i=0; i<num_of_pages; i++){
        page_id[i]=i;
    }
    //simulate load on random pages.
    shuffle(page_id, num_of_pages);
    for(i=0; i<((load_percentage*num_of_pages)/100) / 2; i++){ // write to 50% of the pages in the load percentage
        my_space[page_id[i]*int_per_page]=69;
    }
    for(; i<((load_percentage*num_of_pages)/100); i++){ // read to 50% of the pages in the load percentage
        res=my_space[page_id[i]*int_per_page];
    }
    return res;
}

void struct_print(struct page_info info){
    printf("page info is:\n \tPA:%lx\n\tis_present=%d\n\tis_dirty=%d\tis_young=%d\n", info.physical_addr,info.is_present,info.is_dirty, info.is_young);
}

void self_check(struct results results_t){
    int is_pass=0;
    printf("load percentage = %d%%,\t-- ", load_percentage);

    printf("accessed pages: %d/%d\t-- ", results_t.accessed_pages, num_of_pages);
    if(results_t.accessed_pages == ((load_percentage*num_of_pages)/100)){
        is_pass+=1;
    }

    printf("present pages: %d/%d\t-- ", results_t.present_pages, num_of_pages);
    if(results_t.present_pages == (num_of_pages)){
        is_pass+=1;
    }

    printf("dirty pages: %d/%d\t-\t", results_t.dirty_pages, num_of_pages);
    // if(results_t.dirty_pages == ((load_percentage*num_of_pages)/100)/2){ //50%
    //     is_pass+=1;
    // }
    if(is_pass==2)
        printf("PASS\n");
    else
        printf("FAIL\n");
}


int main(int argc, char *argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <load_percentage> <num_of_pages>\n", argv[0]);
        return 1;
    }

    // Convert strings from command line to integers
    load_percentage = atoi(argv[1]);
    num_of_pages = atoi(argv[2]);
    size = num_of_pages * int_per_page;
    struct results results = {0, 0, 0};

    int *my_space=NULL;
    // int size = 100*256*sizeof(int);
    struct page_info info;

    my_space = (int*)malloc(size*sizeof(int));
    if(my_space==NULL){
        exit(0);
    }
    volatile int res;
    for(int i=0; i<size; i += int_per_page){
        res=my_space[i];
    }

    for(int i=0; i<size; i += int_per_page){
       if (get_page_info_wrapper((unsigned long)&my_space[i], &info, 1) != 0){
            printf("syscall fault\n");
            exit(0);
        }
    }

    load_simulation(my_space);

    for(int i=0; i<size; i += int_per_page){
        if (get_page_info_wrapper((unsigned long)&my_space[i], &info, 0) != 0){
            printf("syscall fault\n");
            exit(0);
        }
        // struct_print(info);
        results.accessed_pages+=(info.is_young ? 1:0);
        results.present_pages+=(info.is_present ? 1:0);
        results.dirty_pages+=(info.is_dirty ? 1:0);
    }

    // printf("======\tsimulation load\t: %d/%d pages\t======\n", ((load_percentage*num_of_pages)/100), num_of_pages);
    // printf("======\taccessed pages\t: %d/%d\t\t======\n\n", access_counter,num_of_pages);
    self_check(results);
    free(my_space);

    return 0;
}
